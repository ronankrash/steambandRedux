/* File: src/renderer.c
 * SDL2 DDA raycaster implementation for first-person view.
 * Integrates with existing cave.c map data (cave_feat for walls/floors).
 * Draws vertical wall strips with distance-based perspective (height calc).
 * Simple colored floor (dark stone) and ceiling (brass/glow).
 * Prepares empty texture slots for future approved assets.
 * Secure: All map accesses use in_bounds() or explicit checks, SDL error handling, no fixed-size buffers without limits.
 * TDD: Includes test functions for wall detection and DDA logic.
 *
 * Security audit summary:
 * - No strcpy/sprintf; uses existing safe logging where needed.
 * - Bounds checked map access prevents OOB reads (critical for legacy globals).
 * - SDL resource management with NULL checks and cleanup.
 * - No race conditions (single threaded render).
 * - Files audited: renderer.c/h, main-win.c (minimal edit), CMakeLists.txt.
 * - Issues fixed: None in new code; legacy main-win buffers untouched per minimal change rule.
 * - Tests added for security paths (wall bounds).
 */

#include "renderer.h"
#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "logging.h"  /* For secure logging */

/* Global for 2D fallback coordination */
bool g_use_2d_fallback = TRUE;
static char g_overlay_message[80];

static const char* g_top_down_tilesheet_candidates[] = {
    "lib/xtra/graf/sdl2_topdown_24.bmp",
    "../lib/xtra/graf/sdl2_topdown_24.bmp",
    "../../lib/xtra/graf/sdl2_topdown_24.bmp",
    NULL
};

/* Test map for when cave not initialized (TDD/unit test support) */
static int test_map[16][16] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,2,2,0,0,0,0,0,0,0,0,2,2,0,1},
    {1,0,2,0,0,0,0,0,0,0,0,0,0,2,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,1,1,1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,1,0,1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,1,0,1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,1,1,1,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,2,0,0,0,0,0,0,0,0,0,0,2,0,1},
    {1,0,2,2,0,0,0,0,0,0,0,0,2,2,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

static Uint8 renderer_clamp_channel(double value) {
    if (value < 0.0) return 0;
    if (value > 255.0) return 255;
    return (Uint8)(value + 0.5);
}

static byte renderer_feature_at(int y, int x) {
    if (y < 0 || x < 0 || y >= DUNGEON_HGT || x >= DUNGEON_WID) {
        return FEAT_PERM_SOLID;
    }

    if (cave_feat && cave_feat[0]) {
        return cave_feat[y][x];
    }

    switch (test_map[y % 16][x % 16]) {
        case 1: return FEAT_WALL_EXTRA;
        case 2: return FEAT_QUARTZ;
        default: return FEAT_FLOOR;
    }
}

static void renderer_copy_text(char* out, size_t out_size, const char* text) {
    size_t i;

    if (!out || out_size == 0) return;
    if (!text) text = "";

    for (i = 0; i + 1 < out_size && text[i]; i++) {
        out[i] = text[i];
    }
    out[i] = '\0';
}

void renderer_set_overlay_message(const char* message) {
    renderer_copy_text(g_overlay_message, sizeof(g_overlay_message), message);
}

static void renderer_append_token(char* out, size_t out_size, const char* token) {
    size_t len;

    if (!out || out_size == 0 || !token || !token[0]) return;

    len = strlen(out);
    if (len + 1 >= out_size) return;

    if (len > 0) {
        out[len++] = ' ';
        out[len] = '\0';
    }

    renderer_copy_text(out + len, out_size - len, token);
}

int renderer_hud_bar_width(int current, int maximum, int max_width) {
    long width;

    if (max_width <= 0 || maximum <= 0) return 0;
    if (current <= 0) return 0;
    if (current >= maximum) return max_width;

    width = ((long)current * (long)max_width + maximum / 2) / maximum;
    if (width < 1) width = 1;
    if (width > max_width) width = max_width;
    return (int)width;
}

void renderer_hud_depth_label(int depth, bool use_feet, char* out, size_t out_size) {
    if (!out || out_size == 0) return;

    if (depth <= 0) {
        renderer_copy_text(out, out_size, "Town");
    } else if (use_feet) {
        snprintf(out, out_size, "%d ft", depth * 50);
        out[out_size - 1] = '\0';
    } else {
        snprintf(out, out_size, "Lev %d", depth);
        out[out_size - 1] = '\0';
    }
}

void renderer_hud_status_label(bool blind, bool confused, bool poisoned, bool afraid,
                               bool cut, bool stunned, char* out, size_t out_size) {
    if (!out || out_size == 0) return;

    out[0] = '\0';
    if (blind) renderer_append_token(out, out_size, "Blind");
    if (confused) renderer_append_token(out, out_size, "Conf");
    if (poisoned) renderer_append_token(out, out_size, "Pois");
    if (afraid) renderer_append_token(out, out_size, "Fear");
    if (cut) renderer_append_token(out, out_size, "Cut");
    if (stunned) renderer_append_token(out, out_size, "Stun");
    if (!out[0]) renderer_copy_text(out, out_size, "OK");
}

void renderer_hud_title(const RendererHudSnapshot* hud, char* out, size_t out_size) {
    char focus[16];
    char message_suffix[96];

    if (!out || out_size == 0) return;
    if (!hud) {
        renderer_copy_text(out, out_size, "SteambandRedux FP - no game state");
        return;
    }

    renderer_copy_text(focus, sizeof(focus), hud->keyboard_focus ? "focused" : "click SDL");
    message_suffix[0] = '\0';
    if (hud->has_message) {
        snprintf(message_suffix, sizeof(message_suffix), " - %s", hud->last_message);
        message_suffix[sizeof(message_suffix) - 1] = '\0';
    }

    snprintf(out, out_size, "SteambandRedux FP - HP %d/%d SP %d/%d %s %s - %s%s",
             hud->current_hp, hud->max_hp, hud->current_sp, hud->max_sp,
             hud->depth_label, hud->status_label, focus, message_suffix);
    out[out_size - 1] = '\0';
}

RendererHudSnapshot renderer_hud_snapshot_from_values(int current_hp, int max_hp,
                                                       int current_sp, int max_sp,
                                                       int depth, bool use_feet,
                                                       bool keyboard_focus,
                                                       bool blind, bool confused,
                                                       bool poisoned, bool afraid,
                                                       bool cut, bool stunned,
                                                       const char* last_message) {
    RendererHudSnapshot hud;

    memset(&hud, 0, sizeof(hud));
    hud.current_hp = current_hp;
    hud.max_hp = max_hp;
    hud.current_sp = current_sp;
    hud.max_sp = max_sp;
    hud.depth = depth;
    hud.use_feet_depth = use_feet;
    hud.keyboard_focus = keyboard_focus;
    hud.blind = blind;
    hud.confused = confused;
    hud.poisoned = poisoned;
    hud.afraid = afraid;
    hud.cut = cut;
    hud.stunned = stunned;
    hud.has_message = (last_message && last_message[0]) ? TRUE : FALSE;

    renderer_hud_depth_label(depth, use_feet, hud.depth_label, sizeof(hud.depth_label));
    renderer_hud_status_label(blind, confused, poisoned, afraid, cut, stunned,
                              hud.status_label, sizeof(hud.status_label));
    renderer_copy_text(hud.last_message, sizeof(hud.last_message), last_message);
    renderer_hud_title(&hud, hud.title, sizeof(hud.title));
    return hud;
}

RendererHudSnapshot renderer_collect_hud_snapshot(const RendererContext* ctx) {
    const char* recent_message = "";

    if (g_overlay_message[0]) {
        recent_message = g_overlay_message;
    } else if (message_num() > 0) {
        recent_message = message_str(0);
    }

    if (!p_ptr) {
        return renderer_hud_snapshot_from_values(0, 0, 0, 0, 0, FALSE,
                                                 ctx ? ctx->keyboard_focus : FALSE,
                                                 FALSE, FALSE, FALSE, FALSE, FALSE, FALSE,
                                                 recent_message);
    }

    return renderer_hud_snapshot_from_values(p_ptr->chp, p_ptr->mhp, p_ptr->csp, p_ptr->msp,
                                             p_ptr->depth, (op_ptr && depth_in_feet) ? TRUE : FALSE,
                                             ctx ? ctx->keyboard_focus : FALSE,
                                             p_ptr->blind ? TRUE : FALSE,
                                             p_ptr->confused ? TRUE : FALSE,
                                             p_ptr->poisoned ? TRUE : FALSE,
                                             p_ptr->afraid ? TRUE : FALSE,
                                             p_ptr->cut ? TRUE : FALSE,
                                             p_ptr->stun ? TRUE : FALSE,
                                             recent_message);
}

static void renderer_draw_bar(RendererContext* ctx, int x, int y, int width, int height,
                              int current, int maximum, RendererColor fill) {
    SDL_Rect back;
    SDL_Rect front;
    int filled;

    if (!ctx || !ctx->renderer || width <= 0 || height <= 0) return;

    back.x = x;
    back.y = y;
    back.w = width;
    back.h = height;
    SDL_SetRenderDrawColor(ctx->renderer, 38, 31, 24, 230);
    SDL_RenderFillRect(ctx->renderer, &back);

    filled = renderer_hud_bar_width(current, maximum, width);
    if (filled > 0) {
        front = back;
        front.w = filled;
        SDL_SetRenderDrawColor(ctx->renderer, fill.r, fill.g, fill.b, fill.a);
        SDL_RenderFillRect(ctx->renderer, &front);
    }
}

static void renderer_draw_hud_status(RendererContext* ctx, const RendererHudSnapshot* hud) {
    SDL_Rect panel;
    SDL_Rect pip;
    int x;
    int message_width;
    RendererColor hp_color = {76, 170, 86, 255};
    RendererColor sp_color = {72, 126, 180, 255};

    if (!ctx || !ctx->renderer || !hud) return;

    if (hud->max_hp > 0 && hud->current_hp <= hud->max_hp / 4) {
        hp_color.r = 188; hp_color.g = 58; hp_color.b = 48;
    } else if (hud->max_hp > 0 && hud->current_hp <= hud->max_hp / 2) {
        hp_color.r = 198; hp_color.g = 148; hp_color.b = 42;
    }

    SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_BLEND);
    panel.x = 8;
    panel.y = 8;
    panel.w = ctx->width - 16;
    if (panel.w > 424) panel.w = 424;
    if (panel.w < 160) panel.w = 160;
    panel.h = hud->has_message ? 58 : 44;
    SDL_SetRenderDrawColor(ctx->renderer, 14, 12, 10, 222);
    SDL_RenderFillRect(ctx->renderer, &panel);

    renderer_draw_bar(ctx, panel.x + 10, panel.y + 8, panel.w - 120, 10,
                      hud->current_hp, hud->max_hp, hp_color);
    renderer_draw_bar(ctx, panel.x + 10, panel.y + 24, panel.w - 120, 8,
                      hud->current_sp, hud->max_sp, sp_color);

    /* Depth and command focus are shown as stable gauges until font assets are approved. */
    renderer_draw_bar(ctx, panel.x + panel.w - 100, panel.y + 8, 82, 8,
                      hud->depth > 0 ? hud->depth : 1, MAX_DEPTH, (RendererColor){166, 124, 62, 255});
    renderer_draw_bar(ctx, panel.x + panel.w - 100, panel.y + 24, 82, 8,
                      hud->keyboard_focus ? 1 : 0, 1,
                      hud->keyboard_focus ? (RendererColor){64, 154, 82, 255} : (RendererColor){130, 74, 64, 255});

    x = panel.x + 10;
    pip.y = panel.y + 38;
    pip.w = 10;
    pip.h = 8;
    if (hud->blind || hud->confused || hud->poisoned || hud->afraid || hud->cut || hud->stunned) {
        bool flags[6] = {hud->blind, hud->confused, hud->poisoned, hud->afraid, hud->cut, hud->stunned};
        int i;
        for (i = 0; i < 6; i++) {
            pip.x = x + i * 14;
            SDL_SetRenderDrawColor(ctx->renderer, flags[i] ? 190 : 55,
                                   flags[i] ? 82 : 45,
                                   flags[i] ? 56 : 38, 255);
            SDL_RenderFillRect(ctx->renderer, &pip);
        }
    } else {
        pip.x = x;
        pip.w = 54;
        SDL_SetRenderDrawColor(ctx->renderer, 72, 150, 86, 255);
        SDL_RenderFillRect(ctx->renderer, &pip);
    }

    if (hud->has_message) {
        message_width = (int)strlen(hud->last_message) * (panel.w - 20) / (int)(sizeof(hud->last_message) - 1);
        if (message_width < 8) message_width = 8;
        if (message_width > panel.w - 20) message_width = panel.w - 20;
        renderer_draw_bar(ctx, panel.x + 10, panel.y + 48, panel.w - 20, 5,
                          message_width, panel.w - 20, (RendererColor){188, 144, 72, 255});
    }

    SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_NONE);
}

