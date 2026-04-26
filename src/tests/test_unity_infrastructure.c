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
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, renderer_handle_events(NULL, 32), "Null event context should be safe");
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, renderer_handle_events(ctx, 0), "Zero event budget should be safe");

    ctx->dirX = -1.0;
    ctx->dirY = 0.0;
    ctx->planeX = 0.0;
    ctx->planeY = 0.66;
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
    ctx->first_person_mode = TRUE;
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
    TEST_ASSERT_TRUE_MESSAGE(fabs(ctx->dirY) > 0.0001, "Keyboard turn should rotate the camera");

    TEST_PASS_MESSAGE("Renderer DDA, strip, focus, camera movement, and texture-approval tests passed");
#else
    TEST_IGNORE_MESSAGE("SDL2 renderer disabled at build time");
#endif
}
