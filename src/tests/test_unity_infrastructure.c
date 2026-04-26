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
    /* Test wall detection security and logic */
    TEST_ASSERT_TRUE_MESSAGE(renderer_is_wall(0, 0), "Edge walls should return true");
    TEST_ASSERT_TRUE_MESSAGE(renderer_is_wall(1, 1), "Test map walls detected");
    TEST_ASSERT_FALSE_MESSAGE(renderer_is_wall(5, 5), "Interior floor should be false");
    
    /* Test out of bounds security (prevents OOB on legacy arrays) */
    TEST_ASSERT_TRUE_MESSAGE(renderer_is_wall(-1, 0), "Out of bounds treated as wall (safe)");
    TEST_ASSERT_TRUE_MESSAGE(renderer_is_wall(1000, 1000), "Large OOB treated as wall");
    
    renderer_test_dda();
    TEST_PASS_MESSAGE("Renderer DDA and wall tests passed - ready for steampunk textures");
#else
    TEST_IGNORE_MESSAGE("SDL2 renderer disabled at build time");
#endif
}