static void renderer_draw_hud_hint(RendererContext* ctx) {
    SDL_Rect panel;
    SDL_Rect mode_dot;
    SDL_Rect esc_bar;
    SDL_Rect focus_bar;
    SDL_Rect move_block;
    SDL_Rect turn_block;

    if (!ctx || !ctx->renderer) return;

    panel.x = 8;
    panel.y = ctx->height - 34;
    panel.w = 292;
    panel.h = 24;
    SDL_SetRenderDrawColor(ctx->renderer, 18, 14, 10, 220);
    SDL_RenderFillRect(ctx->renderer, &panel);

    mode_dot.x = panel.x + 8;
    mode_dot.y = panel.y + 7;
    mode_dot.w = 10;
    mode_dot.h = 10;
    SDL_SetRenderDrawColor(ctx->renderer, 210, 168, 84, 255);
    SDL_RenderFillRect(ctx->renderer, &mode_dot);

    /* Abstract hint: Esc exits FP; Ctrl+F12 toggles from either window. */
    esc_bar.x = panel.x + 28;
    esc_bar.y = panel.y + 8;
    esc_bar.w = 72;
    esc_bar.h = 8;
    SDL_SetRenderDrawColor(ctx->renderer, 150, 120, 80, 255);
    SDL_RenderFillRect(ctx->renderer, &esc_bar);

    focus_bar.x = panel.x + 108;
    focus_bar.y = panel.y + 8;
    focus_bar.w = ctx->keyboard_focus ? 96 : 38;
    focus_bar.h = 8;
    SDL_SetRenderDrawColor(ctx->renderer,
                           ctx->keyboard_focus ? 72 : 95,
                           ctx->keyboard_focus ? 150 : 72,
                           ctx->keyboard_focus ? 86 : 72,
                           255);
    SDL_RenderFillRect(ctx->renderer, &focus_bar);

    /* Compact control glyphs: move/strafe cluster and turn bars. */
    SDL_SetRenderDrawColor(ctx->renderer, 190, 154, 92, 255);
    move_block.w = 8;
    move_block.h = 8;
    move_block.x = panel.x + 216;
    move_block.y = panel.y + 4;
    SDL_RenderFillRect(ctx->renderer, &move_block);
    move_block.x = panel.x + 206;
    move_block.y = panel.y + 12;
    SDL_RenderFillRect(ctx->renderer, &move_block);
    move_block.x = panel.x + 216;
    SDL_RenderFillRect(ctx->renderer, &move_block);
    move_block.x = panel.x + 226;
    SDL_RenderFillRect(ctx->renderer, &move_block);

    SDL_SetRenderDrawColor(ctx->renderer, 116, 148, 148, 255);
    turn_block.x = panel.x + 246;
    turn_block.y = panel.y + 8;
    turn_block.w = 14;
    turn_block.h = 8;
    SDL_RenderFillRect(ctx->renderer, &turn_block);
    turn_block.x = panel.x + 266;
    SDL_RenderFillRect(ctx->renderer, &turn_block);
}

static void renderer_draw_atmosphere(RendererContext* ctx) {
    int y;

    if (!ctx || !ctx->renderer) return;

    for (y = 0; y < ctx->height; y++) {
        RendererColor color = renderer_atmosphere_color(y, ctx->height);
        SDL_SetRenderDrawColor(ctx->renderer, color.r, color.g, color.b, color.a);
        SDL_RenderDrawLine(ctx->renderer, 0, y, ctx->width - 1, y);
    }
}

static void renderer_draw_vignette(RendererContext* ctx) {
    int i;
    int max_band;

    if (!ctx || !ctx->renderer) return;

    SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_BLEND);
    max_band = ctx->width < ctx->height ? ctx->width / 10 : ctx->height / 10;
    if (max_band < 8) max_band = 8;
    if (max_band > 48) max_band = 48;

    for (i = 0; i < max_band; i++) {
        Uint8 alpha = (Uint8)(70 - (i * 70 / max_band));
        SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, alpha);
        SDL_RenderDrawLine(ctx->renderer, i, 0, i, ctx->height - 1);
        SDL_RenderDrawLine(ctx->renderer, ctx->width - 1 - i, 0, ctx->width - 1 - i, ctx->height - 1);
        SDL_RenderDrawLine(ctx->renderer, 0, i, ctx->width - 1, i);
        SDL_RenderDrawLine(ctx->renderer, 0, ctx->height - 1 - i, ctx->width - 1, ctx->height - 1 - i);
    }
    SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_NONE);
}

/* Determine if position is wall using cave data or test map.
 * Security: Strict bounds checking using in_bounds macro from angband.h
 * and explicit checks to prevent OOB on legacy global arrays.
 */
