/* File: src/tests/unity_test_runner.c
 * Unity test runner - runs Unity framework tests
 * 
 * This file provides a main() function for running Unity tests.
 * It can be compiled as a separate executable or integrated into main_test.c
 */

#include "unity.h"

/* Forward declarations for Unity infrastructure tests */
extern void test_unity_assertions(void);
extern void test_unity_runner(void);
extern void test_unity_suite_organization(void);
extern void test_test_helpers_temp_file(void);
extern void test_unity_fixtures_available(void);

int main(void) {
    UNITY_BEGIN();
    
    /* Run Unity infrastructure tests */
    RUN_TEST(test_unity_assertions);
    RUN_TEST(test_unity_runner);
    RUN_TEST(test_unity_suite_organization);
    RUN_TEST(test_test_helpers_temp_file);
    RUN_TEST(test_unity_fixtures_available);
    
    return UNITY_END();
}

