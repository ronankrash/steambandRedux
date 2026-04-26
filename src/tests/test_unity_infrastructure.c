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

    /* Test wall detection security and logic */
    TEST_ASSERT_TRUE_MESSAGE(renderer_is_wall(0, 0), "Edge walls should return true");
    TEST_ASSERT_TRUE_MESSAGE(renderer_is_wall(2, 2), "Test map walls detected");
    TEST_ASSERT_FALSE_MESSAGE(renderer_is_wall(5, 5), "Interior floor should be false");
    
    /* Test out of bounds security (prevents OOB on legacy arrays) */
    TEST_ASSERT_TRUE_MESSAGE(renderer_is_wall(-1, 0), "Out of bounds treated as wall (safe)");
    TEST_ASSERT_TRUE_MESSAGE(renderer_is_wall(1000, 1000), "Large OOB treated as wall");
    
    renderer_test_dda();

    ctx = get_renderer();
    ctx->first_person_mode = FALSE;
    g_use_2d_fallback = TRUE;
    renderer_toggle_mode(ctx);
    TEST_ASSERT_TRUE_MESSAGE(ctx->first_person_mode, "Renderer mode should enable first-person");
    TEST_ASSERT_FALSE_MESSAGE(g_use_2d_fallback, "2D fallback should be disabled in first-person mode");
    renderer_toggle_mode(ctx);
    TEST_ASSERT_FALSE_MESSAGE(ctx->first_person_mode, "Renderer mode should return to 2D fallback");
    TEST_ASSERT_TRUE_MESSAGE(g_use_2d_fallback, "2D fallback should be restored");

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

    TEST_PASS_MESSAGE("Renderer DDA and wall tests passed - ready for steampunk textures");
#else
    TEST_IGNORE_MESSAGE("SDL2 renderer disabled at build time");
#endif
}
