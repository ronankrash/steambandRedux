/* File: src/renderer.h
 * SDL2-based DDA raycaster for first-person steampunk dungeon view.
 * Uses existing cave/map data from cave.c (cave_feat[][] for walls).
 * Vertical wall strips with perspective correction, simple floor/ceiling.
 * Keeps texture slots empty until assets are approved and documented.
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

#define RENDERER_MOVE_FORWARD  1
#define RENDERER_MOVE_BACKWARD 2
#define RENDERER_MOVE_LEFT     3
#define RENDERER_MOVE_RIGHT    4

typedef struct {
    bool hit;
    int map_x;
    int map_y;
    int side;
    int steps;
    double distance;
    int line_height;
    int draw_start;
    int draw_end;
} RendererRayHit;

typedef struct {
    int line_height;
    int draw_start;
    int draw_end;
} RendererWallStrip;

typedef struct {
    Uint8 r;
    Uint8 g;
    Uint8 b;
    Uint8 a;
} RendererColor;

/* Renderer context - extensible for approved textures */
typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* screen_texture;  /* For offscreen rendering if needed */
    int width;
    int height;
    bool first_person_mode;
    bool keyboard_focus;
    bool show_debug_minimap;

    /* Raycasting camera */
    double posX;
    double posY;
    double dirX;
    double dirY;
    double planeX;
    double planeY;

    /* Texture slots remain empty until assets are documented in ASSETS.md. */
    SDL_Texture* wall_textures[8];
    bool textures_approved;
    bool textures_loaded;
} RendererContext;

/* Public API */
bool renderer_init(RendererContext* ctx);
void renderer_shutdown(RendererContext* ctx);
int renderer_handle_events(RendererContext* ctx, int max_events);
void renderer_render(RendererContext* ctx);
void renderer_toggle_mode(RendererContext* ctx);
void renderer_sync_from_player(RendererContext* ctx);  /* Syncs from p_ptr if available */
void renderer_rotate(RendererContext* ctx, double radians);  /* Rotate camera direction and plane */

/* Test/utility functions */
bool renderer_is_wall(int y, int x);  /* Uses cave data or test map */
void renderer_test_dda(void);  /* For unit tests */
double renderer_safe_perp_distance(int side, int map_x, int map_y,
                                   double pos_x, double pos_y,
                                   int step_x, int step_y,
                                   double ray_dir_x, double ray_dir_y);
RendererRayHit renderer_cast_ray(double pos_x, double pos_y,
                                 double ray_dir_x, double ray_dir_y,
                                 int max_steps);
RendererWallStrip renderer_wall_strip(int screen_height, double distance);
int renderer_trace_column(const RendererContext* ctx, int screen_x, RendererRayHit* hit);
void renderer_clamp_viewport(RendererContext* ctx, int width, int height);
RendererColor renderer_atmosphere_color(int y, int height);
RendererColor renderer_wall_base_color(const RendererRayHit* hit);
RendererColor renderer_depth_shade(RendererColor base, double distance, int side);
int renderer_direction_to_command(double dx, double dy);
int renderer_camera_move_command(const RendererContext* ctx, int move);
int renderer_first_person_key_to_command(RendererContext* ctx, SDL_Keycode key, SDL_Keymod mod);
bool renderer_should_forward_key_event(const RendererContext* ctx);
bool renderer_texture_loading_allowed(const RendererContext* ctx);
int renderer_key_to_command(SDL_Keycode key, SDL_Keymod mod);
RendererContext* get_renderer(void);

/* 2D fallback indicator */
extern bool g_use_2d_fallback;

#endif /* RENDERER_H */
