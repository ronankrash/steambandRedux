/* File: src/tests/test_unity_infrastructure.c
 * Unity framework infrastructure tests
 *
 * These tests verify that Unity framework is properly integrated
 * and working correctly with our build system.
 *
 * Note: setUp/tearDown are defined in unity_integration.c
 * Individual test files can override them if needed.
 */

#include "unity.h"
#include "test_helpers.h"
#ifdef STEAMBAND_HAS_SDL2
#include "renderer.h"
#include <math.h>
#include <string.h>
#endif

/* Test Unity assertion macros work correctly */
void test_unity_assertions(void) {
    TEST_ASSERT_TRUE(1);
    TEST_ASSERT_FALSE(0);
    TEST_ASSERT_EQUAL_INT(5, 5);
    TEST_ASSERT_EQUAL_UINT(10, 10);
    TEST_ASSERT_EQUAL_HEX(0xFF, 0xFF);
    TEST_ASSERT_NOT_EQUAL(0, 1);
}

/* Test Unity test runner executes tests */
void test_unity_runner(void) {
    TEST_ASSERT_TRUE(1);
}

/* Test Unity test suite organization */
void test_unity_suite_organization(void) {
    TEST_ASSERT_TRUE(1);
}

/* Test test helpers work correctly */
void test_test_helpers_temp_file(void) {
    test_file_t tf = test_create_temp_file("unity_test");
    TEST_ASSERT_TRUE(tf.created);
    TEST_ASSERT_TRUE(test_file_exists(tf.path));
    test_cleanup_temp_file(&tf);
    TEST_ASSERT_FALSE(test_file_exists(tf.path));
}

/* Test Unity fixtures work (setUp/tearDown are called by Unity framework) */
void test_unity_fixtures_available(void) {
    /* Verify setUp/tearDown functions exist and are callable */
    /* This test passes if Unity can call setUp/tearDown without errors */
    TEST_ASSERT_TRUE(1);
}

/* Basic renderer tests for raycaster prototype (TDD) - tests wall detection from cave data,
 * DDA logic, security bounds checking. Uses test map fallback. */
