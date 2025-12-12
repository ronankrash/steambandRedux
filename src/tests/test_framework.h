/* File: src/tests/test_framework.h */
#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>

static int tests_run = 0;
static int tests_passed = 0;

#define ASSERT(test) do { \
    if (!(test)) { \
        fprintf(stderr, "FAILED: %s (%s:%d)\n", #test, __FILE__, __LINE__); \
        return 0; \
    } \
} while(0)

#define RUN_TEST(test) do { \
    fprintf(stderr, "Running %s...\n", #test); \
    tests_run++; \
    if (test()) { \
        tests_passed++; \
        fprintf(stderr, "PASSED\n"); \
    } \
} while(0)

#define TEST_REPORT() do { \
    fprintf(stderr, "\nTests run: %d\n", tests_run); \
    fprintf(stderr, "Tests passed: %d\n", tests_passed); \
    fprintf(stderr, "Tests failed: %d\n", tests_run - tests_passed); \
} while(0)

#endif