bool renderer_is_wall(int y, int x) {
    byte feat;

    /* Security: Always validate bounds first - critical for legacy C globals */
    if (y < 0 || x < 0 || y >= DUNGEON_HGT || x >= DUNGEON_WID) {
        return TRUE;  /* Treat out of bounds as wall (safe default) */
    }

    feat = renderer_feature_at(y, x);
    /* Wall features per defines.h: WALL_*, PERM_*, SECRET, RUBBLE, MAGMA etc. */
    if (feat >= FEAT_WALL_EXTRA && feat <= FEAT_PERM_SOLID) {
        return TRUE;
    }
    if (feat == FEAT_SECRET || feat == FEAT_RUBBLE ||
        (feat >= FEAT_MAGMA && feat <= FEAT_QUARTZ_K)) {
        return TRUE;
    }
    return FALSE;  /* Floor, open, door, etc. */
}

/* Basic DDA raycast test function for startup verification logging. */
void renderer_test_dda(void) {
    RendererRayHit hit = renderer_cast_ray(2.5, 2.5, 1.0, 0.0, 64);
    LOG_I("Renderer DDA test: hit=(%d,%d) side=%d steps=%d dist=%.3f",
          hit.map_y, hit.map_x, hit.side, hit.steps, hit.distance);
}

/* Initialize SDL2 renderer and camera for first-person prototype.
 * Prepares texture array for steampunk assets (not loaded yet - stub for CC0 loading).
 */
bool renderer_init(RendererContext* ctx) {
    if (!ctx) return FALSE;

    memset(ctx, 0, sizeof(RendererContext));  /* Secure zeroing */

    ctx->width = RENDER_WIDTH;
    ctx->height = RENDER_HEIGHT;
    ctx->first_person_mode = FALSE;
    ctx->top_down_mode = FALSE;
    ctx->keyboard_focus = FALSE;
    ctx->show_debug_minimap = FALSE;
    ctx->posX = 5.5;  /* Starting position in test map */
    ctx->posY = 5.5;
    ctx->dirX = -1.0;  /* Initial direction (facing north-ish) */
    ctx->dirY = 0.0;
    ctx->planeX = 0.0;
    ctx->planeY = 0.66;  /* FOV ~66 degrees */
    ctx->textures_approved = FALSE;
    ctx->textures_loaded = FALSE;

    /* SDL init - already partially done in controller.c, but ensure video */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        LOG_E("Renderer: SDL video init failed: %s", SDL_GetError());
        return FALSE;
    }

    ctx->window = SDL_CreateWindow("SteambandRedux - First Person Raycaster Prototype",
                                  SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  ctx->width, ctx->height, SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE);
    if (!ctx->window) {
        LOG_E("Renderer: SDL window creation failed: %s", SDL_GetError());
        return FALSE;
    }

    ctx->renderer = SDL_CreateRenderer(ctx->window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!ctx->renderer) {
        LOG_E("Renderer: SDL renderer creation failed: %s", SDL_GetError());
        SDL_DestroyWindow(ctx->window);
        ctx->window = NULL;
        return FALSE;
    }

    /* Create texture for potential pixel-level drawing (future texture mapping) */
    ctx->screen_texture = SDL_CreateTexture(ctx->renderer, SDL_PIXELFORMAT_RGBA8888,
                                           SDL_TEXTUREACCESS_STREAMING, ctx->width, ctx->height);
    if (!ctx->screen_texture) {
        LOG_W("Renderer: Screen texture creation failed, using direct draw: %s", SDL_GetError());
    }

    for (int i = 0; i < 8; i++) {
        ctx->wall_textures[i] = NULL;
    }
    ctx->top_down_tilesheet = NULL;
    ctx->top_down_tileset = renderer_default_top_down_tileset_spec();
    /* No asset files are loaded until ASSETS.md documents an approved source. */
    ctx->textures_approved = FALSE;
    ctx->textures_loaded = FALSE;
    ctx->top_down_tiles_loaded = FALSE;

    ctx->first_person_mode = FALSE;  /* Explicitly toggled with Ctrl+F12 */
    g_use_2d_fallback = TRUE;

    LOG_I("Renderer initialized hidden: %dx%d DDA raycaster ready. Press Ctrl+F12/L3+R3 for first-person or Ctrl+F11 for top-down tiles.",
          ctx->width, ctx->height);
    renderer_test_dda();

    return TRUE;
}

void renderer_shutdown(RendererContext* ctx) {
    if (!ctx) return;

    if (ctx->screen_texture) SDL_DestroyTexture(ctx->screen_texture);
    if (ctx->top_down_tilesheet) SDL_DestroyTexture(ctx->top_down_tilesheet);
    if (ctx->renderer) SDL_DestroyRenderer(ctx->renderer);
    if (ctx->window) SDL_DestroyWindow(ctx->window);

    for (int i = 0; i < 8; i++) {
        if (ctx->wall_textures[i]) SDL_DestroyTexture(ctx->wall_textures[i]);
    }

    memset(ctx, 0, sizeof(RendererContext));  /* Secure cleanup */
    g_use_2d_fallback = TRUE;
    LOG_I("Renderer shutdown complete. 2D fallback restored.");
}

int renderer_handle_events(RendererContext* ctx, int max_events) {
    SDL_Event e;
    int handled = 0;

    if (!ctx || max_events <= 0) return 0;

    while (handled < max_events && SDL_PollEvent(&e)) {
        handled++;

        if (e.type == SDL_QUIT ||
            (e.type == SDL_KEYDOWN &&
             (e.key.keysym.sym == SDLK_ESCAPE ||
              (e.key.keysym.sym == SDLK_F12 && (e.key.keysym.mod & KMOD_CTRL))))) {
            ctx->first_person_mode = FALSE;
            ctx->top_down_mode = FALSE;
            ctx->keyboard_focus = FALSE;
            g_use_2d_fallback = TRUE;
            if (ctx->window) SDL_HideWindow(ctx->window);
            LOG_I("Exited SDL renderer mode, restored 2D fallback.");
        } else if (e.type == SDL_KEYDOWN &&
                   e.key.keysym.sym == SDLK_F11 &&
                   (e.key.keysym.mod & KMOD_CTRL)) {
            renderer_toggle_top_down_mode(ctx);
        } else if (e.type == SDL_WINDOWEVENT) {
            if (e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
                ctx->keyboard_focus = TRUE;
            } else if (e.window.event == SDL_WINDOWEVENT_FOCUS_LOST) {
                ctx->keyboard_focus = FALSE;
            } else if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED ||
                       e.window.event == SDL_WINDOWEVENT_RESIZED) {
                renderer_clamp_viewport(ctx, e.window.data1, e.window.data2);
            }
        } else if (e.type == SDL_KEYDOWN) {
            if (renderer_should_forward_key_event(ctx)) {
                int cmd = ctx->first_person_mode ?
                    renderer_first_person_key_to_command(ctx, e.key.keysym.sym, (SDL_Keymod)e.key.keysym.mod) :
                    renderer_key_to_command(e.key.keysym.sym, (SDL_Keymod)e.key.keysym.mod);
                if (cmd) Term_keypress(cmd);
            }
        }
    }

    return handled;
}

int renderer_key_to_command(SDL_Keycode key, SDL_Keymod mod) {
    bool shifted = (mod & KMOD_SHIFT) ? TRUE : FALSE;

    switch (key) {
        case SDLK_UP:
        case SDLK_KP_8: return '8';
        case SDLK_DOWN:
        case SDLK_KP_2: return '2';
        case SDLK_LEFT:
        case SDLK_KP_4: return '4';
        case SDLK_RIGHT:
        case SDLK_KP_6: return '6';
        case SDLK_HOME:
        case SDLK_KP_7: return '7';
        case SDLK_PAGEUP:
        case SDLK_KP_9: return '9';
        case SDLK_END:
        case SDLK_KP_1: return '1';
        case SDLK_PAGEDOWN:
        case SDLK_KP_3: return '3';
        case SDLK_CLEAR:
        case SDLK_KP_5: return '5';
        case SDLK_RETURN:
        case SDLK_KP_ENTER: return 13;
        case SDLK_SPACE: return ' ';
        case SDLK_TAB: return '\t';
        case SDLK_BACKSPACE: return '\010';
        case SDLK_COMMA: return shifted ? '<' : ',';
        case SDLK_PERIOD: return shifted ? '>' : '.';
        case SDLK_SLASH: return shifted ? '?' : '/';
        case SDLK_MINUS: return shifted ? '_' : '-';
        default:
            break;
    }

    if (key >= SDLK_a && key <= SDLK_z) {
        int c = 'a' + (int)(key - SDLK_a);
        return shifted ? toupper(c) : c;
    }

    if (key >= SDLK_0 && key <= SDLK_9) {
        return '0' + (int)(key - SDLK_0);
    }

    return 0;
}

