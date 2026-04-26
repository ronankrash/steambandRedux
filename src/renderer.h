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

#include <stddef.h>
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

#define RENDERER_MARKER_NONE    0
#define RENDERER_MARKER_MONSTER 1
#define RENDERER_MARKER_OBJECT  2
#define RENDERER_MARKER_STAIRS  3
#define RENDERER_MARKER_DOOR    4
#define RENDERER_MARKER_TRAP    5

#define RENDERER_TILE_DARKNESS  0
#define RENDERER_TILE_FLOOR     1
#define RENDERER_TILE_WALL      2
#define RENDERER_TILE_DOOR      3
#define RENDERER_TILE_STAIRS_UP 4
#define RENDERER_TILE_STAIRS_DN 5
#define RENDERER_TILE_TRAP      6
#define RENDERER_TILE_GLYPH     7
#define RENDERER_TILE_SHOP      8
#define RENDERER_TILE_SHOP_GENERAL     9
#define RENDERER_TILE_SHOP_CLOTHING    10
#define RENDERER_TILE_SHOP_GUN         11
#define RENDERER_TILE_SHOP_MACHINIST   12
#define RENDERER_TILE_SHOP_ALCHEMY     13
#define RENDERER_TILE_SHOP_MAGIC       14
#define RENDERER_TILE_SHOP_BLACK_MARKET 15
#define RENDERER_TILE_SHOP_HOME        16
#define RENDERER_TILE_RUBBLE    17
#define RENDERER_TILE_ORE       18
#define RENDERER_TILE_OBJECT    19
#define RENDERER_TILE_OBJECT_FOOD      20
#define RENDERER_TILE_OBJECT_SCROLL    21
#define RENDERER_TILE_OBJECT_POTION    22
#define RENDERER_TILE_OBJECT_WEAPON    23
#define RENDERER_TILE_OBJECT_ARMOR     24
#define RENDERER_TILE_OBJECT_GUN       25
#define RENDERER_TILE_OBJECT_AMMO      26
#define RENDERER_TILE_OBJECT_MONEY     27
#define RENDERER_TILE_OBJECT_JEWELRY   28
#define RENDERER_TILE_OBJECT_DEVICE    29
#define RENDERER_TILE_MONSTER   30
#define RENDERER_TILE_MONSTER_AUTOMATA 31
#define RENDERER_TILE_MONSTER_UNDEAD   32
#define RENDERER_TILE_MONSTER_BEAST    33
#define RENDERER_TILE_MONSTER_HUMANOID 34
#define RENDERER_TILE_PLAYER    35
#define RENDERER_TILE_CATEGORY_COUNT 36

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

typedef struct {
    int current_hp;
    int max_hp;
    int current_sp;
    int max_sp;
    int depth;
    bool use_feet_depth;
    bool keyboard_focus;
    bool has_message;
    bool blind;
    bool confused;
    bool poisoned;
    bool afraid;
    bool cut;
    bool stunned;
    char depth_label[16];
    char status_label[32];
    char last_message[80];
    char title[160];
} RendererHudSnapshot;

typedef struct {
    bool visible;
    int screen_x;
    int top;
    int bottom;
    int size;
    double depth;
} RendererMarkerProjection;

typedef struct {
    int category;
    byte feat;
    bool in_bounds;
    bool remembered;
    bool has_player;
    bool has_monster;
    bool has_object;
} RendererTileInfo;

typedef struct {
    int tile_size;
    int origin_x;
    int origin_y;
    int cols;
    int rows;
    int pixel_width;
    int pixel_height;
} RendererTileViewport;

typedef struct {
    int tile_width;
    int tile_height;
    int columns;
    int rows;
    int category_to_tile[RENDERER_TILE_CATEGORY_COUNT];
} RendererTopDownTilesetSpec;

/* Renderer context - extensible for approved textures */
typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Texture* screen_texture;  /* For offscreen rendering if needed */
    int width;
    int height;
    bool first_person_mode;
    bool top_down_mode;
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
    SDL_Texture* top_down_tilesheet;
    RendererTopDownTilesetSpec top_down_tileset;
    bool textures_approved;
    bool textures_loaded;
    bool top_down_tiles_loaded;
} RendererContext;

/* Public API */
bool renderer_init(RendererContext* ctx);
void renderer_shutdown(RendererContext* ctx);
int renderer_handle_events(RendererContext* ctx, int max_events);
void renderer_render(RendererContext* ctx);
void renderer_toggle_mode(RendererContext* ctx);
void renderer_toggle_top_down_mode(RendererContext* ctx);
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
RendererColor renderer_wall_detail_color(RendererColor shaded, const RendererRayHit* hit,
                                         int screen_x, int screen_y);
int renderer_marker_kind(byte feat, int has_monster, int has_object);
RendererColor renderer_marker_color(int marker_kind);
RendererMarkerProjection renderer_project_marker(const RendererContext* ctx,
                                                 double world_x, double world_y);
int renderer_tile_category_from_values(byte feat, bool remembered,
                                       bool has_player, bool has_monster,
                                       bool has_object);
int renderer_monster_family_category_from_values(u32b flags3, char d_char);
int renderer_object_family_category_from_tval(byte tval);
int renderer_shop_category_from_feat(byte feat);
RendererTileInfo renderer_classify_tile(int y, int x);
RendererTileViewport renderer_tile_viewport(const RendererContext* ctx, int tile_size);
RendererColor renderer_tile_color(int category);
RendererTopDownTilesetSpec renderer_default_top_down_tileset_spec(void);
int renderer_top_down_tile_index(const RendererTopDownTilesetSpec* spec, int category);
SDL_Rect renderer_top_down_source_rect(const RendererTopDownTilesetSpec* spec, int category);
bool renderer_load_top_down_tilesheet(RendererContext* ctx);
int renderer_hud_bar_width(int current, int maximum, int max_width);
void renderer_hud_depth_label(int depth, bool use_feet, char* out, size_t out_size);
void renderer_hud_status_label(bool blind, bool confused, bool poisoned, bool afraid,
                               bool cut, bool stunned, char* out, size_t out_size);
void renderer_hud_title(const RendererHudSnapshot* hud, char* out, size_t out_size);
void renderer_top_down_title(const RendererHudSnapshot* hud, char* out, size_t out_size);
RendererHudSnapshot renderer_hud_snapshot_from_values(int current_hp, int max_hp,
                                                       int current_sp, int max_sp,
                                                       int depth, bool use_feet,
                                                       bool keyboard_focus,
                                                       bool blind, bool confused,
                                                       bool poisoned, bool afraid,
                                                       bool cut, bool stunned,
                                                       const char* last_message);
RendererHudSnapshot renderer_collect_hud_snapshot(const RendererContext* ctx);
int renderer_direction_to_command(double dx, double dy);
int renderer_camera_move_command(const RendererContext* ctx, int move);
int renderer_first_person_key_to_command(RendererContext* ctx, SDL_Keycode key, SDL_Keymod mod);
bool renderer_key_exits_mode(const RendererContext* ctx, SDL_Keycode key, SDL_Keymod mod);
void renderer_set_overlay_message(const char* message);
bool renderer_should_forward_key_event(const RendererContext* ctx);
bool renderer_texture_loading_allowed(const RendererContext* ctx);
int renderer_key_to_command(SDL_Keycode key, SDL_Keymod mod);
RendererContext* get_renderer(void);

/* 2D fallback indicator */
extern bool g_use_2d_fallback;

#endif /* RENDERER_H */
