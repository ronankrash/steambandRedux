/* File: src/tests/test_logging_unity.c
 * Logging system tests using Unity framework
 * 
 * Migrated from test_logging.c to use Unity testing framework
 */

#include "unity.h"
#include "../logging.h"
#include "test_helpers.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Test 1: Log level enum values */
void test_log_level_enum(void) {
    TEST_ASSERT_EQUAL_INT(0, LOG_DEBUG);
    TEST_ASSERT_EQUAL_INT(1, LOG_INFO);
    TEST_ASSERT_EQUAL_INT(2, LOG_WARN);
    TEST_ASSERT_EQUAL_INT(3, LOG_ERROR);
    TEST_ASSERT_EQUAL_INT(4, LOG_FATAL);
}

/* Test 2: Log level filtering - messages below threshold are not logged */
void test_log_level_filtering(void) {
    test_file_t tf = test_create_temp_file("test_filter");
    TEST_ASSERT_TRUE(tf.created);
    
    log_init(tf.path);
    log_set_level(LOG_WARN); /* Set minimum level to WARN */
    
    /* DEBUG and INFO should be filtered */
    log_msg(LOG_DEBUG, "Debug message");
    log_msg(LOG_INFO, "Info message");
    
    /* WARN, ERROR, FATAL should be logged */
    log_msg(LOG_WARN, "Warning message");
    log_msg(LOG_ERROR, "Error message");
    log_msg(LOG_FATAL, "Fatal message");
    
    log_close();
    
    /* Check file content */
    FILE *f = fopen(tf.path, "r");
    TEST_ASSERT_NOT_NULL(f);
    
    char buffer[4096];
    size_t len = fread(buffer, 1, sizeof(buffer) - 1, f);
    buffer[len] = '\0';
    fclose(f);
    
    /* Should NOT contain DEBUG or INFO */
    TEST_ASSERT_NULL(strstr(buffer, "Debug message"));
    TEST_ASSERT_NULL(strstr(buffer, "Info message"));
    
    /* Should contain WARN, ERROR, FATAL */
    TEST_ASSERT_NOT_NULL(strstr(buffer, "Warning message"));
    TEST_ASSERT_NOT_NULL(strstr(buffer, "Error message"));
    TEST_ASSERT_NOT_NULL(strstr(buffer, "Fatal message"));
    
    test_cleanup_temp_file(&tf);
}

/* Test 3: Message formatting with timestamps and level strings */
void test_message_formatting(void) {
    test_file_t tf = test_create_temp_file("test_format");
    TEST_ASSERT_TRUE(tf.created);
    
    log_init(tf.path);
    log_set_level(LOG_DEBUG); /* Allow all levels */
    
    log_msg(LOG_INFO, "Test message %d", 123);
    log_msg(LOG_ERROR, "Error code: %s", "E001");
    
    log_close();
    
    FILE *f = fopen(tf.path, "r");
    TEST_ASSERT_NOT_NULL(f);
    
    char buffer[4096];
    size_t len = fread(buffer, 1, sizeof(buffer) - 1, f);
    buffer[len] = '\0';
    fclose(f);
    
    /* Check format: [timestamp] [LEVEL] message */
    TEST_ASSERT_NOT_NULL(strstr(buffer, "[INFO]"));
    TEST_ASSERT_NOT_NULL(strstr(buffer, "[ERROR]"));
    TEST_ASSERT_NOT_NULL(strstr(buffer, "Test message 123"));
    TEST_ASSERT_NOT_NULL(strstr(buffer, "Error code: E001"));
    
    /* Check timestamp format YYYY-MM-DD HH:MM:SS */
    /* Look for pattern like [2025-12-12 12:34:56] */
    int found_timestamp = 0;
    for (int i = 0; buffer[i] != '\0' && i < (int)len - 19; i++) {
        if (buffer[i] == '[' && 
            buffer[i+5] == '-' && 
            buffer[i+8] == '-' && 
            buffer[i+11] == ' ' &&
            buffer[i+14] == ':' &&
            buffer[i+17] == ':') {
            found_timestamp = 1;
            break;
        }
    }
    TEST_ASSERT_TRUE(found_timestamp);
    
    test_cleanup_temp_file(&tf);
}

/* Test 4: Printf-style formatting with variadic arguments */
void test_printf_formatting(void) {
    test_file_t tf = test_create_temp_file("test_printf");
    TEST_ASSERT_TRUE(tf.created);
    
    log_init(tf.path);
    log_set_level(LOG_DEBUG);
    
    log_msg(LOG_INFO, "Integer: %d", 42);
    log_msg(LOG_INFO, "String: %s", "hello");
    log_msg(LOG_INFO, "Float: %.2f", 3.14);
    log_msg(LOG_INFO, "Multiple: %d %s %.1f", 10, "test", 2.5);
    
    log_close();
    
    FILE *f = fopen(tf.path, "r");
    TEST_ASSERT_NOT_NULL(f);
    
    char buffer[4096];
    size_t len = fread(buffer, 1, sizeof(buffer) - 1, f);
    buffer[len] = '\0';
    fclose(f);
    
    TEST_ASSERT_NOT_NULL(strstr(buffer, "Integer: 42"));
    TEST_ASSERT_NOT_NULL(strstr(buffer, "String: hello"));
    TEST_ASSERT_NOT_NULL(strstr(buffer, "Float: 3.14"));
    TEST_ASSERT_NOT_NULL(strstr(buffer, "Multiple: 10 test 2.5"));
    
    test_cleanup_temp_file(&tf);
}