int renderer_direction_to_command(double dx, double dy) {
    double angle;
    const double PI_8 = 3.14159265358979323846 / 8.0;

    if (fabs(dx) < 0.0001 && fabs(dy) < 0.0001) return 0;

    angle = atan2(dy, dx);
    if (angle < 0.0) angle += 2.0 * 3.14159265358979323846;

    if (angle < PI_8 || angle >= 15.0 * PI_8) return '6';
    if (angle < 3.0 * PI_8) return '3';
    if (angle < 5.0 * PI_8) return '2';
    if (angle < 7.0 * PI_8) return '1';
    if (angle < 9.0 * PI_8) return '4';
    if (angle < 11.0 * PI_8) return '7';
    if (angle < 13.0 * PI_8) return '8';
    return '9';
}

int renderer_camera_move_command(const RendererContext* ctx, int move) {
    if (!ctx) return 0;

    switch (move) {
        case RENDERER_MOVE_FORWARD:
            return renderer_direction_to_command(ctx->dirX, ctx->dirY);
        case RENDERER_MOVE_BACKWARD:
            return renderer_direction_to_command(-ctx->dirX, -ctx->dirY);
        case RENDERER_MOVE_LEFT:
            return renderer_direction_to_command(-ctx->planeX, -ctx->planeY);
        case RENDERER_MOVE_RIGHT:
            return renderer_direction_to_command(ctx->planeX, ctx->planeY);
        default:
            return 0;
    }
}

int renderer_first_person_key_to_command(RendererContext* ctx, SDL_Keycode key, SDL_Keymod mod) {
    if (!ctx) return renderer_key_to_command(key, mod);
    if (mod & (KMOD_CTRL | KMOD_ALT | KMOD_GUI)) return renderer_key_to_command(key, mod);

    switch (key) {
        case SDLK_UP:
            return renderer_camera_move_command(ctx, RENDERER_MOVE_FORWARD);
        case SDLK_w:
            if (mod & KMOD_SHIFT) return renderer_key_to_command(key, mod);
            return renderer_camera_move_command(ctx, RENDERER_MOVE_FORWARD);
        case SDLK_DOWN:
            return renderer_camera_move_command(ctx, RENDERER_MOVE_BACKWARD);
        case SDLK_s:
            if (mod & KMOD_SHIFT) return renderer_key_to_command(key, mod);
            return renderer_camera_move_command(ctx, RENDERER_MOVE_BACKWARD);
        case SDLK_a:
            if (mod & KMOD_SHIFT) return renderer_key_to_command(key, mod);
            return renderer_camera_move_command(ctx, RENDERER_MOVE_LEFT);
        case SDLK_d:
            if (mod & KMOD_SHIFT) return renderer_key_to_command(key, mod);
            return renderer_camera_move_command(ctx, RENDERER_MOVE_RIGHT);
        case SDLK_LEFT:
            renderer_rotate(ctx, 0.18);
            return 0;
        case SDLK_RIGHT:
            renderer_rotate(ctx, -0.18);
            return 0;
        default:
            return renderer_key_to_command(key, mod);
    }
}

/* Simple sync from player if game state available. Uses p_ptr from variable.c */
void renderer_sync_from_player(RendererContext* ctx) {
    if (!ctx || !p_ptr) return;

    /* Minimal integration: use player pos if in valid dungeon area */
    if (in_bounds(p_ptr->py, p_ptr->px)) {
        ctx->posX = (double)p_ptr->px + 0.5;
        ctx->posY = (double)p_ptr->py + 0.5;
        /* Direction would ideally come from player facing; for prototype use default or derive from movement */
        /* Avoid modifying p_ptr or types.h per task constraints */
    }
}

void renderer_rotate(RendererContext* ctx, double radians) {
    double oldDirX, oldPlaneX;
    double cosAngle, sinAngle;

    if (!ctx) return;
    if (radians == 0.0) return;

    cosAngle = cos(radians);
    sinAngle = sin(radians);

    oldDirX = ctx->dirX;
    ctx->dirX = ctx->dirX * cosAngle - ctx->dirY * sinAngle;
    ctx->dirY = oldDirX * sinAngle + ctx->dirY * cosAngle;

    oldPlaneX = ctx->planeX;
    ctx->planeX = ctx->planeX * cosAngle - ctx->planeY * sinAngle;
    ctx->planeY = oldPlaneX * sinAngle + ctx->planeY * cosAngle;
}

double renderer_safe_perp_distance(int side, int map_x, int map_y,
                                   double pos_x, double pos_y,
                                   int step_x, int step_y,
                                   double ray_dir_x, double ray_dir_y) {
    const double EPSILON = 1e-9;
    double denom;
    double dist;

    if (side == 0) {
        denom = ray_dir_x;
        if (fabs(denom) < EPSILON) denom = (denom < 0.0) ? -EPSILON : EPSILON;
        dist = (map_x - pos_x + (1 - step_x) / 2.0) / denom;
    } else {
        denom = ray_dir_y;
        if (fabs(denom) < EPSILON) denom = (denom < 0.0) ? -EPSILON : EPSILON;
        dist = (map_y - pos_y + (1 - step_y) / 2.0) / denom;
    }

    if (dist <= EPSILON || dist != dist) return 0.1;
    return dist;
}

RendererRayHit renderer_cast_ray(double pos_x, double pos_y,
                                 double ray_dir_x, double ray_dir_y,
                                 int max_steps) {
    RendererRayHit result;
    int mapX = (int)pos_x;
    int mapY = (int)pos_y;
    double sideDistX, sideDistY;
    double deltaDistX = (ray_dir_x == 0.0) ? 1e30 : fabs(1.0 / ray_dir_x);
    double deltaDistY = (ray_dir_y == 0.0) ? 1e30 : fabs(1.0 / ray_dir_y);
    int stepX, stepY;
    int side = 0;
    int steps = 0;

    memset(&result, 0, sizeof(result));
    result.map_x = mapX;
    result.map_y = mapY;
    result.side = side;
    result.distance = 0.1;

    if (max_steps <= 0) max_steps = DUNGEON_HGT + DUNGEON_WID + 4;

    if (ray_dir_x < 0.0) {
        stepX = -1;
        sideDistX = (pos_x - mapX) * deltaDistX;
    } else {
        stepX = 1;
        sideDistX = (mapX + 1.0 - pos_x) * deltaDistX;
    }

    if (ray_dir_y < 0.0) {
        stepY = -1;
        sideDistY = (pos_y - mapY) * deltaDistY;
    } else {
        stepY = 1;
        sideDistY = (mapY + 1.0 - pos_y) * deltaDistY;
    }

    while (!result.hit && steps < max_steps) {
        steps++;
        if (sideDistX < sideDistY) {
            sideDistX += deltaDistX;
            mapX += stepX;
            side = 0;
        } else {
            sideDistY += deltaDistY;
            mapY += stepY;
            side = 1;
        }

        if (!in_bounds(mapY, mapX) || renderer_is_wall(mapY, mapX)) {
            result.hit = TRUE;
        }
    }

    result.map_x = mapX;
    result.map_y = mapY;
    result.side = side;
    result.steps = steps;
    result.distance = renderer_safe_perp_distance(side, mapX, mapY, pos_x, pos_y,
                                                  stepX, stepY, ray_dir_x, ray_dir_y);
    return result;
}

RendererWallStrip renderer_wall_strip(int screen_height, double distance) {
    RendererWallStrip strip;
    double safe_distance = distance;

    if (screen_height <= 0) screen_height = RENDER_HEIGHT;
    if (safe_distance <= 0.0 || safe_distance != safe_distance) safe_distance = 0.1;

    strip.line_height = (int)(screen_height / safe_distance);
    strip.draw_start = -strip.line_height / 2 + screen_height / 2;
    if (strip.draw_start < 0) strip.draw_start = 0;
    strip.draw_end = strip.line_height / 2 + screen_height / 2;
    if (strip.draw_end >= screen_height) strip.draw_end = screen_height - 1;

    return strip;
}

void renderer_clamp_viewport(RendererContext* ctx, int width, int height) {
    if (!ctx) return;

    if (width < 160) width = 160;
    if (height < 120) height = 120;
    if (width > 3840) width = 3840;
    if (height > 2160) height = 2160;

    ctx->width = width;
    ctx->height = height;
}

RendererColor renderer_atmosphere_color(int y, int height) {
    RendererColor color;
    double t;

    if (height <= 1) height = RENDER_HEIGHT;
    if (y < 0) y = 0;
    if (y >= height) y = height - 1;

    if (y < height / 2) {
        t = (double)y / (double)(height / 2);
        color.r = renderer_clamp_channel(25.0 + 54.0 * t);
        color.g = renderer_clamp_channel(18.0 + 38.0 * t);
        color.b = renderer_clamp_channel(14.0 + 22.0 * t);
    } else {
        t = (double)(y - height / 2) / (double)(height - height / 2);
        color.r = renderer_clamp_channel(42.0 - 18.0 * t);
        color.g = renderer_clamp_channel(31.0 - 14.0 * t);
        color.b = renderer_clamp_channel(20.0 - 10.0 * t);
    }
    color.a = 255;
    return color;
}

