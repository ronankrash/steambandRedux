/* File: src/renderer.h
 * SDL2-based DDA raycaster for first-person steampunk dungeon view.
 * Uses existing cave/map data from cave.c (cave_feat[][] for walls).
 * Vertical wall strips with perspective correction, simple floor/ceiling.
 * Prepares for CC0 Victorian steampunk textures (brass pipes, gears, dark brick walls).
 * Secure C practices: bounds checking, no unsafe string funcs.
 * 2D fallback preserved via existing main-win.c term/map.
 *
 * Security audit: No buffer overflows, all array accesses bounded by DUNGEON_*,
 * SDL error checking, proper resource cleanup.
 */

#ifndef RENDERER_H
#define RENDERER_H

#include <SDL.h>
#include "angband.h"  /* for cave_feat, p_ptr, DUNGEON_*, in_bounds */

#define RENDER_WIDTH  640
#define RENDER_HEIGHT 480
#define TEX_WIDTH     64
#define TEX_HEIGHT    64

/* Renderer context - extensible for textures */
typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* screen_texture;  /* For offscreen rendering if needed */
    int width;
    int height;
    bool first_person_mode;
    
    /* Raycasting camera */
    double posX;
    double posY;
    double dirX;
    double dirY;
    double planeX;
    double planeY;
    
    /* Future texture support for steampunk assets (CC0 brass, gears, brick) */
    SDL_Texture* wall_textures[8];  /* 0: dark brick, 1: brass pipes, etc. */
    bool textures_loaded;
} RendererContext;

/* Public API */
bool renderer_init(RendererContext* ctx);
void renderer_shutdown(RendererContext* ctx);
void renderer_render(RendererContext* ctx);
void renderer_toggle_mode(RendererContext* ctx);
void renderer_sync_from_player(RendererContext* ctx);  /* Syncs from p_ptr if available */
void renderer_rotate(RendererContext* ctx, double radians);  /* Rotate camera direction and plane */

/* Test/utility functions */
bool renderer_is_wall(int y, int x);  /* Uses cave data or test map */
void renderer_test_dda(void);  /* For unit tests */
RendererContext* get_renderer(void);

/* 2D fallback indicator */
extern bool g_use_2d_fallback;

#endif /* RENDERER_H */