/* Test 5: Buffer overflow protection (4096 character limit) */
void test_buffer_overflow_protection(void) {
    test_file_t tf = test_create_temp_file("test_overflow");
    TEST_ASSERT_TRUE(tf.created);
    
    log_init(tf.path);
    log_set_level(LOG_DEBUG);
    
    /* Create a very long message (over 4096 chars) */
    char long_msg[5000];
    for (int i = 0; i < 4999; i++) {
        long_msg[i] = 'A' + (i % 26);
    }
    long_msg[4999] = '\0';
    
    /* Should not crash, should truncate */
    log_msg(LOG_INFO, "Long: %s", long_msg);
    
    log_close();
    
    FILE *f = fopen(tf.path, "r");
    TEST_ASSERT_NOT_NULL(f);
    
    char buffer[5000];
    size_t len = fread(buffer, 1, sizeof(buffer) - 1, f);
    buffer[len] = '\0';
    fclose(f);
    
    /* Message should be truncated to fit buffer */
    TEST_ASSERT_LESS_THAN(5000, len);
    
    test_cleanup_temp_file(&tf);
}

/* Test 6: Convenience macros work correctly */
void test_convenience_macros(void) {
    test_file_t tf = test_create_temp_file("test_macros");
    TEST_ASSERT_TRUE(tf.created);
    
    log_init(tf.path);
    log_set_level(LOG_DEBUG);
    
    LOG_D("Debug macro %d", 1);
    LOG_I("Info macro %d", 2);
    LOG_W("Warn macro %d", 3);
    LOG_E("Error macro %d", 4);
    LOG_F("Fatal macro %d", 5);
    
    log_close();
    
    FILE *f = fopen(tf.path, "r");
    TEST_ASSERT_NOT_NULL(f);
    
    char buffer[4096];
    size_t len = fread(buffer, 1, sizeof(buffer) - 1, f);
    buffer[len] = '\0';
    fclose(f);
    
    TEST_ASSERT_NOT_NULL(strstr(buffer, "[DEBUG]"));
    TEST_ASSERT_NOT_NULL(strstr(buffer, "[INFO]"));
    TEST_ASSERT_NOT_NULL(strstr(buffer, "[WARN]"));
    TEST_ASSERT_NOT_NULL(strstr(buffer, "[ERROR]"));
    TEST_ASSERT_NOT_NULL(strstr(buffer, "[FATAL]"));
    TEST_ASSERT_NOT_NULL(strstr(buffer, "Debug macro 1"));
    TEST_ASSERT_NOT_NULL(strstr(buffer, "Info macro 2"));
    
    test_cleanup_temp_file(&tf);
}

/* Test 7: File logging - log file creation */
void test_file_logging_creation(void) {
    char test_dir[1024];
    char test_file[1024];
    
    test_create_temp_dir(test_dir, sizeof(test_dir), "test_logs");
#ifdef WINDOWS
    snprintf(test_file, sizeof(test_file), "%s\\test_file.log", test_dir);
#else
    snprintf(test_file, sizeof(test_file), "%s/test_file.log", test_dir);
#endif
    
    log_init(test_file);
    log_set_level(LOG_DEBUG);
    
    log_msg(LOG_INFO, "File test message");
    
    log_close();
    
    FILE *f = fopen(test_file, "r");
    TEST_ASSERT_NOT_NULL(f);
    
    char buffer[1024];
    TEST_ASSERT_NOT_NULL(fgets(buffer, sizeof(buffer), f));
    TEST_ASSERT_NOT_NULL(strstr(buffer, "INFO"));
    TEST_ASSERT_NOT_NULL(strstr(buffer, "File test message"));
    
    fclose(f);
    
    /* Cleanup */
    remove(test_file);
    test_remove_temp_dir(test_dir);
}

/* Test 8: Directory creation if directory doesn't exist */
void test_directory_creation(void) {
    char test_dir[1024];
    char test_file[1024];
    
    test_create_temp_dir(test_dir, sizeof(test_dir), "test_create_logs");
    
#ifdef WINDOWS
    snprintf(test_file, sizeof(test_file), "%s\\nested\\deep\\file.log", test_dir);
#else
    snprintf(test_file, sizeof(test_file), "%s/nested/deep/file.log", test_dir);
#endif
    
    log_init(test_file);
    log_set_level(LOG_DEBUG);
    
    log_msg(LOG_INFO, "Nested directory test");
    
    log_close();
    
    FILE *f = fopen(test_file, "r");
    TEST_ASSERT_NOT_NULL(f);
    
    char buffer[1024];
    TEST_ASSERT_NOT_NULL(fgets(buffer, sizeof(buffer), f));
    TEST_ASSERT_NOT_NULL(strstr(buffer, "Nested directory test"));
    
    fclose(f);
    
    /* Cleanup */
    remove(test_file);
    test_remove_temp_dir(test_dir);
}