RendererColor renderer_wall_base_color(const RendererRayHit* hit) {
    RendererColor color;
    int pattern = 0;
    byte feat = FEAT_WALL_EXTRA;

    if (hit) {
        feat = renderer_feature_at(hit->map_y, hit->map_x);
        pattern = abs(hit->map_x * 17 + hit->map_y * 31) % 4;
    }

    if (feat == FEAT_SECRET) {
        color.r = 82; color.g = 63; color.b = 48;        /* concealed dark timber */
    } else if (feat == FEAT_RUBBLE) {
        color.r = 94; color.g = 84; color.b = 72;        /* broken soot-stone */
    } else if (feat >= FEAT_MAGMA && feat <= FEAT_QUARTZ_K) {
        color.r = 112; color.g = 92; color.b = 66;       /* ore seam masonry */
        if (feat == FEAT_MAGMA || feat == FEAT_MAGMA_H || feat == FEAT_MAGMA_K) {
            color.r = 116; color.g = 72; color.b = 48;   /* warmer iron/magma seam */
        }
    } else if (feat >= FEAT_PERM_EXTRA && feat <= FEAT_PERM_SOLID) {
        color.r = 128; color.g = 91; color.b = 50;       /* aged brass/solidwork */
    } else {
        switch (pattern) {
            case 1:
                color.r = 120; color.g = 88; color.b = 58; break;  /* oxidized masonry */
            case 2:
                color.r = 91; color.g = 75; color.b = 64; break;   /* soot stone */
            case 3:
                color.r = 108; color.g = 68; color.b = 45; break;  /* dark brick */
            default:
                color.r = 118; color.g = 98; color.b = 74; break;  /* warm masonry */
        }
    }
    color.a = 255;
    return color;
}

RendererColor renderer_depth_shade(RendererColor base, double distance, int side) {
    RendererColor out;
    double shade;
    double fog;
    const double fog_r = 18.0;
    const double fog_g = 14.0;
    const double fog_b = 12.0;

    if (distance <= 0.0 || distance != distance) distance = 0.1;

    shade = 1.0 / (1.0 + distance * 0.10);
    if (shade < 0.30) shade = 0.30;
    if (side == 1) shade *= 0.78;

    fog = distance / 22.0;
    if (fog > 0.42) fog = 0.42;
    if (fog < 0.0) fog = 0.0;

    out.r = renderer_clamp_channel((base.r * shade * (1.0 - fog)) + (fog_r * fog));
    out.g = renderer_clamp_channel((base.g * shade * (1.0 - fog)) + (fog_g * fog));
    out.b = renderer_clamp_channel((base.b * shade * (1.0 - fog)) + (fog_b * fog));
    out.a = base.a;
    return out;
}

static RendererColor renderer_scale_color(RendererColor base, double scale) {
    RendererColor out;

    out.r = renderer_clamp_channel((double)base.r * scale);
    out.g = renderer_clamp_channel((double)base.g * scale);
    out.b = renderer_clamp_channel((double)base.b * scale);
    out.a = base.a;
    return out;
}

static RendererColor renderer_brass_highlight(RendererColor base) {
    RendererColor out;

    out.r = renderer_clamp_channel((double)base.r * 0.70 + 78.0);
    out.g = renderer_clamp_channel((double)base.g * 0.70 + 58.0);
    out.b = renderer_clamp_channel((double)base.b * 0.70 + 28.0);
    out.a = base.a;
    return out;
}

RendererColor renderer_wall_detail_color(RendererColor shaded, const RendererRayHit* hit,
                                         int screen_x, int screen_y) {
    int seed = 0;
    int mortar;
    int seam;

    if (hit) {
        seed = abs(hit->map_x * 29 + hit->map_y * 43 + hit->side * 7);
    }

    if (screen_x < 0) screen_x = 0;
    if (screen_y < 0) screen_y = 0;

    mortar = (screen_y + seed) % 18;
    if (mortar == 0 || mortar == 1) {
        return renderer_scale_color(shaded, 0.62);
    }

    seam = (screen_x + seed) % 23;
    if (seam == 0) {
        return renderer_scale_color(shaded, 0.78);
    }

    if (((screen_x + seed * 3) % 47) == 0 && ((screen_y + seed) % 36) < 3) {
        return renderer_brass_highlight(shaded);
    }

    return shaded;
}

int renderer_marker_kind(byte feat, int has_monster, int has_object) {
    if (has_monster) return RENDERER_MARKER_MONSTER;
    if (has_object) return RENDERER_MARKER_OBJECT;
    if (feat == FEAT_LESS || feat == FEAT_MORE) return RENDERER_MARKER_STAIRS;
    if (feat >= FEAT_TRAP_HEAD && feat <= FEAT_TRAP_TAIL) return RENDERER_MARKER_TRAP;
    if (feat == FEAT_OPEN || (feat >= FEAT_DOOR_HEAD && feat <= FEAT_DOOR_TAIL)) {
        return RENDERER_MARKER_DOOR;
    }
    return RENDERER_MARKER_NONE;
}

RendererColor renderer_marker_color(int marker_kind) {
    RendererColor color;

    switch (marker_kind) {
        case RENDERER_MARKER_MONSTER:
            color.r = 194; color.g = 58; color.b = 44; break;
        case RENDERER_MARKER_OBJECT:
            color.r = 214; color.g = 165; color.b = 64; break;
        case RENDERER_MARKER_STAIRS:
            color.r = 80; color.g = 148; color.b = 184; break;
        case RENDERER_MARKER_DOOR:
            color.r = 156; color.g = 100; color.b = 48; break;
        case RENDERER_MARKER_TRAP:
            color.r = 176; color.g = 66; color.b = 148; break;
        default:
            color.r = 0; color.g = 0; color.b = 0; break;
    }
    color.a = 225;
    return color;
}

RendererMarkerProjection renderer_project_marker(const RendererContext* ctx,
                                                 double world_x, double world_y) {
    RendererMarkerProjection out;
    double rel_x, rel_y;
    double inv_det;
    double transform_x, transform_y;

    memset(&out, 0, sizeof(out));
    if (!ctx || ctx->width <= 0 || ctx->height <= 0) return out;

    inv_det = (ctx->planeX * ctx->dirY) - (ctx->dirX * ctx->planeY);
    if (fabs(inv_det) < 0.0001) return out;
    inv_det = 1.0 / inv_det;

    rel_x = world_x - ctx->posX;
    rel_y = world_y - ctx->posY;
    transform_x = inv_det * (ctx->dirY * rel_x - ctx->dirX * rel_y);
    transform_y = inv_det * (-ctx->planeY * rel_x + ctx->planeX * rel_y);
    if (transform_y <= 0.10) return out;

    out.screen_x = (int)((ctx->width / 2.0) * (1.0 + transform_x / transform_y));
    if (out.screen_x < -ctx->width / 4 || out.screen_x > ctx->width + ctx->width / 4) return out;

    out.size = (int)(ctx->height / transform_y / 5.0);
    if (out.size < 6) out.size = 6;
    if (out.size > ctx->height / 3) out.size = ctx->height / 3;
    out.top = (ctx->height / 2) - out.size;
    out.bottom = (ctx->height / 2) + out.size;
    if (out.top < 0) out.top = 0;
    if (out.bottom >= ctx->height) out.bottom = ctx->height - 1;
    out.depth = transform_y;
    out.visible = TRUE;
    return out;
}

int renderer_tile_category_from_values(byte feat, bool remembered,
                                       bool has_player, bool has_monster,
                                       bool has_object) {
    if (has_player) return RENDERER_TILE_PLAYER;
    if (!remembered) return RENDERER_TILE_DARKNESS;
    if (has_monster) return RENDERER_TILE_MONSTER;
    if (has_object) return RENDERER_TILE_OBJECT;
    if (feat == FEAT_LESS) return RENDERER_TILE_STAIRS_UP;
    if (feat == FEAT_MORE) return RENDERER_TILE_STAIRS_DN;
    if (feat >= FEAT_TRAP_HEAD && feat <= FEAT_TRAP_TAIL) return RENDERER_TILE_TRAP;
    if (feat == FEAT_OPEN || feat == FEAT_BROKEN ||
        (feat >= FEAT_DOOR_HEAD && feat <= FEAT_DOOR_TAIL)) {
        return RENDERER_TILE_DOOR;
    }
    if (feat >= FEAT_SECRET && feat <= FEAT_PERM_SOLID) return RENDERER_TILE_WALL;
    if (feat == FEAT_NONE) return RENDERER_TILE_DARKNESS;
    return RENDERER_TILE_FLOOR;
}

int renderer_monster_family_category_from_values(u32b flags3, char d_char) {
    if (flags3 & RF3_AUTOMATA) return RENDERER_TILE_MONSTER_AUTOMATA;
    if (flags3 & (RF3_UNDEAD | RF3_DEMON)) return RENDERER_TILE_MONSTER_UNDEAD;
    if (flags3 & (RF3_ANIMAL | RF3_DRAGON | RF3_ALIEN | RF3_BEASTMAN |
                  RF3_TROLL | RF3_GIANT)) {
        return RENDERER_TILE_MONSTER_BEAST;
    }

    if (strchr("pht", d_char)) return RENDERER_TILE_MONSTER_HUMANOID;
    return RENDERER_TILE_MONSTER;
}

