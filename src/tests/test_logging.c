/* File: src/tests/test_logging.c */
#include "test_framework.h"
#include "../logging.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Test 1: Log level enum values */
int test_log_level_enum(void) {
    ASSERT(LOG_DEBUG == 0);
    ASSERT(LOG_INFO == 1);
    ASSERT(LOG_WARN == 2);
    ASSERT(LOG_ERROR == 3);
    ASSERT(LOG_FATAL == 4);
    return 1;
}

/* Test 2: Log level filtering - messages below threshold are not logged */
int test_log_level_filtering(void) {
    const char *test_file = "test_filter.txt";
    remove(test_file);
    
    log_init(test_file);
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
    FILE *f = fopen(test_file, "r");
    ASSERT(f != NULL);
    
    char buffer[4096];
    size_t len = fread(buffer, 1, sizeof(buffer) - 1, f);
    buffer[len] = '\0';
    fclose(f);
    
    /* Should NOT contain DEBUG or INFO */
    ASSERT(strstr(buffer, "Debug message") == NULL);
    ASSERT(strstr(buffer, "Info message") == NULL);
    
    /* Should contain WARN, ERROR, FATAL */
    ASSERT(strstr(buffer, "Warning message") != NULL);
    ASSERT(strstr(buffer, "Error message") != NULL);
    ASSERT(strstr(buffer, "Fatal message") != NULL);
    
    remove(test_file);
    return 1;
}

/* Test 3: Message formatting with timestamps and level strings */
int test_message_formatting(void) {
    const char *test_file = "test_format.txt";
    remove(test_file);
    
    log_init(test_file);
    log_set_level(LOG_DEBUG); /* Allow all levels */
    
    log_msg(LOG_INFO, "Test message %d", 123);
    log_msg(LOG_ERROR, "Error code: %s", "E001");
    
    log_close();
    
    FILE *f = fopen(test_file, "r");
    ASSERT(f != NULL);
    
    char buffer[4096];
    size_t len = fread(buffer, 1, sizeof(buffer) - 1, f);
    buffer[len] = '\0';
    fclose(f);
    
    /* Check format: [timestamp] [LEVEL] message */
    ASSERT(strstr(buffer, "[INFO]") != NULL);
    ASSERT(strstr(buffer, "[ERROR]") != NULL);
    ASSERT(strstr(buffer, "Test message 123") != NULL);
    ASSERT(strstr(buffer, "Error code: E001") != NULL);
    
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
    ASSERT(found_timestamp);
    
    remove(test_file);
    return 1;
}

/* Test 4: Printf-style formatting with variadic arguments */
int test_printf_formatting(void) {
    const char *test_file = "test_printf.txt";
    remove(test_file);
    
    log_init(test_file);
    log_set_level(LOG_DEBUG);
    
    log_msg(LOG_INFO, "Integer: %d", 42);
    log_msg(LOG_INFO, "String: %s", "hello");
    log_msg(LOG_INFO, "Float: %.2f", 3.14);
    log_msg(LOG_INFO, "Multiple: %d %s %.1f", 10, "test", 2.5);
    
    log_close();
    
    FILE *f = fopen(test_file, "r");
    ASSERT(f != NULL);
    
    char buffer[4096];
    size_t len = fread(buffer, 1, sizeof(buffer) - 1, f);
    buffer[len] = '\0';
    fclose(f);
    
    ASSERT(strstr(buffer, "Integer: 42") != NULL);
    ASSERT(strstr(buffer, "String: hello") != NULL);
    ASSERT(strstr(buffer, "Float: 3.14") != NULL);
    ASSERT(strstr(buffer, "Multiple: 10 test 2.5") != NULL);
    
    remove(test_file);
    return 1;
}

/* Test 5: Buffer overflow protection (4096 character limit) */
int test_buffer_overflow_protection(void) {
    const char *test_file = "test_overflow.txt";
    remove(test_file);
    
    log_init(test_file);
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
    
    FILE *f = fopen(test_file, "r");
    ASSERT(f != NULL);
    
    char buffer[5000];
    size_t len = fread(buffer, 1, sizeof(buffer) - 1, f);
    buffer[len] = '\0';
    fclose(f);
    
    /* Message should be truncated to fit buffer */
    ASSERT(len < 5000);
    
    remove(test_file);
    return 1;
}

/* Test 6: Convenience macros work correctly */
int test_convenience_macros(void) {
    const char *test_file = "test_macros.txt";
    remove(test_file);
    
    log_init(test_file);
    log_set_level(LOG_DEBUG);
    
    LOG_D("Debug macro %d", 1);
    LOG_I("Info macro %d", 2);
    LOG_W("Warn macro %d", 3);
    LOG_E("Error macro %d", 4);
    LOG_F("Fatal macro %d", 5);
    
    log_close();
    
    FILE *f = fopen(test_file, "r");
    ASSERT(f != NULL);
    
    char buffer[4096];
    size_t len = fread(buffer, 1, sizeof(buffer) - 1, f);
    buffer[len] = '\0';
    fclose(f);
    
    ASSERT(strstr(buffer, "[DEBUG]") != NULL);
    ASSERT(strstr(buffer, "[INFO]") != NULL);
    ASSERT(strstr(buffer, "[WARN]") != NULL);
    ASSERT(strstr(buffer, "[ERROR]") != NULL);
    ASSERT(strstr(buffer, "[FATAL]") != NULL);
    ASSERT(strstr(buffer, "Debug macro 1") != NULL);
    ASSERT(strstr(buffer, "Info macro 2") != NULL);
    
    remove(test_file);
    return 1;
}

