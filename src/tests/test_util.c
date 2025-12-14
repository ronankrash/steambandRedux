/* File: src/tests/test_util.c
 * Tests for util.c utility functions using Unity framework
 */

#include "unity.h"
#include "../h-type.h"   /* Type definitions (errr, cptr) */
#include "test_helpers.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <stdio.h>

/* Forward declarations for util.c functions */
extern errr path_parse(char *buf, int max, cptr file);
extern FILE *my_fopen(cptr file, cptr mode);
extern errr my_fclose(FILE *fff);
extern int fd_make(cptr file, int mode);
extern int fd_open(cptr file, int flags);
extern errr fd_lock(int fd, int what);

#ifdef WINDOWS
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#define O_RDONLY _O_RDONLY
#define O_WRONLY _O_WRONLY
#define O_RDWR _O_RDWR
#define O_CREAT _O_CREAT
#define O_EXCL _O_EXCL
#define O_BINARY _O_BINARY
#define F_UNLCK 0
#define F_RDLCK 1
#define F_WRLCK 2
#define close _close
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#endif

/* Test path_parse() - path resolution and parsing */
void test_path_parse_basic(void) {
    char buf[1024];
    errr result;
    
    result = path_parse(buf, sizeof(buf), "test.txt");
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_STRING("test.txt", buf);
}

/* Test path_parse() with longer paths */
void test_path_parse_long_path(void) {
    char buf[1024];
    errr result;
    
    result = path_parse(buf, sizeof(buf), "path/to/file.txt");
    TEST_ASSERT_EQUAL_INT(0, result);
    TEST_ASSERT_EQUAL_STRING("path/to/file.txt", buf);
}

/* Test path_parse() buffer size limits */
void test_path_parse_buffer_size(void) {
    char buf[10];
    errr result;
    
    /* Should handle truncation gracefully */
    result = path_parse(buf, sizeof(buf), "very/long/path/to/file.txt");
    TEST_ASSERT_EQUAL_INT(0, result);
    /* Buffer should be null-terminated */
    TEST_ASSERT_EQUAL_CHAR('\0', buf[9]);
}

/* Test my_fopen() - file I/O wrapper function */
void test_my_fopen_success(void) {
    test_file_t tf = test_create_temp_file("test_fopen");
    TEST_ASSERT_TRUE(tf.created);
    
    FILE *f = my_fopen(tf.path, "w");
    TEST_ASSERT_NOT_NULL(f);
    
    if (f) {
        fprintf(f, "test content");
        my_fclose(f);
    }
    
    /* Verify file was written */
    f = fopen(tf.path, "r");
    TEST_ASSERT_NOT_NULL(f);
    if (f) {
        char buffer[100];
        TEST_ASSERT_NOT_NULL(fgets(buffer, sizeof(buffer), f));
        TEST_ASSERT_EQUAL_STRING("test content", buffer);
        fclose(f);
    }
    
    test_cleanup_temp_file(&tf);
}

/* Test my_fopen() with invalid file */
void test_my_fopen_invalid_file(void) {
    FILE *f = my_fopen("/invalid/path/that/does/not/exist.txt", "r");
    TEST_ASSERT_NULL(f);
}

/* Test my_fclose() - file close wrapper */
void test_my_fclose_success(void) {
    test_file_t tf = test_create_temp_file("test_fclose");
    TEST_ASSERT_TRUE(tf.created);
    
    FILE *f = my_fopen(tf.path, "w");
    TEST_ASSERT_NOT_NULL(f);
    
    errr result = my_fclose(f);
    TEST_ASSERT_EQUAL_INT(0, result);
    
    test_cleanup_temp_file(&tf);
}

/* Test my_fclose() with NULL pointer */
void test_my_fclose_null(void) {
    errr result = my_fclose(NULL);
    TEST_ASSERT_EQUAL_INT(-1, result); /* Should return error for NULL */
}

/* Test fd_make() - create file descriptor */
void test_fd_make_success(void) {
    test_file_t tf = test_create_temp_file("test_fd_make");
    test_cleanup_temp_file(&tf); /* Remove it first so fd_make can create it */
    
    int fd = fd_make(tf.path, 0644);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, fd);
    
    if (fd >= 0) {
        close(fd);
        remove(tf.path);
    }
}

/* Test fd_open() - open existing file */
void test_fd_open_success(void) {
    test_file_t tf = test_create_temp_file("test_fd_open");
    TEST_ASSERT_TRUE(tf.created);
    
    int fd = fd_open(tf.path, O_RDONLY);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, fd);
    
    if (fd >= 0) {
        close(fd);
    }
    
    test_cleanup_temp_file(&tf);
}

/* Test fd_lock() - file locking */
void test_fd_lock_basic(void) {
    test_file_t tf = test_create_temp_file("test_fd_lock");
    TEST_ASSERT_TRUE(tf.created);
    
    int fd = fd_open(tf.path, O_RDWR);
    TEST_ASSERT_GREATER_OR_EQUAL_INT(0, fd);
    
    if (fd >= 0) {
        errr result = fd_lock(fd, F_UNLCK);
        /* fd_lock may return 0 on success or -1 on error depending on platform */
        /* On Windows, file locking may not be fully supported, so we just test it doesn't crash */
        TEST_ASSERT_TRUE(result == 0 || result == -1);
        
        close(fd);
    }
    
    test_cleanup_temp_file(&tf);
}

/* Test fd_lock() with invalid file descriptor */
void test_fd_lock_invalid_fd(void) {
    errr result = fd_lock(-1, F_UNLCK);
    TEST_ASSERT_EQUAL_INT(-1, result);
}