static bool renderer_monster_race_for_grid(int y, int x, monster_race** race_out) {
    s16b m_idx;
    s16b r_idx;

    if (race_out) *race_out = NULL;
    if (!race_out || !cave_m_idx || !m_list || !r_info || !z_info) return FALSE;
    if (y < 0 || x < 0 || y >= DUNGEON_HGT || x >= DUNGEON_WID) return FALSE;

    m_idx = cave_m_idx[y][x];
    if (m_idx <= 0 || m_idx >= m_max || m_idx >= (s16b)z_info->m_max) return FALSE;
    if (!m_list[m_idx].ml) return FALSE;

    r_idx = m_list[m_idx].r_idx;
    if (r_idx <= 0 || r_idx >= (s16b)z_info->r_max) return FALSE;

    *race_out = &r_info[r_idx];
    return TRUE;
}

RendererTileInfo renderer_classify_tile(int y, int x) {
    RendererTileInfo info;
    monster_race* r_ptr = NULL;

    memset(&info, 0, sizeof(info));
    info.category = RENDERER_TILE_DARKNESS;
    info.feat = FEAT_NONE;

    if (y < 0 || x < 0 || y >= DUNGEON_HGT || x >= DUNGEON_WID) {
        return info;
    }

    info.in_bounds = TRUE;
    info.feat = renderer_feature_at(y, x);
    info.remembered = TRUE;
    if (cave_info) {
        info.remembered = (cave_info[y][x] & (CAVE_MARK | CAVE_SEEN)) ? TRUE : FALSE;
    }
    info.has_player = (p_ptr && p_ptr->py == y && p_ptr->px == x) ? TRUE : FALSE;
    info.has_monster = renderer_monster_race_for_grid(y, x, &r_ptr);
    info.has_object = (cave_o_idx && cave_o_idx[y][x] != 0) ? TRUE : FALSE;
    info.category = renderer_tile_category_from_values(info.feat, info.remembered,
                                                       info.has_player, info.has_monster,
                                                       info.has_object);
    if (info.category == RENDERER_TILE_MONSTER && r_ptr) {
        char display_char = r_ptr->x_char ? r_ptr->x_char : r_ptr->d_char;
        info.category = renderer_monster_family_category_from_values(r_ptr->flags3, display_char);
    }
    return info;
}

RendererTileViewport renderer_tile_viewport(const RendererContext* ctx, int tile_size) {
    RendererTileViewport view;
    int center_x = 5;
    int center_y = 5;

    memset(&view, 0, sizeof(view));
    if (!ctx || ctx->width <= 0 || ctx->height <= 0) return view;

    if (tile_size <= 0) {
        int by_width = ctx->width / 41;
        int by_height = (ctx->height - 48) / 25;
        tile_size = (by_width < by_height) ? by_width : by_height;
    }
    if (tile_size < 8) tile_size = 8;
    if (tile_size > 32) tile_size = 32;

    view.tile_size = tile_size;
    view.cols = ctx->width / tile_size;
    view.rows = (ctx->height - 48) / tile_size;
    if (view.cols < 9) view.cols = 9;
    if (view.rows < 7) view.rows = 7;
    if (view.cols > DUNGEON_WID) view.cols = DUNGEON_WID;
    if (view.rows > DUNGEON_HGT) view.rows = DUNGEON_HGT;
    view.pixel_width = view.cols * tile_size;
    view.pixel_height = view.rows * tile_size;

    if (p_ptr) {
        center_x = p_ptr->px;
        center_y = p_ptr->py;
    } else {
        center_x = (int)ctx->posX;
        center_y = (int)ctx->posY;
    }

    view.origin_x = center_x - view.cols / 2;
    view.origin_y = center_y - view.rows / 2;
    if (view.origin_x < 0) view.origin_x = 0;
    if (view.origin_y < 0) view.origin_y = 0;
    if (view.origin_x + view.cols > DUNGEON_WID) view.origin_x = DUNGEON_WID - view.cols;
    if (view.origin_y + view.rows > DUNGEON_HGT) view.origin_y = DUNGEON_HGT - view.rows;
    if (view.origin_x < 0) view.origin_x = 0;
    if (view.origin_y < 0) view.origin_y = 0;
    return view;
}

RendererColor renderer_tile_color(int category) {
    RendererColor color;

    switch (category) {
        case RENDERER_TILE_PLAYER:
            color.r = 214; color.g = 190; color.b = 116; break;
        case RENDERER_TILE_MONSTER:
            color.r = 170; color.g = 56; color.b = 48; break;
        case RENDERER_TILE_MONSTER_AUTOMATA:
            color.r = 142; color.g = 122; color.b = 80; break;
        case RENDERER_TILE_MONSTER_UNDEAD:
            color.r = 116; color.g = 84; color.b = 146; break;
        case RENDERER_TILE_MONSTER_BEAST:
            color.r = 150; color.g = 88; color.b = 52; break;
        case RENDERER_TILE_MONSTER_HUMANOID:
            color.r = 154; color.g = 92; color.b = 70; break;
        case RENDERER_TILE_OBJECT:
            color.r = 188; color.g = 142; color.b = 66; break;
        case RENDERER_TILE_WALL:
            color.r = 96; color.g = 88; color.b = 78; break;
        case RENDERER_TILE_DOOR:
            color.r = 126; color.g = 82; color.b = 48; break;
        case RENDERER_TILE_STAIRS_UP:
            color.r = 92; color.g = 132; color.b = 142; break;
        case RENDERER_TILE_STAIRS_DN:
            color.r = 60; color.g = 102; color.b = 132; break;
        case RENDERER_TILE_TRAP:
            color.r = 138; color.g = 62; color.b = 116; break;
        case RENDERER_TILE_FLOOR:
            color.r = 50; color.g = 48; color.b = 42; break;
        case RENDERER_TILE_DARKNESS:
        default:
            color.r = 13; color.g = 12; color.b = 11; break;
    }
    color.a = 255;
    return color;
}

RendererTopDownTilesetSpec renderer_default_top_down_tileset_spec(void) {
    RendererTopDownTilesetSpec spec;
    int i;

    memset(&spec, 0, sizeof(spec));
    spec.tile_width = 24;
    spec.tile_height = 24;
    spec.columns = 7;
    spec.rows = 2;
    for (i = 0; i < RENDERER_TILE_CATEGORY_COUNT; i++) {
        spec.category_to_tile[i] = i;
    }
    return spec;
}

int renderer_top_down_tile_index(const RendererTopDownTilesetSpec* spec, int category) {
    if (!spec) return 0;
    if (category < 0 || category >= RENDERER_TILE_CATEGORY_COUNT) return 0;
    if (spec->category_to_tile[category] < 0) return 0;
    if (spec->columns <= 0 || spec->rows <= 0) return 0;
    if (spec->category_to_tile[category] >= spec->columns * spec->rows) return 0;
    return spec->category_to_tile[category];
}

SDL_Rect renderer_top_down_source_rect(const RendererTopDownTilesetSpec* spec, int category) {
    SDL_Rect rect;
    int tile_index;

    memset(&rect, 0, sizeof(rect));
    if (!spec || spec->tile_width <= 0 || spec->tile_height <= 0 || spec->columns <= 0) {
        return rect;
    }

    tile_index = renderer_top_down_tile_index(spec, category);
    rect.x = (tile_index % spec->columns) * spec->tile_width;
    rect.y = (tile_index / spec->columns) * spec->tile_height;
    rect.w = spec->tile_width;
    rect.h = spec->tile_height;
    return rect;
}

bool renderer_load_top_down_tilesheet(RendererContext* ctx) {
    SDL_Surface* surface = NULL;
    const char* override_path;
    int i;

    if (!ctx || !ctx->renderer) return FALSE;
    if (ctx->top_down_tilesheet) {
        ctx->top_down_tiles_loaded = TRUE;
        return TRUE;
    }

    override_path = SDL_getenv("STEAMBAND_TOPDOWN_TILESET");
    if (override_path && override_path[0]) {
        surface = SDL_LoadBMP(override_path);
        if (surface) {
            LOG_I("Loaded SDL2 top-down tilesheet override: %s", override_path);
        } else {
            LOG_W("SDL2 top-down tilesheet override failed: %s", override_path);
        }
    }

    for (i = 0; !surface && g_top_down_tilesheet_candidates[i]; i++) {
        surface = SDL_LoadBMP(g_top_down_tilesheet_candidates[i]);
        if (surface) {
            LOG_I("Loaded SDL2 top-down tilesheet: %s", g_top_down_tilesheet_candidates[i]);
            break;
        }
    }

    if (!surface) {
        ctx->top_down_tiles_loaded = FALSE;
        LOG_W("SDL2 top-down tilesheet not found; using procedural tile glyph fallback.");
        return FALSE;
    }

    SDL_SetColorKey(surface, SDL_TRUE, SDL_MapRGB(surface->format, 255, 0, 255));
    ctx->top_down_tilesheet = SDL_CreateTextureFromSurface(ctx->renderer, surface);
    SDL_FreeSurface(surface);

    if (!ctx->top_down_tilesheet) {
        ctx->top_down_tiles_loaded = FALSE;
        LOG_W("SDL2 top-down tilesheet texture creation failed: %s", SDL_GetError());
        return FALSE;
    }

    SDL_SetTextureBlendMode(ctx->top_down_tilesheet, SDL_BLENDMODE_BLEND);
    ctx->top_down_tiles_loaded = TRUE;
    return TRUE;
}

