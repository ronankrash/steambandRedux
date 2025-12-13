/* File: src/tests/test_helpers.h
 * Test helper utilities for Unity framework
 */

#ifndef TEST_HELPERS_H
#define TEST_HELPERS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef WINDOWS
#include <windows.h>
#include <direct.h>
#include <io.h>
#include <errno.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#endif

/* Temporary file management helpers for file I/O tests */
typedef struct {
    char path[1024];
    int created;
} test_file_t;

/* Create a temporary test file */
static inline test_file_t test_create_temp_file(const char* prefix) {
    test_file_t tf;
    tf.created = 0;
    
#ifdef WINDOWS
    char tmp_dir[MAX_PATH];
    if (GetTempPath(MAX_PATH, tmp_dir) > 0) {
        snprintf(tf.path, sizeof(tf.path), "%s%s_%d.tmp", tmp_dir, prefix, (int)GetTickCount());
    } else {
        snprintf(tf.path, sizeof(tf.path), "%s_%d.tmp", prefix, (int)GetTickCount());
    }
#else
    snprintf(tf.path, sizeof(tf.path), "/tmp/%s_%d.tmp", prefix, getpid());
#endif
    
    FILE* f = fopen(tf.path, "w");
    if (f) {
        fclose(f);
        tf.created = 1;
    }
    
    return tf;
}

/* Clean up temporary test file */
static inline void test_cleanup_temp_file(test_file_t* tf) {
    if (tf->created) {
        remove(tf->path);
        tf->created = 0;
    }
}

/* Create a temporary test directory */
static inline int test_create_temp_dir(char* path, size_t path_size, const char* prefix) {
#ifdef WINDOWS
    char tmp_dir[MAX_PATH];
    if (GetTempPath(MAX_PATH, tmp_dir) > 0) {
        snprintf(path, path_size, "%s%s_%d", tmp_dir, prefix, (int)GetTickCount());
    } else {
        snprintf(path, path_size, "%s_%d", prefix, (int)GetTickCount());
    }
    return _mkdir(path) == 0 || errno == EEXIST;
#else
    snprintf(path, path_size, "/tmp/%s_%d", prefix, getpid());
    return mkdir(path, 0755) == 0 || errno == EEXIST;
#endif
}

/* Remove a temporary test directory */
static inline void test_remove_temp_dir(const char* path) {
#ifdef WINDOWS
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "rmdir /s /q \"%s\" 2>nul", path);
    system(cmd);
#else
    char cmd[2048];
    snprintf(cmd, sizeof(cmd), "rm -rf \"%s\"", path);
    system(cmd);
#endif
}

/* Check if a file exists */
static inline int test_file_exists(const char* path) {
#ifdef WINDOWS
    return _access(path, 0) == 0;
#else
    return access(path, F_OK) == 0;
#endif
}

#endif /* TEST_HELPERS_H */