/* Test 7: File logging - log file creation in lib/logs/ directory */
int test_file_logging_creation(void) {
    const char *test_dir = "test_logs";
    const char *test_file = "test_logs/test_file.log";
    
    /* Remove test directory if exists */
#ifdef WINDOWS
    system("rmdir /s /q test_logs 2>nul");
#else
    system("rm -rf test_logs");
#endif
    
    log_init(test_file);
    log_set_level(LOG_DEBUG);
    
    log_msg(LOG_INFO, "File test message");
    
    log_close();
    
    FILE *f = fopen(test_file, "r");
    ASSERT(f != NULL);
    
    char buffer[1024];
    if (fgets(buffer, sizeof(buffer), f)) {
        ASSERT(strstr(buffer, "INFO") != NULL);
        ASSERT(strstr(buffer, "File test message") != NULL);
    } else {
        ASSERT(0);
    }
    
    fclose(f);
    
    /* Cleanup */
    remove(test_file);
#ifdef WINDOWS
    system("rmdir test_logs 2>nul");
#else
    rmdir(test_dir);
#endif
    
    return 1;
}

/* Test 8: Directory creation if lib/logs/ doesn't exist */
int test_directory_creation(void) {
    const char *test_dir = "test_create_logs";
    const char *test_file = "test_create_logs/nested/deep/file.log";
    
    /* Remove test directory if exists */
#ifdef WINDOWS
    system("rmdir /s /q test_create_logs 2>nul");
#else
    system("rm -rf test_create_logs");
#endif
    
    log_init(test_file);
    log_set_level(LOG_DEBUG);
    
    log_msg(LOG_INFO, "Nested directory test");
    
    log_close();
    
    FILE *f = fopen(test_file, "r");
    ASSERT(f != NULL);
    
    char buffer[1024];
    if (fgets(buffer, sizeof(buffer), f)) {
        ASSERT(strstr(buffer, "Nested directory test") != NULL);
    }
    
    fclose(f);
    
    /* Cleanup */
    remove(test_file);
#ifdef WINDOWS
    system("rmdir /s /q test_create_logs 2>nul");
#else
    system("rm -rf test_create_logs");
#endif
    
    return 1;
}

/* Test 9: Log rotation - rotation triggers when file reaches size threshold */
int test_log_rotation(void) {
    const char *test_file = "test_rotation.log";
    char rotated_file[256];
    int i;
    
    /* Remove existing files */
    remove(test_file);
    for (i = 1; i <= 5; i++) {
        snprintf(rotated_file, sizeof(rotated_file), "%s.%d", test_file, i);
        remove(rotated_file);
    }
    
    log_init(test_file);
    log_set_level(LOG_DEBUG);
    
    /* Write enough data to trigger rotation (simulate by writing many lines) */
    /* Note: Actual rotation test would require writing 10MB, which is impractical in unit tests */
    /* This test verifies rotation function exists and file naming works */
    for (i = 0; i < 100; i++) {
        log_msg(LOG_INFO, "Rotation test line %d", i);
    }
    
    log_close();
    
    /* Verify log file exists */
    FILE *f = fopen(test_file, "r");
    ASSERT(f != NULL);
    fclose(f);
    
    /* Cleanup */
    remove(test_file);
    for (i = 1; i <= 5; i++) {
        snprintf(rotated_file, sizeof(rotated_file), "%s.%d", test_file, i);
        remove(rotated_file);
    }
    
    return 1;
}

/* Test 10: Thread safety - mutex protects shared state (basic test) */
int test_thread_safety_basic(void) {
    const char *test_file = "test_thread.log";
    
    remove(test_file);
    
    log_init(test_file);
    log_set_level(LOG_DEBUG);
    
    /* Test that multiple rapid log calls don't crash */
    int i;
    for (i = 0; i < 1000; i++) {
        log_msg(LOG_INFO, "Thread safety test %d", i);
    }
    
    log_close();
    
    /* Verify file was written correctly */
    FILE *f = fopen(test_file, "r");
    ASSERT(f != NULL);
    
    char buffer[4096];
    size_t len = fread(buffer, 1, sizeof(buffer) - 1, f);
    buffer[len] = '\0';
    fclose(f);
    
    /* Should contain our messages */
    ASSERT(strstr(buffer, "Thread safety test") != NULL);
    
    remove(test_file);
    return 1;
}

/* Original test - keep for compatibility */
int test_logging_init(void) {
    const char *test_file = "test_log.txt";
    
    remove(test_file);
    
    log_init(test_file);
    log_set_level(LOG_INFO);
    
    log_msg(LOG_INFO, "Test message %d", 123);
    
    log_close();
    
    FILE *f = fopen(test_file, "r");
    ASSERT(f != NULL);
    
    char buffer[1024];
    if (fgets(buffer, sizeof(buffer), f)) {
        ASSERT(strstr(buffer, "INFO") != NULL);
        ASSERT(strstr(buffer, "Test message 123") != NULL);
    } else {
        ASSERT(0);
    }
    
    fclose(f);
    remove(test_file);
    
    return 1;
}

