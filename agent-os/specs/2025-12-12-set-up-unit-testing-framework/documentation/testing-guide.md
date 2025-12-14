# Testing Guide for SteambandRedux

This guide explains how to write and run unit tests for the SteambandRedux project using the Unity testing framework.

## Table of Contents

1. [Overview](#overview)
2. [Running Tests](#running-tests)
3. [Writing Tests](#writing-tests)
4. [Test Organization](#test-organization)
5. [Unity Assertion Macros](#unity-assertion-macros)
6. [Test Fixtures](#test-fixtures)
7. [Test Helpers](#test-helpers)
8. [Best Practices](#best-practices)

## Overview

The SteambandRedux project uses the **Unity** testing framework, a lightweight C unit testing framework designed for embedded systems and C projects. Unity provides:

- Simple, intuitive assertion macros
- Clear test failure reporting with file and line numbers
- Test organization and grouping
- Setup and teardown functions for test fixtures
- Integration with CMake and `ctest`

## Running Tests

### Using CMake and ctest

The recommended way to run tests is through CMake's testing infrastructure:

```bash
# Configure CMake (if not already done)
cmake -S . -B build

# Build the UnityTestRunner executable (Debug configuration)
cmake --build build --config Debug --target UnityTestRunner

# Run all tests via ctest
cd build
ctest -C Debug
```

### Running Tests Directly

You can also run the test executable directly:

```bash
cd build/Debug
./UnityTestRunner.exe
```

This will output detailed test results showing which tests passed or failed.

## Writing Tests

### Basic Test Structure

A Unity test is a function that returns `void` and follows the naming convention `test_<description>`:

```c
#include "unity.h"
#include "../z-util.h"

void test_streq_basic(void) {
    TEST_ASSERT_TRUE(streq("hello", "hello"));
    TEST_ASSERT_FALSE(streq("hello", "world"));
}
```

### Test File Organization

Tests are organized in `src/tests/` with one test file per source module:

- `test_logging_unity.c` - Tests for `logging.c`
- `test_z_util.c` - Tests for `z-util.c`
- `test_unity_infrastructure.c` - Tests for Unity framework itself

### Registering Tests

All tests must be registered in `src/tests/unity_test_runner.c`:

```c
/* Forward declaration */
extern void test_streq_basic(void);

int main(void) {
    UNITY_BEGIN();
    
    RUN_TEST(test_streq_basic);
    
    return UNITY_END();
}
```

## Test Organization

### Arrange-Act-Assert Pattern

Follow the Arrange-Act-Assert (AAA) pattern for clear test structure:

```c
void test_my_strcpy_basic(void) {
    /* Arrange: Set up test data */
    char buf[100];
    size_t ret;
    
    /* Act: Execute the function under test */
    ret = my_strcpy(buf, "hello", sizeof(buf));
    
    /* Assert: Verify the results */
    TEST_ASSERT_EQUAL_UINT(5, ret);
    TEST_ASSERT_EQUAL_STRING("hello", buf);
}
```

### Test Naming Conventions

- Use descriptive names: `test_<function>_<scenario>`
- Examples:
  - `test_streq_basic` - Basic functionality test
  - `test_my_strcpy_buffer_overflow` - Edge case test
  - `test_log_level_filtering` - Feature-specific test

## Unity Assertion Macros

Unity provides a comprehensive set of assertion macros:

### Boolean Assertions

```c
TEST_ASSERT_TRUE(condition);   // Assert condition is true
TEST_ASSERT_FALSE(condition);  // Assert condition is false
```

### Integer Assertions

```c
TEST_ASSERT_EQUAL_INT(expected, actual);
TEST_ASSERT_EQUAL_UINT(expected, actual);
TEST_ASSERT_NOT_EQUAL(expected, actual);
TEST_ASSERT_GREATER_THAN(threshold, actual);
TEST_ASSERT_LESS_THAN(threshold, actual);
TEST_ASSERT_GREATER_OR_EQUAL(threshold, actual);
TEST_ASSERT_LESS_OR_EQUAL(threshold, actual);
```

### String Assertions

```c
TEST_ASSERT_EQUAL_STRING(expected, actual);
TEST_ASSERT_NOT_EQUAL_STRING(expected, actual);
```

### Pointer Assertions

```c
TEST_ASSERT_NULL(pointer);
TEST_ASSERT_NOT_NULL(pointer);
```

### Character Assertions

```c
TEST_ASSERT_EQUAL_CHAR(expected, actual);
```

### Custom Messages

You can add custom failure messages:

```c
TEST_FAIL_MESSAGE("Custom failure message");
```

## Test Fixtures

Unity supports setup and teardown functions that run before and after each test:

### Global Setup/Teardown

Defined in `src/tests/unity_integration.c`:

```c
void setUp(void) {
    /* Set up test fixtures before each test */
}

void tearDown(void) {
    /* Clean up after each test */
}
```

### Per-Test Setup/Teardown

For test-specific setup, use local variables and helper functions:

```c
void test_file_operations(void) {
    test_file_t tf = test_create_temp_file("test_file");
    
    /* Use temporary file for testing */
    FILE *f = fopen(tf.path, "w");
    TEST_ASSERT_NOT_NULL(f);
    
    /* Cleanup */
    test_cleanup_temp_file(&tf);
}
```

## Test Helpers

The project provides helper functions in `src/tests/test_helpers.h`:

### Temporary File Management

```c
/* Create a temporary file */
test_file_t test_create_temp_file(const char *base_name);

/* Check if a file exists */
int test_file_exists(const char *path);

/* Clean up a temporary file */
void test_cleanup_temp_file(test_file_t *tf);
```

### Example Usage

```c
void test_logging_file_creation(void) {
    test_file_t tf = test_create_temp_file("test_log");
    TEST_ASSERT_TRUE(tf.created);
    
    log_init(tf.path);
    log_msg(LOG_INFO, "Test message");
    log_close();
    
    /* Verify file was created */
    TEST_ASSERT_TRUE(test_file_exists(tf.path));
    
    /* Cleanup */
    test_cleanup_temp_file(&tf);
}
```

## Best Practices

### 1. Test Isolation

Each test should be independent and not rely on state from other tests:

```c
/* Good: Each test creates its own temporary file */
void test_a(void) {
    test_file_t tf = test_create_temp_file("test_a");
    /* ... */
    test_cleanup_temp_file(&tf);
}

void test_b(void) {
    test_file_t tf = test_create_temp_file("test_b");
    /* ... */
    test_cleanup_temp_file(&tf);
}
```

### 2. Clear Test Names

Use descriptive names that explain what is being tested:

```c
/* Good */
void test_my_strcpy_truncates_long_strings(void);

/* Bad */
void test_strcpy(void);
```

### 3. Test Edge Cases

Include tests for boundary conditions and error cases:

```c
void test_my_strcpy_zero_buffer(void) {
    char buf[1];
    size_t ret = my_strcpy(buf, "hello", 0);
    TEST_ASSERT_EQUAL_UINT(5, ret); /* Returns strlen(src) */
}

void test_my_strcpy_buffer_overflow(void) {
    char buf[10];
    const char *long_str = "this is a very long string";
    size_t ret = my_strcpy(buf, long_str, sizeof(buf));
    TEST_ASSERT_GREATER_THAN(sizeof(buf), ret);
    TEST_ASSERT_EQUAL_UINT(9, strlen(buf)); /* Truncated */
}
```

### 4. Clean Up Resources

Always clean up temporary files, memory, or other resources:

```c
void test_with_cleanup(void) {
    test_file_t tf = test_create_temp_file("test");
    
    /* ... test code ... */
    
    /* Always cleanup */
    test_cleanup_temp_file(&tf);
}
```

### 5. Use Appropriate Assertions

Choose the most specific assertion macro for better error messages:

```c
/* Good: Specific assertion */
TEST_ASSERT_EQUAL_STRING("expected", actual);

/* Less ideal: Generic assertion */
TEST_ASSERT_TRUE(strcmp("expected", actual) == 0);
```

### 6. Test One Thing Per Test

Each test should verify one specific behavior:

```c
/* Good: Focused test */
void test_streq_returns_true_for_equal_strings(void) {
    TEST_ASSERT_TRUE(streq("hello", "hello"));
}

/* Less ideal: Multiple behaviors */
void test_streq_all_cases(void) {
    TEST_ASSERT_TRUE(streq("hello", "hello"));
    TEST_ASSERT_FALSE(streq("hello", "world"));
    /* ... many more assertions ... */
}
```

## Examples

### Complete Test Example

```c
/* File: src/tests/test_z_util.c */
#include "unity.h"
#include "../z-util.h"
#include <string.h>

void test_my_strcpy_basic(void) {
    /* Arrange */
    char buf[100];
    size_t ret;
    
    /* Act */
    ret = my_strcpy(buf, "hello", sizeof(buf));
    
    /* Assert */
    TEST_ASSERT_EQUAL_UINT(5, ret);
    TEST_ASSERT_EQUAL_STRING("hello", buf);
    TEST_ASSERT_EQUAL_CHAR('\0', buf[5]);
}

void test_my_strcpy_buffer_overflow(void) {
    /* Arrange */
    char buf[10];
    const char *long_str = "this is a very long string";
    
    /* Act */
    size_t ret = my_strcpy(buf, long_str, sizeof(buf));
    
    /* Assert */
    TEST_ASSERT_GREATER_THAN(sizeof(buf), ret);
    TEST_ASSERT_EQUAL_UINT(9, strlen(buf));
    TEST_ASSERT_EQUAL_CHAR('\0', buf[9]);
}
```

## Migration from Old Test Framework

If you have tests using the old custom test framework (`test_framework.h`), here's how to migrate:

### Old Framework

```c
#include "test_framework.h"

int test_example(void) {
    ASSERT(condition);
    return 1; /* Success */
}
```

### Unity Framework

```c
#include "unity.h"

void test_example(void) {
    TEST_ASSERT_TRUE(condition);
    /* No return value needed */
}
```

### Key Differences

1. **Return Type**: Old framework used `int`, Unity uses `void`
2. **Return Value**: Old framework returned `1` for success, Unity doesn't return
3. **Assertions**: Old framework used `ASSERT()`, Unity uses `TEST_ASSERT_*` macros
4. **Test Registration**: Old framework used function pointers, Unity uses `RUN_TEST()` macro

## Troubleshooting

### Tests Not Running

1. Ensure tests are registered in `unity_test_runner.c`
2. Check that test files are included in `CMakeLists.txt`
3. Verify the build succeeded: `cmake --build build --config Debug --target UnityTestRunner`

### Link Errors

If you get unresolved symbol errors:
1. Ensure source files are included in `CMakeLists.txt` for `UnityTestRunner`
2. Check that required headers are included
3. Verify function declarations match implementations

### Test Failures

When a test fails, Unity will show:
- File and line number of the failure
- Expected vs actual values
- The assertion that failed

Use this information to debug the issue.

## Resources

- [Unity Framework Documentation](https://github.com/ThrowTheSwitch/Unity)
- [CMake Testing Documentation](https://cmake.org/cmake/help/latest/manual/ctest.1.html)
- Project test files in `src/tests/` for examples