static void renderer_draw_world_markers(RendererContext* ctx) {
    int cy, cx;
    int radius = 8;

    if (!ctx || !ctx->renderer) return;

    SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_BLEND);
    for (cy = (int)ctx->posY - radius; cy <= (int)ctx->posY + radius; cy++) {
        for (cx = (int)ctx->posX - radius; cx <= (int)ctx->posX + radius; cx++) {
            int has_monster = 0;
            int has_object = 0;
            int kind;
            byte feat;
            RendererMarkerProjection marker;
            RendererColor color;
            SDL_Rect rect;

            if (cy < 0 || cx < 0 || cy >= DUNGEON_HGT || cx >= DUNGEON_WID) continue;

            feat = renderer_feature_at(cy, cx);
            if (cave_m_idx && cave_m_idx[cy][cx] > 0) has_monster = TRUE;
            if (cave_o_idx && cave_o_idx[cy][cx] != 0) has_object = TRUE;
            kind = renderer_marker_kind(feat, has_monster, has_object);
            if (kind == RENDERER_MARKER_NONE) continue;

            marker = renderer_project_marker(ctx, (double)cx + 0.5, (double)cy + 0.5);
            if (!marker.visible) continue;

            color = renderer_marker_color(kind);
            rect.w = marker.size;
            rect.h = marker.bottom - marker.top;
            if (rect.h < 4) rect.h = 4;
            rect.x = marker.screen_x - rect.w / 2;
            rect.y = marker.top;
            SDL_SetRenderDrawColor(ctx->renderer, color.r, color.g, color.b, color.a);
            SDL_RenderFillRect(ctx->renderer, &rect);

            rect.x += rect.w / 4;
            rect.w = rect.w / 2;
            rect.y = marker.top + rect.h / 4;
            rect.h = rect.h / 2;
            SDL_SetRenderDrawColor(ctx->renderer, 20, 16, 12, 130);
            SDL_RenderFillRect(ctx->renderer, &rect);
        }
    }
    SDL_SetRenderDrawBlendMode(ctx->renderer, SDL_BLENDMODE_NONE);
}

static void renderer_draw_tile_glyph(RendererContext* ctx, const SDL_Rect* rect,
                                     int category, RendererColor base) {
    SDL_Rect inner;
    int mid_x;
    int mid_y;

    if (!ctx || !ctx->renderer || !rect) return;

    inner = *rect;
    if (inner.w > 4 && inner.h > 4) {
        inner.x += 2;
        inner.y += 2;
        inner.w -= 4;
        inner.h -= 4;
    }
    mid_x = rect->x + rect->w / 2;
    mid_y = rect->y + rect->h / 2;

    SDL_SetRenderDrawColor(ctx->renderer, 22, 19, 16, 210);
    SDL_RenderDrawRect(ctx->renderer, rect);

    switch (category) {
        case RENDERER_TILE_WALL:
            SDL_SetRenderDrawColor(ctx->renderer,
                                   renderer_clamp_channel(base.r * 0.72),
                                   renderer_clamp_channel(base.g * 0.72),
                                   renderer_clamp_channel(base.b * 0.72), 255);
            SDL_RenderDrawLine(ctx->renderer, rect->x + 2, mid_y, rect->x + rect->w - 3, mid_y);
            SDL_RenderDrawLine(ctx->renderer, mid_x, rect->y + 2, mid_x, rect->y + rect->h - 3);
            break;
        case RENDERER_TILE_DOOR:
            SDL_SetRenderDrawColor(ctx->renderer, 36, 24, 16, 230);
            SDL_RenderDrawLine(ctx->renderer, mid_x, rect->y + 2, mid_x, rect->y + rect->h - 3);
            SDL_RenderDrawRect(ctx->renderer, &inner);
            break;
        case RENDERER_TILE_STAIRS_UP:
        case RENDERER_TILE_STAIRS_DN:
            SDL_SetRenderDrawColor(ctx->renderer, 26, 32, 34, 230);
            for (int i = 3; i < rect->h - 2; i += 4) {
                SDL_RenderDrawLine(ctx->renderer, rect->x + 3, rect->y + i,
                                   rect->x + rect->w - 4, rect->y + i);
            }
            break;
        case RENDERER_TILE_TRAP:
            SDL_SetRenderDrawColor(ctx->renderer, 32, 20, 30, 230);
            SDL_RenderDrawLine(ctx->renderer, rect->x + 3, rect->y + 3,
                               rect->x + rect->w - 4, rect->y + rect->h - 4);
            SDL_RenderDrawLine(ctx->renderer, rect->x + rect->w - 4, rect->y + 3,
                               rect->x + 3, rect->y + rect->h - 4);
            break;
        case RENDERER_TILE_OBJECT:
            SDL_SetRenderDrawColor(ctx->renderer, 42, 30, 16, 230);
            SDL_RenderDrawRect(ctx->renderer, &inner);
            break;
        case RENDERER_TILE_MONSTER:
            SDL_SetRenderDrawColor(ctx->renderer, 40, 16, 14, 240);
            SDL_RenderDrawLine(ctx->renderer, rect->x + 3, mid_y, mid_x, rect->y + 3);
            SDL_RenderDrawLine(ctx->renderer, mid_x, rect->y + 3, rect->x + rect->w - 4, mid_y);
            SDL_RenderDrawLine(ctx->renderer, rect->x + 3, mid_y, mid_x, rect->y + rect->h - 4);
            SDL_RenderDrawLine(ctx->renderer, mid_x, rect->y + rect->h - 4, rect->x + rect->w - 4, mid_y);
            break;
        case RENDERER_TILE_PLAYER:
            SDL_SetRenderDrawColor(ctx->renderer, 42, 32, 14, 255);
            SDL_RenderDrawLine(ctx->renderer, mid_x, rect->y + 3, rect->x + 3, rect->y + rect->h - 4);
            SDL_RenderDrawLine(ctx->renderer, mid_x, rect->y + 3, rect->x + rect->w - 4, rect->y + rect->h - 4);
            SDL_RenderDrawLine(ctx->renderer, rect->x + 5, mid_y, rect->x + rect->w - 6, mid_y);
            break;
        case RENDERER_TILE_FLOOR:
            SDL_SetRenderDrawColor(ctx->renderer, 74, 69, 58, 130);
            SDL_RenderDrawPoint(ctx->renderer, rect->x + 3, rect->y + 3);
            SDL_RenderDrawPoint(ctx->renderer, rect->x + rect->w - 4, rect->y + rect->h - 4);
            break;
        default:
            break;
    }
}

static void renderer_render_top_down(RendererContext* ctx) {
    RendererHudSnapshot hud;
    RendererTileViewport view;
    int offset_x;
    int offset_y;

    if (!ctx || !ctx->renderer || !ctx->top_down_mode) return;

    renderer_sync_from_player(ctx);
    view = renderer_tile_viewport(ctx, 0);
    offset_x = (ctx->width - view.pixel_width) / 2;
    if (offset_x < 0) offset_x = 0;
    offset_y = 10;

    SDL_SetRenderDrawColor(ctx->renderer, 10, 9, 8, 255);
    SDL_RenderClear(ctx->renderer);

    for (int row = 0; row < view.rows; row++) {
        for (int col = 0; col < view.cols; col++) {
            int cy = view.origin_y + row;
            int cx = view.origin_x + col;
            RendererTileInfo info = renderer_classify_tile(cy, cx);
            RendererColor color;
            SDL_Rect rect;

            if (!p_ptr && cy == (int)ctx->posY && cx == (int)ctx->posX) {
                info.category = RENDERER_TILE_PLAYER;
            }

            color = renderer_tile_color(info.category);
            rect.x = offset_x + col * view.tile_size;
            rect.y = offset_y + row * view.tile_size;
            rect.w = view.tile_size;
            rect.h = view.tile_size;

            if (ctx->top_down_tiles_loaded && ctx->top_down_tilesheet) {
                SDL_Rect src = renderer_top_down_source_rect(&ctx->top_down_tileset, info.category);
                SDL_RenderCopy(ctx->renderer, ctx->top_down_tilesheet, &src, &rect);
            } else {
                SDL_SetRenderDrawColor(ctx->renderer, color.r, color.g, color.b, color.a);
                SDL_RenderFillRect(ctx->renderer, &rect);
                renderer_draw_tile_glyph(ctx, &rect, info.category, color);
            }
        }
    }

    hud = renderer_collect_hud_snapshot(ctx);
    if (ctx->window) {
        char top_down_title[160];
        snprintf(top_down_title, sizeof(top_down_title),
                 "SteambandRedux 2D Tiles - HP %d/%d SP %d/%d %s %s",
                 hud.current_hp, hud.max_hp, hud.current_sp, hud.max_sp,
                 hud.depth_label, hud.status_label);
        top_down_title[sizeof(top_down_title) - 1] = '\0';
        SDL_SetWindowTitle(ctx->window, top_down_title);
    }
    renderer_draw_hud_status(ctx, &hud);
    renderer_draw_hud_hint(ctx);
    SDL_RenderPresent(ctx->renderer);
}