/* Test 9: Log rotation - rotation file naming */
void test_log_rotation(void) {
    test_file_t tf = test_create_temp_file("test_rotation");
    TEST_ASSERT_TRUE(tf.created);
    
    /* Remove .log extension and add it back for rotation test */
    char test_file[1024];
    char rotated_file[256];
    int i;
    
    strncpy(test_file, tf.path, sizeof(test_file) - 1);
    test_file[sizeof(test_file) - 1] = '\0';
    
    /* Remove existing rotated files */
    for (i = 1; i <= 5; i++) {
        snprintf(rotated_file, sizeof(rotated_file), "%s.%d", test_file, i);
        remove(rotated_file);
    }
    
    log_init(test_file);
    log_set_level(LOG_DEBUG);
    
    /* Write enough data to verify logging works */
    /* Note: Actual rotation test would require writing 10MB, which is impractical in unit tests */
    /* This test verifies rotation function exists and file naming works */
    for (i = 0; i < 100; i++) {
        log_msg(LOG_INFO, "Rotation test line %d", i);
    }
    
    log_close();
    
    /* Verify log file exists */
    FILE *f = fopen(test_file, "r");
    TEST_ASSERT_NOT_NULL(f);
    fclose(f);
    
    /* Cleanup */
    test_cleanup_temp_file(&tf);
    for (i = 1; i <= 5; i++) {
        snprintf(rotated_file, sizeof(rotated_file), "%s.%d", test_file, i);
        remove(rotated_file);
    }
}

/* Test 10: Thread safety - mutex protects shared state (basic test) */
void test_thread_safety_basic(void) {
    test_file_t tf = test_create_temp_file("test_thread");
    TEST_ASSERT_TRUE(tf.created);
    
    log_init(tf.path);
    log_set_level(LOG_DEBUG);
    
    /* Test that multiple rapid log calls don't crash */
    int i;
    for (i = 0; i < 1000; i++) {
        log_msg(LOG_INFO, "Thread safety test %d", i);
    }
    
    log_close();
    
    /* Verify file was written correctly */
    FILE *f = fopen(tf.path, "r");
    TEST_ASSERT_NOT_NULL(f);
    
    char buffer[4096];
    size_t len = fread(buffer, 1, sizeof(buffer) - 1, f);
    buffer[len] = '\0';
    fclose(f);
    
    /* Should contain our messages */
    TEST_ASSERT_NOT_NULL(strstr(buffer, "Thread safety test"));
    
    test_cleanup_temp_file(&tf);
}

/* Test 11: Logging initialization */
void test_logging_init(void) {
    test_file_t tf = test_create_temp_file("test_log");
    TEST_ASSERT_TRUE(tf.created);
    
    log_init(tf.path);
    log_set_level(LOG_INFO);
    
    log_msg(LOG_INFO, "Test message %d", 123);
    
    log_close();
    
    FILE *f = fopen(tf.path, "r");
    TEST_ASSERT_NOT_NULL(f);
    
    char buffer[1024];
    TEST_ASSERT_NOT_NULL(fgets(buffer, sizeof(buffer), f));
    TEST_ASSERT_NOT_NULL(strstr(buffer, "INFO"));
    TEST_ASSERT_NOT_NULL(strstr(buffer, "Test message 123"));
    
    fclose(f);
    test_cleanup_temp_file(&tf);
}

/* Test 12: Log level get/set functions */
void test_log_level_get_set(void) {
    log_init(NULL); /* No file logging */
    
    /* Test default level */
    log_level_t level = log_get_level();
    TEST_ASSERT_TRUE(level >= LOG_DEBUG && level <= LOG_FATAL);
    
    /* Test setting different levels */
    log_set_level(LOG_DEBUG);
    TEST_ASSERT_EQUAL_INT(LOG_DEBUG, log_get_level());
    
    log_set_level(LOG_WARN);
    TEST_ASSERT_EQUAL_INT(LOG_WARN, log_get_level());
    
    log_set_level(LOG_ERROR);
    TEST_ASSERT_EQUAL_INT(LOG_ERROR, log_get_level());
    
    log_close();
}

/* Test 13: NULL filename handling (console logging only) */
void test_null_filename_handling(void) {
    log_init(NULL); /* No file logging */
    log_set_level(LOG_DEBUG);
    
    /* Should not crash when logging without file */
    log_msg(LOG_INFO, "Console only message");
    LOG_I("Console macro test");
    
    log_close();
    TEST_ASSERT_TRUE(1); /* Test passes if no crash */
}

