/* File: src/tests/test_z_util.c
 * Tests for z-util.c utility functions using Unity framework
 */

#include "unity.h"
#include "../z-util.h"
#include "../h-type.h"
#include <string.h>
#include <stdlib.h>

/* Test streq() - string equality function */
void test_streq_basic(void) {
    TEST_ASSERT_TRUE(streq("hello", "hello"));
    TEST_ASSERT_FALSE(streq("hello", "world"));
    TEST_ASSERT_FALSE(streq("hello", "HELLO"));
    TEST_ASSERT_TRUE(streq("", ""));
}

/* Test streq() with NULL handling */
void test_streq_null_handling(void) {
    /* Note: streq() uses strcmp which may crash on NULL - test carefully */
    /* For now, test with empty strings */
    TEST_ASSERT_TRUE(streq("", ""));
}

/* Test prefix() - check if string is prefix */
void test_prefix_basic(void) {
    TEST_ASSERT_TRUE(prefix("hello", "he"));
    TEST_ASSERT_TRUE(prefix("hello", "hello"));
    TEST_ASSERT_FALSE(prefix("hello", "world"));
    TEST_ASSERT_FALSE(prefix("hello", "helloworld"));
    TEST_ASSERT_TRUE(prefix("hello", ""));
}

/* Test prefix() edge cases */
void test_prefix_edge_cases(void) {
    TEST_ASSERT_TRUE(prefix("a", "a"));
    TEST_ASSERT_FALSE(prefix("a", "b"));
    TEST_ASSERT_TRUE(prefix("test", "t"));
}

/* Test suffix() - check if string is suffix */
void test_suffix_basic(void) {
    TEST_ASSERT_TRUE(suffix("hello", "lo"));
    TEST_ASSERT_TRUE(suffix("hello", "hello"));
    TEST_ASSERT_FALSE(suffix("hello", "world"));
    TEST_ASSERT_FALSE(suffix("hello", "xhello"));
    TEST_ASSERT_TRUE(suffix("hello", ""));
}

/* Test suffix() edge cases */
void test_suffix_edge_cases(void) {
    TEST_ASSERT_TRUE(suffix("hello", "o"));
    TEST_ASSERT_FALSE(suffix("hello", "h"));
    TEST_ASSERT_FALSE(suffix("short", "verylongsuffix"));
}

/* Test my_strcpy() - safe string copy function */
void test_my_strcpy_basic(void) {
    char buf[100];
    size_t ret;
    
    ret = my_strcpy(buf, "hello", sizeof(buf));
    TEST_ASSERT_EQUAL_UINT(5, ret);
    TEST_ASSERT_EQUAL_STRING("hello", buf);
}

/* Test my_strcpy() buffer overflow protection */
void test_my_strcpy_buffer_overflow(void) {
    char buf[10];
    size_t ret;
    const char *long_str = "this is a very long string that exceeds buffer size";
    
    ret = my_strcpy(buf, long_str, sizeof(buf));
    TEST_ASSERT_GREATER_THAN(sizeof(buf), ret); /* Return value indicates truncation */
    TEST_ASSERT_EQUAL_UINT(9, strlen(buf)); /* Should be truncated to buf_size - 1 */
    TEST_ASSERT_EQUAL_CHAR('\0', buf[9]); /* Should be null-terminated */
}

/* Test my_strcpy() with zero buffer size */
void test_my_strcpy_zero_buffer(void) {
    char buf[1];
    size_t ret;
    
    ret = my_strcpy(buf, "hello", 0);
    TEST_ASSERT_EQUAL_UINT(5, ret); /* Returns strlen(src) even if buffer is 0 */
}

/* Test my_strcpy() with exact buffer size */
void test_my_strcpy_exact_buffer_size(void) {
    char buf[6]; /* Exactly fits "hello" + null terminator */
    size_t ret;
    
    ret = my_strcpy(buf, "hello", sizeof(buf));
    TEST_ASSERT_EQUAL_UINT(5, ret);
    TEST_ASSERT_EQUAL_STRING("hello", buf);
}