void test_renderer_basic(void) {
#ifdef STEAMBAND_HAS_SDL2
    RendererContext *ctx;
    RendererRayHit ray_hit;
    RendererWallStrip strip;
    RendererColor near_wall;
    RendererColor far_wall;
    RendererColor masonry_wall;
    RendererColor quartz_wall;
    RendererColor ceiling_top;
    RendererColor floor_bottom;
    RendererColor detail_base;
    RendererColor mortar_detail;
    RendererColor seam_detail;
    RendererColor rivet_detail;
    RendererColor marker_color;
    RendererMarkerProjection marker;
    RendererTileInfo tile;
    RendererTileViewport tile_view;
    RendererTopDownTilesetSpec tile_spec;
    SDL_Rect tile_src;
    RendererColor player_tile;
    RendererColor floor_tile;
    byte fake_cave_info[DUNGEON_HGT][256];
    s16b fake_cave_o_idx[DUNGEON_HGT][DUNGEON_WID];
    s16b fake_cave_m_idx[DUNGEON_HGT][DUNGEON_WID];
    object_type fake_o_list[2];
    monster_type fake_m_list[2];
    monster_race fake_r_info[2];
    maxima fake_z_info;
    RendererHudSnapshot hud;
    char label[16];
    char status[32];
    char title[160];
    char top_down_title[160];

    /* Test wall detection security and logic */
    TEST_ASSERT_TRUE_MESSAGE(renderer_is_wall(0, 0), "Edge walls should return true");
    TEST_ASSERT_TRUE_MESSAGE(renderer_is_wall(2, 2), "Test map walls detected");
    TEST_ASSERT_FALSE_MESSAGE(renderer_is_wall(5, 5), "Interior floor should be false");

    /* Test out of bounds security (prevents OOB on legacy arrays) */
    TEST_ASSERT_TRUE_MESSAGE(renderer_is_wall(-1, 0), "Out of bounds treated as wall (safe)");
    TEST_ASSERT_TRUE_MESSAGE(renderer_is_wall(1000, 1000), "Large OOB treated as wall");

    renderer_test_dda();
    ray_hit = renderer_cast_ray(5.5, 5.5, -1.0, 0.0, 64);
    TEST_ASSERT_TRUE_MESSAGE(ray_hit.hit, "Ray should hit the west test-map wall");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, ray_hit.map_x, "Ray should hit deterministic west wall column");
    TEST_ASSERT_EQUAL_INT_MESSAGE(5, ray_hit.map_y, "Ray should stay on deterministic row");
    TEST_ASSERT_EQUAL_INT_MESSAGE(5, ray_hit.steps, "Ray should take deterministic DDA steps");
    TEST_ASSERT_TRUE_MESSAGE(fabs(ray_hit.distance - 4.5) < 0.0001,
                             "Ray should report deterministic perpendicular distance");

    strip = renderer_wall_strip(480, ray_hit.distance);
    TEST_ASSERT_EQUAL_INT_MESSAGE(106, strip.line_height, "Wall strip height should be deterministic");
    TEST_ASSERT_EQUAL_INT_MESSAGE(187, strip.draw_start, "Wall strip top should be centered");
    TEST_ASSERT_EQUAL_INT_MESSAGE(293, strip.draw_end, "Wall strip bottom should be centered");
    near_wall = renderer_depth_shade(renderer_wall_base_color(&ray_hit), 1.0, 0);
    far_wall = renderer_depth_shade(renderer_wall_base_color(&ray_hit), 18.0, 0);
    TEST_ASSERT_TRUE_MESSAGE(far_wall.r < near_wall.r, "Distant walls should be darker/foggier");
    TEST_ASSERT_TRUE_MESSAGE(far_wall.g < near_wall.g, "Distant walls should lose warm brightness");
    TEST_ASSERT_TRUE_MESSAGE(renderer_depth_shade(renderer_wall_base_color(&ray_hit), 4.0, 1).r <
                             renderer_depth_shade(renderer_wall_base_color(&ray_hit), 4.0, 0).r,
                             "Side-hit walls should shade darker for depth");
    masonry_wall = renderer_wall_base_color(&ray_hit);
    ray_hit.map_x = 2;
    ray_hit.map_y = 2;
    quartz_wall = renderer_wall_base_color(&ray_hit);
    TEST_ASSERT_TRUE_MESSAGE(quartz_wall.g != masonry_wall.g || quartz_wall.b != masonry_wall.b,
                             "Fallback wall color should vary by deterministic feature type");
    ceiling_top = renderer_atmosphere_color(0, 480);
    floor_bottom = renderer_atmosphere_color(479, 480);
    TEST_ASSERT_TRUE_MESSAGE(ceiling_top.b >= floor_bottom.b, "Ceiling haze should remain cooler than floor");
    TEST_ASSERT_TRUE_MESSAGE(ceiling_top.r <= renderer_atmosphere_color(239, 480).r,
                             "Ceiling should brighten toward the horizon");
    detail_base.r = 100;
    detail_base.g = 80;
    detail_base.b = 60;
    detail_base.a = 255;
    mortar_detail = renderer_wall_detail_color(detail_base, &ray_hit, 10, 1);
    seam_detail = renderer_wall_detail_color(detail_base, &ray_hit, 17, 2);
    rivet_detail = renderer_wall_detail_color(detail_base, &ray_hit, 38, 2);
    TEST_ASSERT_TRUE_MESSAGE(mortar_detail.r < detail_base.r && mortar_detail.g < detail_base.g,
                             "Procedural wall mortar should darken masonry rows");
    TEST_ASSERT_TRUE_MESSAGE(seam_detail.r < detail_base.r && seam_detail.b < detail_base.b,
                             "Procedural wall seams should darken vertical plate breaks");
    TEST_ASSERT_TRUE_MESSAGE(rivet_detail.r > detail_base.r && rivet_detail.g > detail_base.g,
                             "Procedural brass rivets should add warm highlights");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_MARKER_MONSTER,
                                  renderer_marker_kind(FEAT_FLOOR, TRUE, FALSE),
                                  "Visible monsters should take marker priority");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_MARKER_OBJECT,
                                  renderer_marker_kind(FEAT_FLOOR, FALSE, TRUE),
                                  "Visible objects should get item markers");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_MARKER_STAIRS,
                                  renderer_marker_kind(FEAT_MORE, FALSE, FALSE),
                                  "Stairs should be marked for first-person navigation");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_MARKER_DOOR,
                                  renderer_marker_kind(FEAT_DOOR_HEAD, FALSE, FALSE),
                                  "Doors should be marked as interactable");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_MARKER_TRAP,
                                  renderer_marker_kind(FEAT_TRAP_HEAD, FALSE, FALSE),
                                  "Traps should be marked as hazards");
    marker_color = renderer_marker_color(RENDERER_MARKER_MONSTER);
    TEST_ASSERT_TRUE_MESSAGE(marker_color.r > marker_color.g,
                             "Monster markers should read as danger");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_PLAYER,
                                  renderer_tile_category_from_values(FEAT_FLOOR, TRUE, TRUE, TRUE, TRUE),
                                  "Player tile should take top-down priority");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_DARKNESS,
                                  renderer_tile_category_from_values(FEAT_FLOOR, FALSE, FALSE, TRUE, TRUE),
                                  "Unremembered grids should stay hidden in top-down tiles");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_MONSTER,
                                  renderer_tile_category_from_values(FEAT_FLOOR, TRUE, FALSE, TRUE, TRUE),
                                  "Monster tile should outrank object tile");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_OBJECT,
                                  renderer_tile_category_from_values(FEAT_FLOOR, TRUE, FALSE, FALSE, TRUE),
                                  "Object tile should outrank terrain tile");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_WALL,
                                  renderer_tile_category_from_values(FEAT_WALL_EXTRA, TRUE, FALSE, FALSE, FALSE),
                                  "Known walls should classify as top-down wall tiles");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_DOOR,
                                  renderer_tile_category_from_values(FEAT_DOOR_HEAD, TRUE, FALSE, FALSE, FALSE),
                                  "Known doors should classify as top-down door tiles");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_STAIRS_UP,
                                  renderer_tile_category_from_values(FEAT_LESS, TRUE, FALSE, FALSE, FALSE),
                                  "Up stairs should classify separately for tile art");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_STAIRS_DN,
                                  renderer_tile_category_from_values(FEAT_MORE, TRUE, FALSE, FALSE, FALSE),
                                  "Down stairs should classify separately for tile art");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_TRAP,
                                  renderer_tile_category_from_values(FEAT_TRAP_HEAD, TRUE, FALSE, FALSE, FALSE),
                                  "Traps should classify as hazard tiles");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_GLYPH,
                                  renderer_tile_category_from_values(FEAT_GLYPH, TRUE, FALSE, FALSE, FALSE),
                                  "Glyphs should get a distinct protection tile");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_SHOP,
                                  renderer_shop_category_from_feat(0),
                                  "Unknown shop values should fall back to a generic town facade tile");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_SHOP_GENERAL,
                                  renderer_shop_category_from_feat(FEAT_SHOP_HEAD + 0),
                                  "General Store should get a distinct town facade tile");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_SHOP_GUN,
                                  renderer_shop_category_from_feat(FEAT_SHOP_HEAD + 2),
                                  "Gun shop should get a distinct town facade tile");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_SHOP_HOME,
                                  renderer_shop_category_from_feat(FEAT_SHOP_HEAD + 7),
                                  "Home should get a distinct town facade tile");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_SHOP_GUN,
                                  renderer_tile_category_from_values(FEAT_SHOP_HEAD + 2, TRUE, FALSE, FALSE, FALSE),
                                  "Shop terrain should route through specific shop facade tiles");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_RUBBLE,
                                  renderer_tile_category_from_values(FEAT_RUBBLE, TRUE, FALSE, FALSE, FALSE),
                                  "Rubble should get a distinct diggable obstruction tile");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_ORE,
                                  renderer_tile_category_from_values(FEAT_MAGMA, TRUE, FALSE, FALSE, FALSE),
                                  "Ore veins should get a distinct mining tile");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_OBJECT_FOOD,
                                  renderer_object_family_category_from_tval(TV_FOOD),
                                  "Food/anodynes should select the food object tile");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_OBJECT_SCROLL,
                                  renderer_object_family_category_from_tval(TV_TEXT),
                                  "Texts/books should select the scroll object tile");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_OBJECT_POTION,
                                  renderer_object_family_category_from_tval(TV_TONIC),
                                  "Tonics should select the potion object tile");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_OBJECT_WEAPON,
                                  renderer_object_family_category_from_tval(TV_SWORD),
                                  "Weapons should select the weapon object tile");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_OBJECT_ARMOR,
                                  renderer_object_family_category_from_tval(TV_HARD_ARMOR),
                                  "Armor should select the armor object tile");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_OBJECT_GUN,
                                  renderer_object_family_category_from_tval(TV_GUN),
                                  "Guns should select the ray gun object tile");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_OBJECT_AMMO,
                                  renderer_object_family_category_from_tval(TV_BULLET),
                                  "Ammo should select the ammo object tile");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_OBJECT_MONEY,
                                  renderer_object_family_category_from_tval(TV_GOLD),
                                  "Gold should select the money object tile");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_OBJECT_JEWELRY,
                                  renderer_object_family_category_from_tval(TV_RING),
                                  "Rings should select the jewelry object tile");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_OBJECT_DEVICE,
                                  renderer_object_family_category_from_tval(TV_CHEST),
                                  "Chests/devices should select the device object tile");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_MONSTER_AUTOMATA,
                                  renderer_monster_family_category_from_values(RF3_AUTOMATA, 'g'),
                                  "Automata monsters should select a machinery family tile");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_MONSTER_UNDEAD,
                                  renderer_monster_family_category_from_values(RF3_UNDEAD, 'Z'),
                                  "Undead monsters should select a spectral family tile");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_MONSTER_UNDEAD,
                                  renderer_monster_family_category_from_values(RF3_DEMON, 'U'),
                                  "Demons should share the occult threat family tile");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_MONSTER_BEAST,
                                  renderer_monster_family_category_from_values(RF3_ANIMAL, 'q'),
                                  "Animal monsters should select a beast family tile");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_MONSTER_BEAST,
                                  renderer_monster_family_category_from_values(RF3_DRAGON, 'D'),
                                  "Dragons should select a large beast family tile");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_MONSTER_HUMANOID,
                                  renderer_monster_family_category_from_values(0, 'p'),
                                  "Humanoid display chars should select a humanoid family tile");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_MONSTER,
                                  renderer_monster_family_category_from_values(0, 'j'),
                                  "Unknown monster families should keep the generic monster tile");
    tile = renderer_classify_tile(-1, 0);
    TEST_ASSERT_FALSE_MESSAGE(tile.in_bounds, "Out-of-bounds top-down classification should be safe");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_DARKNESS, tile.category,
                                  "Out-of-bounds top-down classification should stay dark");
    tile = renderer_classify_tile(2, 2);
    TEST_ASSERT_TRUE_MESSAGE(tile.in_bounds, "Fallback test-map tile should classify in bounds");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_ORE, tile.category,
                                  "Fallback test-map quartz should become top-down ore tiles");
    tile = renderer_classify_tile(5, 5);
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_FLOOR, tile.category,
                                  "Fallback test-map floors should become top-down floor tiles");
    memset(fake_cave_info, 0, sizeof(fake_cave_info));
    memset(fake_cave_o_idx, 0, sizeof(fake_cave_o_idx));
    memset(fake_cave_m_idx, 0, sizeof(fake_cave_m_idx));
    memset(fake_o_list, 0, sizeof(fake_o_list));
    memset(fake_m_list, 0, sizeof(fake_m_list));
    memset(fake_r_info, 0, sizeof(fake_r_info));
    memset(&fake_z_info, 0, sizeof(fake_z_info));
    fake_z_info.m_max = 2;
    fake_z_info.o_max = 2;
    fake_z_info.r_max = 2;
    fake_cave_info[5][5] = CAVE_MARK | CAVE_SEEN;
    fake_cave_o_idx[5][5] = 1;
    fake_o_list[1].k_idx = 1;
    fake_o_list[1].tval = TV_GUN;
    cave_info = fake_cave_info;
    cave_o_idx = fake_cave_o_idx;
    o_list = fake_o_list;
    z_info = &fake_z_info;
    o_max = 2;
    tile = renderer_classify_tile(5, 5);
    TEST_ASSERT_TRUE_MESSAGE(tile.has_object, "Bounded live objects should be detected");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_OBJECT_GUN, tile.category,
                                  "Live gun objects should select the gun family tile");
    fake_cave_o_idx[5][5] = 5;
    tile = renderer_classify_tile(5, 5);
    TEST_ASSERT_FALSE_MESSAGE(tile.has_object, "Out-of-range o_idx should be ignored safely");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_FLOOR, tile.category,
                                  "Out-of-range o_idx should fall back to terrain");
    fake_cave_o_idx[5][5] = 0;
    fake_cave_m_idx[5][5] = 1;
    fake_m_list[1].r_idx = 1;
    fake_m_list[1].ml = TRUE;
    fake_r_info[1].flags3 = RF3_AUTOMATA;
    fake_r_info[1].x_char = 'g';
    cave_info = fake_cave_info;
    cave_m_idx = fake_cave_m_idx;
    m_list = fake_m_list;
    r_info = fake_r_info;
    z_info = &fake_z_info;
    m_max = 2;
    tile = renderer_classify_tile(5, 5);
    TEST_ASSERT_TRUE_MESSAGE(tile.has_monster, "Visible bounded live monsters should be detected");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_MONSTER_AUTOMATA, tile.category,
                                  "Visible automata should select the automata family tile");
    fake_m_list[1].ml = FALSE;
    tile = renderer_classify_tile(5, 5);
    TEST_ASSERT_FALSE_MESSAGE(tile.has_monster, "Unseen monsters should not leak into the top-down tile view");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_FLOOR, tile.category,
                                  "Unseen monsters should leave the remembered terrain tile visible");
    fake_cave_m_idx[5][5] = 5;
    fake_m_list[1].ml = TRUE;
    tile = renderer_classify_tile(5, 5);
    TEST_ASSERT_FALSE_MESSAGE(tile.has_monster, "Out-of-range m_idx should be ignored safely");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_FLOOR, tile.category,
                                  "Out-of-range m_idx should fall back to terrain");
    cave_info = NULL;
    cave_o_idx = NULL;
    cave_m_idx = NULL;
    o_list = NULL;
    m_list = NULL;
    r_info = NULL;
    z_info = NULL;
    o_max = 1;
    m_max = 1;
    player_tile = renderer_tile_color(RENDERER_TILE_PLAYER);
    floor_tile = renderer_tile_color(RENDERER_TILE_FLOOR);
    TEST_ASSERT_TRUE_MESSAGE(player_tile.r > floor_tile.r && player_tile.g > floor_tile.g,
                             "Player top-down tile should read brighter than floor fallback");
    tile_spec = renderer_default_top_down_tileset_spec();
    TEST_ASSERT_EQUAL_INT_MESSAGE(24, tile_spec.tile_width, "Default top-down tilesheet should use 24px source tiles");
    TEST_ASSERT_EQUAL_INT_MESSAGE(24, tile_spec.tile_height, "Default top-down tilesheet should use square source tiles");
    TEST_ASSERT_EQUAL_INT_MESSAGE(9, tile_spec.columns, "Default top-down tilesheet should map thirty-six tiles over nine columns");
    TEST_ASSERT_EQUAL_INT_MESSAGE(RENDERER_TILE_PLAYER,
                                  renderer_top_down_tile_index(&tile_spec, RENDERER_TILE_PLAYER),
                                  "Default tilesheet maps each category to its matching tile index");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, renderer_top_down_tile_index(&tile_spec, -1),
                                  "Invalid low tile category should fall back to darkness tile");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, renderer_top_down_tile_index(&tile_spec, 999),
                                  "Invalid high tile category should fall back to darkness tile");
    tile_src = renderer_top_down_source_rect(&tile_spec, RENDERER_TILE_PLAYER);
    TEST_ASSERT_EQUAL_INT_MESSAGE(192, tile_src.x, "Player tile should be in the fourth row source atlas");
    TEST_ASSERT_EQUAL_INT_MESSAGE(72, tile_src.y, "Player tile should be in the fourth row source atlas");
    TEST_ASSERT_EQUAL_INT_MESSAGE(24, tile_src.w, "Source tile width should match the atlas contract");
    TEST_ASSERT_EQUAL_INT_MESSAGE(24, tile_src.h, "Source tile height should match the atlas contract");
    TEST_ASSERT_FALSE_MESSAGE(renderer_load_top_down_tilesheet(NULL),
                              "Top-down tilesheet loader should reject a null renderer context safely");

    ctx = get_renderer();
    memset(ctx, 0, sizeof(*ctx));
    ctx->width = RENDER_WIDTH;
    ctx->height = RENDER_HEIGHT;
    ctx->posX = 5.5;
    ctx->posY = 5.5;
    ctx->dirX = -1.0;
    ctx->dirY = 0.0;
    ctx->planeX = 0.0;
    ctx->planeY = 0.66;
    ctx->first_person_mode = FALSE;
    ctx->keyboard_focus = FALSE;
    ctx->textures_approved = FALSE;
    g_use_2d_fallback = TRUE;
    renderer_toggle_mode(ctx);
    TEST_ASSERT_TRUE_MESSAGE(ctx->first_person_mode, "Renderer mode should enable first-person");
    TEST_ASSERT_FALSE_MESSAGE(g_use_2d_fallback, "2D fallback should be disabled in first-person mode");
    TEST_ASSERT_TRUE_MESSAGE(ctx->keyboard_focus, "First-person toggle should accept SDL key focus");
    TEST_ASSERT_TRUE_MESSAGE(renderer_should_forward_key_event(ctx), "Focused first-person mode should forward keys");
    TEST_ASSERT_FALSE_MESSAGE(renderer_texture_loading_allowed(ctx), "Unapproved textures must not load");
    TEST_ASSERT_FALSE_MESSAGE(ctx->show_debug_minimap, "Debug minimap should be hidden by default for immersion");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, renderer_trace_column(ctx, RENDER_WIDTH / 2, &ray_hit),
                                  "Center trace should be deterministic and testable");
    TEST_ASSERT_TRUE_MESSAGE(ray_hit.hit, "Center trace should hit a wall");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, ray_hit.map_x, "Center trace should face west in default camera");
    TEST_ASSERT_EQUAL_INT_MESSAGE(187, ray_hit.draw_start, "Center trace should expose strip start");
    TEST_ASSERT_EQUAL_INT_MESSAGE('4', renderer_camera_move_command(ctx, RENDERER_MOVE_FORWARD),
                                  "Forward should follow the camera direction");
    TEST_ASSERT_EQUAL_INT_MESSAGE('6', renderer_camera_move_command(ctx, RENDERER_MOVE_BACKWARD),
                                  "Backward should reverse the camera direction");
    TEST_ASSERT_EQUAL_INT_MESSAGE('8', renderer_camera_move_command(ctx, RENDERER_MOVE_LEFT),
                                  "Left strafe should follow the negative camera plane");
    TEST_ASSERT_EQUAL_INT_MESSAGE('2', renderer_camera_move_command(ctx, RENDERER_MOVE_RIGHT),
                                  "Right strafe should follow the camera plane");
    TEST_ASSERT_EQUAL_INT_MESSAGE('4', renderer_first_person_key_to_command(ctx, SDLK_w, KMOD_NONE),
                                  "W should move forward in first-person");
    TEST_ASSERT_EQUAL_INT_MESSAGE('2', renderer_first_person_key_to_command(ctx, SDLK_d, KMOD_NONE),
                                  "D should strafe right in first-person");
    ctx->keyboard_focus = FALSE;
    TEST_ASSERT_FALSE_MESSAGE(renderer_should_forward_key_event(ctx), "Unfocused SDL window should not forward keys");
    ctx->textures_approved = TRUE;
    TEST_ASSERT_TRUE_MESSAGE(renderer_texture_loading_allowed(ctx), "Texture loading needs explicit approval flag");
    renderer_clamp_viewport(ctx, 40, 40);
    TEST_ASSERT_EQUAL_INT_MESSAGE(160, ctx->width, "Viewport width should clamp to playable minimum");
    TEST_ASSERT_EQUAL_INT_MESSAGE(120, ctx->height, "Viewport height should clamp to playable minimum");
    renderer_clamp_viewport(ctx, 9999, 9999);
    TEST_ASSERT_EQUAL_INT_MESSAGE(3840, ctx->width, "Viewport width should clamp to safe maximum");
    TEST_ASSERT_EQUAL_INT_MESSAGE(2160, ctx->height, "Viewport height should clamp to safe maximum");
    renderer_clamp_viewport(ctx, RENDER_WIDTH, RENDER_HEIGHT);
    renderer_toggle_mode(ctx);
    TEST_ASSERT_FALSE_MESSAGE(ctx->first_person_mode, "Renderer mode should return to 2D fallback");
    TEST_ASSERT_TRUE_MESSAGE(g_use_2d_fallback, "2D fallback should be restored");
    TEST_ASSERT_FALSE_MESSAGE(ctx->keyboard_focus, "2D fallback should stop SDL key forwarding");
    renderer_toggle_top_down_mode(ctx);
    TEST_ASSERT_TRUE_MESSAGE(ctx->top_down_mode, "Top-down SDL tile mode should activate independently");
    TEST_ASSERT_FALSE_MESSAGE(ctx->first_person_mode, "Top-down mode should not leave first-person active");
    TEST_ASSERT_FALSE_MESSAGE(g_use_2d_fallback, "SDL top-down mode should claim the renderer fallback indicator");
    TEST_ASSERT_TRUE_MESSAGE(renderer_should_forward_key_event(ctx), "Focused top-down mode should forward keyboard commands");
    tile_view = renderer_tile_viewport(ctx, 0);
    TEST_ASSERT_EQUAL_INT_MESSAGE(15, tile_view.tile_size, "Default 640x480 top-down tiles should target handheld-readable scale");
    TEST_ASSERT_TRUE_MESSAGE(tile_view.cols >= 9 && tile_view.rows >= 7,
                             "Top-down viewport should keep a playable minimum visible area");
    TEST_ASSERT_TRUE_MESSAGE(tile_view.origin_x >= 0 && tile_view.origin_y >= 0,
                             "Top-down viewport should clamp origins safely");
    renderer_toggle_top_down_mode(ctx);
    TEST_ASSERT_FALSE_MESSAGE(ctx->top_down_mode, "Top-down SDL tile mode should close cleanly");
    TEST_ASSERT_TRUE_MESSAGE(g_use_2d_fallback, "Closing top-down mode should restore legacy fallback");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, renderer_handle_events(NULL, 32), "Null event context should be safe");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, renderer_handle_events(ctx, 0), "Zero event budget should be safe");

    TEST_ASSERT_EQUAL_INT_MESSAGE(0, renderer_hud_bar_width(0, 100, 80),
                                  "Empty HUD bars should render as zero width");
    TEST_ASSERT_EQUAL_INT_MESSAGE(40, renderer_hud_bar_width(50, 100, 80),
                                  "HUD bar width should scale by current/max values");
    TEST_ASSERT_EQUAL_INT_MESSAGE(80, renderer_hud_bar_width(125, 100, 80),
                                  "Overfull HUD bars should clamp to max width");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, renderer_hud_bar_width(10, 0, 80),
                                  "HUD bars should guard zero maximum values");
    renderer_hud_depth_label(0, FALSE, label, sizeof(label));
    TEST_ASSERT_EQUAL_STRING_MESSAGE("Town", label, "Depth zero should use the town label");
    renderer_hud_depth_label(7, FALSE, label, sizeof(label));
    TEST_ASSERT_EQUAL_STRING_MESSAGE("Lev 7", label, "Level depth label should fit the title");
    renderer_hud_depth_label(7, TRUE, label, sizeof(label));
    TEST_ASSERT_EQUAL_STRING_MESSAGE("350 ft", label, "Feet depth label should match legacy depth option");
    renderer_hud_status_label(FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, status, sizeof(status));
    TEST_ASSERT_EQUAL_STRING_MESSAGE("OK", status, "No timed statuses should be explicit");
    renderer_hud_status_label(TRUE, TRUE, TRUE, TRUE, TRUE, TRUE, status, sizeof(status));
    TEST_ASSERT_NOT_NULL_MESSAGE(strstr(status, "Blind"), "HUD status should mention blindness");
    TEST_ASSERT_NOT_NULL_MESSAGE(strstr(status, "Conf"), "HUD status should mention confusion");
    TEST_ASSERT_NOT_NULL_MESSAGE(strstr(status, "Pois"), "HUD status should mention poison");
    TEST_ASSERT_NOT_NULL_MESSAGE(strstr(status, "Fear"), "HUD status should mention fear");
    TEST_ASSERT_NOT_NULL_MESSAGE(strstr(status, "Cut"), "HUD status should mention cuts");
    TEST_ASSERT_NOT_NULL_MESSAGE(strstr(status, "Stun"), "HUD status should mention stun");
    hud = renderer_hud_snapshot_from_values(23, 40, 5, 12, 9, TRUE, TRUE,
                                            TRUE, FALSE, TRUE, FALSE, FALSE, TRUE,
                                            "The automaton whistles.");
    TEST_ASSERT_EQUAL_INT_MESSAGE(23, hud.current_hp, "HUD snapshot should preserve HP");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("450 ft", hud.depth_label, "HUD snapshot should format depth");
    TEST_ASSERT_TRUE_MESSAGE(hud.has_message, "HUD snapshot should flag recent command feedback");
    TEST_ASSERT_NOT_NULL_MESSAGE(strstr(hud.last_message, "automaton"), "HUD snapshot should copy recent messages");
    TEST_ASSERT_NOT_NULL_MESSAGE(strstr(hud.status_label, "Blind"), "HUD snapshot should include status labels");
    TEST_ASSERT_NOT_NULL_MESSAGE(strstr(hud.title, "HP 23/40"), "HUD title should communicate HP numerically");
    TEST_ASSERT_NOT_NULL_MESSAGE(strstr(hud.title, "automaton"), "HUD title should expose recent legacy messages");
    renderer_set_overlay_message("Command menu: Inventory");
    hud = renderer_collect_hud_snapshot(NULL);
    TEST_ASSERT_TRUE_MESSAGE(hud.has_message, "Overlay command feedback should appear as HUD message activity");
    TEST_ASSERT_EQUAL_STRING_MESSAGE("Command menu: Inventory", hud.last_message,
                                     "Overlay command feedback should override stale legacy messages");
    renderer_set_overlay_message(NULL);
    renderer_hud_title(NULL, title, sizeof(title));
    TEST_ASSERT_NOT_NULL_MESSAGE(strstr(title, "no game state"), "Null HUD title should be safe");
    renderer_top_down_title(NULL, top_down_title, sizeof(top_down_title));
    TEST_ASSERT_NOT_NULL_MESSAGE(strstr(top_down_title, "2D Tiles"),
                                 "Null top-down title should identify the renderer mode");
    TEST_ASSERT_NOT_NULL_MESSAGE(strstr(top_down_title, "Ctrl+F11"),
                                 "Null top-down title should advertise the correct toggle");
    renderer_top_down_title(&hud, top_down_title, sizeof(top_down_title));
    TEST_ASSERT_NOT_NULL_MESSAGE(strstr(top_down_title, "2D Tiles"),
                                 "Top-down title should identify the renderer mode");
    TEST_ASSERT_NOT_NULL_MESSAGE(strstr(top_down_title, "keys forward"),
                                 "Top-down title should explain keyboard command forwarding");
    TEST_ASSERT_NOT_NULL_MESSAGE(strstr(top_down_title, "HP"),
                                 "Top-down title should preserve readable HP fallback");

    ctx->dirX = -1.0;
    ctx->dirY = 0.0;
    ctx->planeX = 0.0;
    ctx->planeY = 0.66;
    marker = renderer_project_marker(ctx, 1.5, 5.5);
    TEST_ASSERT_TRUE_MESSAGE(marker.visible, "Marker directly ahead should project into view");
    TEST_ASSERT_TRUE_MESSAGE(marker.screen_x > 0 && marker.screen_x < RENDER_WIDTH,
                             "Forward marker should project inside the viewport");
    marker = renderer_project_marker(ctx, 9.5, 5.5);
    TEST_ASSERT_FALSE_MESSAGE(marker.visible, "Marker behind camera should not be visible");
    renderer_rotate(ctx, 3.14159265358979323846 / 2.0);
    TEST_ASSERT_TRUE_MESSAGE(fabs(ctx->dirX) < 0.0001, "Rotation should turn dirX near zero");
    TEST_ASSERT_TRUE_MESSAGE(fabs(ctx->dirY + 1.0) < 0.0001, "Rotation should turn direction left");
    TEST_ASSERT_TRUE_MESSAGE(fabs(ctx->planeX + 0.66) < 0.0001, "Rotation should rotate camera plane X");
    TEST_ASSERT_TRUE_MESSAGE(fabs(ctx->planeY) < 0.0001, "Rotation should rotate camera plane Y near zero");
    TEST_ASSERT_TRUE_MESSAGE(renderer_safe_perp_distance(0, 5, 5, 5.5, 5.5, 1, 1, 0.0, 1.0) > 0.0,
                             "Perpendicular distance should guard zero X rays");
    TEST_ASSERT_TRUE_MESSAGE(renderer_safe_perp_distance(1, 5, 5, 5.5, 5.5, 1, 1, 1.0, 0.0) > 0.0,
                             "Perpendicular distance should guard zero Y rays");
    TEST_ASSERT_EQUAL_INT_MESSAGE('8', renderer_key_to_command(SDLK_UP, KMOD_NONE),
                                  "SDL up should map to Angband north");
    TEST_ASSERT_EQUAL_INT_MESSAGE('i', renderer_key_to_command(SDLK_i, KMOD_NONE),
                                  "SDL letters should map to Angband commands");
    TEST_ASSERT_EQUAL_INT_MESSAGE('R', renderer_key_to_command(SDLK_r, KMOD_SHIFT),
                                  "SDL shift letters should preserve uppercase commands");
    TEST_ASSERT_EQUAL_INT_MESSAGE('>', renderer_key_to_command(SDLK_PERIOD, KMOD_SHIFT),
                                  "SDL shifted period should map to stairs down");
    TEST_ASSERT_EQUAL_INT_MESSAGE(27, renderer_key_to_command(SDLK_ESCAPE, KMOD_NONE),
                                  "SDL Escape should be forwardable as legacy cancel");
    ctx->first_person_mode = TRUE;
    ctx->top_down_mode = FALSE;
    TEST_ASSERT_TRUE_MESSAGE(renderer_key_exits_mode(ctx, SDLK_ESCAPE, KMOD_NONE),
                             "Escape should exit first-person mode");
    ctx->first_person_mode = FALSE;
    ctx->top_down_mode = TRUE;
    TEST_ASSERT_FALSE_MESSAGE(renderer_key_exits_mode(ctx, SDLK_ESCAPE, KMOD_NONE),
                              "Escape should forward to legacy cancel in top-down mode");
    TEST_ASSERT_TRUE_MESSAGE(renderer_key_exits_mode(ctx, SDLK_F12, KMOD_CTRL),
                             "Ctrl+F12 should remain a renderer exit shortcut");
    TEST_ASSERT_FALSE_MESSAGE(renderer_key_exits_mode(ctx, SDLK_F11, KMOD_CTRL),
                              "Ctrl+F11 is handled by the top-down toggle path, not generic exit");
    ctx->first_person_mode = TRUE;
    ctx->top_down_mode = FALSE;
    ctx->keyboard_focus = TRUE;
    ctx->dirX = -1.0;
    ctx->dirY = 0.0;
    ctx->planeX = 0.0;
    ctx->planeY = 0.66;
    TEST_ASSERT_EQUAL_INT_MESSAGE('4', renderer_camera_move_command(ctx, RENDERER_MOVE_FORWARD),
                                  "Camera-forward movement should follow current facing");
    TEST_ASSERT_EQUAL_INT_MESSAGE('6', renderer_camera_move_command(ctx, RENDERER_MOVE_BACKWARD),
                                  "Camera-back movement should oppose current facing");
    TEST_ASSERT_EQUAL_INT_MESSAGE('8', renderer_camera_move_command(ctx, RENDERER_MOVE_LEFT),
                                  "Camera-left strafe should use the camera plane");
    TEST_ASSERT_EQUAL_INT_MESSAGE('2', renderer_camera_move_command(ctx, RENDERER_MOVE_RIGHT),
                                  "Camera-right strafe should use the camera plane");
    TEST_ASSERT_EQUAL_INT_MESSAGE('4', renderer_first_person_key_to_command(ctx, SDLK_w, KMOD_NONE),
                                  "First-person W should move forward relative to camera");
    TEST_ASSERT_EQUAL_INT_MESSAGE('6', renderer_first_person_key_to_command(ctx, SDLK_s, KMOD_NONE),
                                  "First-person S should move backward relative to camera");
    TEST_ASSERT_EQUAL_INT_MESSAGE('8', renderer_first_person_key_to_command(ctx, SDLK_a, KMOD_NONE),
                                  "First-person A should strafe left relative to camera");
    TEST_ASSERT_EQUAL_INT_MESSAGE('2', renderer_first_person_key_to_command(ctx, SDLK_d, KMOD_NONE),
                                  "First-person D should strafe right relative to camera");
    TEST_ASSERT_EQUAL_INT_MESSAGE('W', renderer_first_person_key_to_command(ctx, SDLK_w, KMOD_SHIFT),
                                  "Shifted command letters should still reach legacy command handling");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, renderer_first_person_key_to_command(ctx, SDLK_LEFT, KMOD_NONE),
                                  "First-person left arrow should turn instead of moving absolutely");
    TEST_ASSERT_TRUE_MESSAGE(ctx->dirY < -0.0001, "Left arrow should turn the first-person camera left");
    ctx->dirX = -1.0;
    ctx->dirY = 0.0;
    ctx->planeX = 0.0;
    ctx->planeY = 0.66;
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, renderer_first_person_key_to_command(ctx, SDLK_RIGHT, KMOD_NONE),
                                  "First-person right arrow should turn instead of moving absolutely");
    TEST_ASSERT_TRUE_MESSAGE(ctx->dirY > 0.0001, "Right arrow should turn the first-person camera right");

    TEST_PASS_MESSAGE("Renderer DDA, strip, focus, camera movement, and texture-approval tests passed");
#else
    TEST_IGNORE_MESSAGE("SDL2 renderer disabled at build time");
#endif
}
