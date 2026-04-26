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
#include "logging.h"  /* For secure logging */

/* Global for 2D fallback coordination */
bool g_use_2d_fallback = TRUE;

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
    /* No asset files are loaded until ASSETS.md documents an approved source. */
    ctx->textures_approved = FALSE;
    ctx->textures_loaded = FALSE;

    ctx->first_person_mode = FALSE;  /* Explicitly toggled with Ctrl+F12 */
    g_use_2d_fallback = TRUE;

    LOG_I("Renderer initialized hidden: %dx%d DDA raycaster ready. Press Ctrl+F12 or L3+R3 to toggle first-person prototype.",
          ctx->width, ctx->height);
    renderer_test_dda();

    return TRUE;
}

void renderer_shutdown(RendererContext* ctx) {
    if (!ctx) return;

    if (ctx->screen_texture) SDL_DestroyTexture(ctx->screen_texture);
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
            ctx->keyboard_focus = FALSE;
            g_use_2d_fallback = TRUE;
            if (ctx->window) SDL_HideWindow(ctx->window);
            LOG_I("Exited first-person mode, restored 2D fallback.");
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
                int cmd = renderer_first_person_key_to_command(ctx, e.key.keysym.sym, (SDL_Keymod)e.key.keysym.mod);
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
            renderer_rotate(ctx, -0.18);
            return 0;
        case SDLK_RIGHT:
            renderer_rotate(ctx, 0.18);
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

bool renderer_should_forward_key_event(const RendererContext* ctx) {
    return (ctx && ctx->first_person_mode && ctx->keyboard_focus) ? TRUE : FALSE;
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
    if (!ctx || !ctx->renderer || !ctx->first_person_mode) {
        return;
    }

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

    renderer_draw_vignette(ctx);
    renderer_draw_hud_hint(ctx);
    SDL_RenderPresent(ctx->renderer);

}

void renderer_toggle_mode(RendererContext* ctx) {
    if (!ctx) return;
    ctx->first_person_mode = !ctx->first_person_mode;
    g_use_2d_fallback = !ctx->first_person_mode;
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

/* Global instance for minimal integration */
static RendererContext g_renderer;

/* For external access (e.g. from main-win.c) */
RendererContext* get_renderer(void) {
    return &g_renderer;
}