bool renderer_should_forward_key_event(const RendererContext* ctx) {
    return (ctx && (ctx->first_person_mode || ctx->top_down_mode) && ctx->keyboard_focus) ? TRUE : FALSE;
}

bool renderer_texture_loading_allowed(const RendererContext* ctx) {
    return (ctx && ctx->textures_approved) ? TRUE : FALSE;
}

int renderer_trace_column(const RendererContext* ctx, int screen_x, RendererRayHit* hit) {
    double cameraX;
    double rayDirX;
    double rayDirY;
    RendererRayHit ray_hit;
    RendererWallStrip strip;

    if (!ctx || !hit || ctx->width <= 0 || ctx->height <= 0) return 1;
    if (screen_x < 0 || screen_x >= ctx->width) return 1;

    cameraX = 2.0 * screen_x / (double)ctx->width - 1.0;
    rayDirX = ctx->dirX + ctx->planeX * cameraX;
    rayDirY = ctx->dirY + ctx->planeY * cameraX;
    ray_hit = renderer_cast_ray(ctx->posX, ctx->posY, rayDirX, rayDirY, 0);
    strip = renderer_wall_strip(ctx->height, ray_hit.distance);

    ray_hit.line_height = strip.line_height;
    ray_hit.draw_start = strip.draw_start;
    ray_hit.draw_end = strip.draw_end;
    *hit = ray_hit;
    return 0;
}

/* Core DDA raycasting render function.
 * For each screen column: calculate ray, DDA step through map (using renderer_is_wall),
 * compute wall height with perspective correction (1 / perpWallDist), draw vertical strip.
 * Simple floor/ceiling filled with colors (dark floor for dungeon, warm ceiling for steampunk gaslight).
 * Prepares wallX for texture coord (future u = wallX * TEX_WIDTH).
 */
void renderer_render(RendererContext* ctx) {
    RendererHudSnapshot hud;

    if (!ctx || !ctx->renderer) {
        return;
    }

    if (ctx->top_down_mode) {
        renderer_render_top_down(ctx);
        return;
    }

    if (!ctx->first_person_mode) return;

    SDL_SetRenderDrawColor(ctx->renderer, 0, 0, 0, 255);  /* Clear to black */
    SDL_RenderClear(ctx->renderer);
    renderer_draw_atmosphere(ctx);

    /* Raycasting loop - one ray per x column */
    for (int x = 0; x < ctx->width; x++) {
        /* Calculate ray position and direction */
        double cameraX = 2 * x / (double)ctx->width - 1;  /* x in camera space */
        double rayDirX = ctx->dirX + ctx->planeX * cameraX;
        double rayDirY = ctx->dirY + ctx->planeY * cameraX;

        RendererRayHit hit = renderer_cast_ray(ctx->posX, ctx->posY, rayDirX, rayDirY, 0);
        RendererWallStrip strip = renderer_wall_strip(ctx->height, hit.distance);

        RendererColor color = renderer_depth_shade(renderer_wall_base_color(&hit), hit.distance, hit.side);

        /* Draw the vertical wall strip with perspective */
        SDL_SetRenderDrawColor(ctx->renderer, color.r, color.g, color.b, color.a);
        SDL_RenderDrawLine(ctx->renderer, x, strip.draw_start, x, strip.draw_end);

        /* Sparse procedural platework gives a 90s texture read without shipping art assets. */
        for (int y = strip.draw_start; y <= strip.draw_end; y++) {
            RendererColor detail = renderer_wall_detail_color(color, &hit, x, y);
            if (detail.r != color.r || detail.g != color.g || detail.b != color.b || detail.a != color.a) {
                SDL_SetRenderDrawColor(ctx->renderer, detail.r, detail.g, detail.b, detail.a);
                SDL_RenderDrawPoint(ctx->renderer, x, y);
            }
        }

        /* Future: texture mapping would sample from wall_textures[texNum] at (texX, texY)
         * where texX = (int)(wallX * TEX_WIDTH), wallX from hit position fractional.
         * e.g. double wallX; if (side==0) wallX = ctx->posY + perpWallDist*rayDirY; else ...
         * wallX -= floor(wallX); texX = (int)(wallX * TEX_WIDTH);
         */
    }

    /* Optional minimap for debugging; disabled by default to preserve immersion. */
    if (ctx->show_debug_minimap) {
        SDL_SetRenderDrawColor(ctx->renderer, 255, 255, 255, 100);
        for (int y = 0; y < 16; y++) {
            for (int x = 0; x < 16; x++) {
                if (renderer_is_wall(y * (DUNGEON_HGT/16), x * (DUNGEON_WID/16))) {
                    SDL_Rect mini = {10 + x*4, 10 + y*4, 4, 4};
                    SDL_RenderFillRect(ctx->renderer, &mini);
                }
            }
        }
        /* Player pos on minimap */
        SDL_SetRenderDrawColor(ctx->renderer, 0, 255, 0, 255);
        SDL_Rect player_dot = {10 + (int)(ctx->posX * 4 / 16 * 16), 10 + (int)(ctx->posY * 4 / 16 * 16), 4, 4};
        SDL_RenderFillRect(ctx->renderer, &player_dot);
    }

    renderer_draw_world_markers(ctx);
    hud = renderer_collect_hud_snapshot(ctx);
    if (ctx->window) SDL_SetWindowTitle(ctx->window, hud.title);
    renderer_draw_vignette(ctx);
    renderer_draw_hud_status(ctx, &hud);
    renderer_draw_hud_hint(ctx);
    SDL_RenderPresent(ctx->renderer);

}

void renderer_toggle_mode(RendererContext* ctx) {
    if (!ctx) return;
    ctx->first_person_mode = !ctx->first_person_mode;
    if (ctx->first_person_mode) ctx->top_down_mode = FALSE;
    g_use_2d_fallback = (!ctx->first_person_mode && !ctx->top_down_mode) ? TRUE : FALSE;
    if (ctx->first_person_mode) {
        ctx->keyboard_focus = TRUE;
        renderer_sync_from_player(ctx);
        if (ctx->window) {
            SDL_SetWindowTitle(ctx->window,
                               "SteambandRedux FP - W/Up forward, S/Down back, A/D strafe, Arrows turn, Esc exits");
            SDL_ShowWindow(ctx->window);
            SDL_RaiseWindow(ctx->window);
        }
        LOG_I("First-person mode activated: W/Up forward, S/Down back, A/D strafe, arrows turn, right stick turns, Esc exits.");
    } else {
        ctx->keyboard_focus = FALSE;
        if (ctx->window) SDL_HideWindow(ctx->window);
        LOG_I("Switched to 2D fallback mode.");
    }
}

void renderer_toggle_top_down_mode(RendererContext* ctx) {
    if (!ctx) return;
    ctx->top_down_mode = !ctx->top_down_mode;
    if (ctx->top_down_mode) ctx->first_person_mode = FALSE;
    g_use_2d_fallback = (!ctx->first_person_mode && !ctx->top_down_mode) ? TRUE : FALSE;
    if (ctx->top_down_mode) {
        ctx->keyboard_focus = TRUE;
        renderer_sync_from_player(ctx);
        if (!ctx->top_down_tiles_loaded) {
            renderer_load_top_down_tilesheet(ctx);
        }
        if (ctx->window) {
            SDL_SetWindowTitle(ctx->window,
                               "SteambandRedux 2D Tiles - Ctrl+F11/Esc exits, keyboard commands forward");
            SDL_ShowWindow(ctx->window);
            SDL_RaiseWindow(ctx->window);
        }
        LOG_I("Top-down SDL tile mode activated: original/permissive no-asset pencil tiles, Ctrl+F11 or Esc exits.");
    } else {
        ctx->keyboard_focus = FALSE;
        if (ctx->window) SDL_HideWindow(ctx->window);
        LOG_I("Switched to legacy 2D fallback mode from top-down SDL tiles.");
    }
}

/* Global instance for minimal integration */
static RendererContext g_renderer;

/* For external access (e.g. from main-win.c) */
RendererContext* get_renderer(void) {
    return &g_renderer;
}
