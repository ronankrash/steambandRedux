/* File: logging.c */
#include "logging.h"
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#ifdef WINDOWS
#include <io.h>
#else
#include <unistd.h>
#endif

#ifdef WINDOWS
#include <windows.h>
#include <io.h>
#include <direct.h>
#define PATH_SEP "\\"
#define mkdir(path, mode) _mkdir(path)
#else
#include <sys/stat.h>
#include <unistd.h>
#define PATH_SEP "/"
#endif

static FILE *log_file = NULL;
static log_level_t min_log_level = LOG_INFO; /* Default to INFO in Release */
static char log_file_path[1024] = {0};
#define LOG_ROTATION_SIZE (10 * 1024 * 1024) /* 10MB */
#define LOG_ROTATION_COUNT 5 /* Keep 5 rotated files */

#ifdef WINDOWS
static HANDLE log_mutex = NULL;
#endif

/* Helper function to ensure directory exists */
static int ensure_directory_exists(const char *filepath) {
    char dir_path[1024];
    char *last_sep;
    size_t len;
    
    /* Copy filepath */
    strncpy(dir_path, filepath, sizeof(dir_path) - 1);
    dir_path[sizeof(dir_path) - 1] = '\0';
    
    /* Find last path separator */
#ifdef WINDOWS
    last_sep = strrchr(dir_path, '\\');
    if (!last_sep) {
        last_sep = strrchr(dir_path, '/');
    }
#else
    last_sep = strrchr(dir_path, '/');
#endif
    
    if (!last_sep) {
        /* No directory component, current directory */
        return 0;
    }
    
    /* Null-terminate at last separator */
    *last_sep = '\0';
    len = strlen(dir_path);
    
    /* Skip if empty or root */
    if (len == 0 || (len == 1 && (dir_path[0] == '/' || dir_path[0] == '\\'))) {
        return 0;
    }
    
    /* Check if directory exists */
#ifdef WINDOWS
    {
        DWORD attrib = GetFileAttributes(dir_path);
        if (attrib != INVALID_FILE_ATTRIBUTES && (attrib & FILE_ATTRIBUTE_DIRECTORY)) {
            return 0; /* Directory exists */
        }
    }
#else
    {
        struct stat st;
        if (stat(dir_path, &st) == 0 && S_ISDIR(st.st_mode)) {
            return 0; /* Directory exists */
        }
    }
#endif
    
    /* Try to create directory */
    if (mkdir(dir_path, 0755) != 0) {
        /* If creation failed, check if parent exists and create recursively */
        if (errno == ENOENT) {
            /* Parent doesn't exist, create it first */
            if (ensure_directory_exists(dir_path) != 0) {
                return -1;
            }
            /* Try again */
            if (mkdir(dir_path, 0755) != 0 && errno != EEXIST) {
                return -1;
            }
        } else if (errno != EEXIST) {
            return -1;
        }
    }
    
    return 0;
}

/* Rotate log files when size exceeds threshold */
static void rotate_log_file(void) {
    char old_path[1024];
    char new_path[1024];
    int i;
    
    if (!log_file || log_file_path[0] == '\0') {
        return;
    }
    
    /* Close current file */
    fclose(log_file);
    log_file = NULL;
    
    /* Delete oldest file (steamband.log.5) */
    snprintf(old_path, sizeof(old_path), "%s.5", log_file_path);
    old_path[sizeof(old_path) - 1] = '\0';
    remove(old_path);
    
    /* Rotate files: .4 -> .5, .3 -> .4, ..., .1 -> .2, current -> .1 */
    for (i = LOG_ROTATION_COUNT - 1; i >= 1; i--) {
        if (i == LOG_ROTATION_COUNT - 1) {
            snprintf(old_path, sizeof(old_path), "%s.%d", log_file_path, i);
        } else {
            strncpy(old_path, new_path, sizeof(old_path) - 1);
            old_path[sizeof(old_path) - 1] = '\0';
        }
        snprintf(new_path, sizeof(new_path), "%s.%d", log_file_path, i + 1);
        new_path[sizeof(new_path) - 1] = '\0';
        
        /* Rename file if it exists */
#ifdef WINDOWS
        if (i < LOG_ROTATION_COUNT - 1 || _access(old_path, 0) == 0) {
#else
        if (i < LOG_ROTATION_COUNT - 1 || access(old_path, 0) == 0) {
#endif
            rename(old_path, new_path);
        }
    }
    
    /* Rename current log file to .1 */
    snprintf(new_path, sizeof(new_path), "%s.1", log_file_path);
    new_path[sizeof(new_path) - 1] = '\0';
    rename(log_file_path, new_path);
    
    /* Open new log file */
    log_file = fopen(log_file_path, "w");
    if (!log_file) {
        /* If write fails, try append */
        log_file = fopen(log_file_path, "a");
    }
}

/* Check if log file needs rotation */
static void check_log_rotation(void) {
    long file_size;
    
    if (!log_file || log_file_path[0] == '\0') {
        return;
    }
    
    /* Get current file size */
    fseek(log_file, 0, SEEK_END);
    file_size = ftell(log_file);
    fseek(log_file, 0, SEEK_END); /* Return to end for appending */
    
    /* Rotate if size exceeds threshold */
    if (file_size >= LOG_ROTATION_SIZE) {
        rotate_log_file();
    }
}

void log_init(const char *filename) {
#ifdef WINDOWS
    /* Create mutex for thread safety */
    if (!log_mutex) {
        log_mutex = CreateMutex(NULL, FALSE, NULL);
        if (!log_mutex) {
            /* Mutex creation failed, continue without thread safety */
        }
    }
#endif
    if (filename) {
        /* Store the log file path */
        strncpy(log_file_path, filename, sizeof(log_file_path) - 1);
        log_file_path[sizeof(log_file_path) - 1] = '\0';
        
        /* Ensure directory exists */
        if (ensure_directory_exists(filename) != 0) {
            /* Directory creation failed, log to console only */
            log_file = NULL;
        } else {
            /* Try to open file in append mode */
            log_file = fopen(filename, "a");
            if (!log_file) {
                /* If append fails, try write mode */
                log_file = fopen(filename, "w");
            }
            
            if (!log_file) {
                /* File open failed, continue without file logging */
                /* Error will be visible in debug console */
            }
        }
    } else {
        log_file_path[0] = '\0';
        log_file = NULL;
    }
    
    /* Set default log level based on build type */
#ifdef _DEBUG
    min_log_level = LOG_DEBUG; /* DEBUG in Debug builds */
#else
    min_log_level = LOG_INFO;  /* INFO in Release builds */
#endif
}

void log_close(void) {
#ifdef WINDOWS
    /* Acquire mutex */
    if (log_mutex) {
        WaitForSingleObject(log_mutex, INFINITE);
    }
#endif
    
    if (log_file) {
        fflush(log_file);
        fclose(log_file);
        log_file = NULL;
    }
    
#ifdef WINDOWS
    /* Release mutex */
    if (log_mutex) {
        ReleaseMutex(log_mutex);
    }
    
    /* Close mutex handle */
    if (log_mutex) {
        CloseHandle(log_mutex);
        log_mutex = NULL;
    }
#endif
}

void log_flush(void) {
#ifdef WINDOWS
    /* Acquire mutex */
    if (log_mutex) {
        WaitForSingleObject(log_mutex, INFINITE);
    }
#endif
    
    if (log_file) {
        fflush(log_file);
    }
    
#ifdef WINDOWS
    /* Release mutex */
    if (log_mutex) {
        ReleaseMutex(log_mutex);
    }
#endif
}

void log_set_level(log_level_t level) {
    min_log_level = level;
}

log_level_t log_get_level(void) {
    return min_log_level;
}

void log_msg(log_level_t level, const char *fmt, ...) {
    /* Early return if message level is below threshold */
    if (level < min_log_level) {
        return;
    }
    
#ifdef WINDOWS
    /* Acquire mutex for thread safety */
    if (log_mutex) {
        WaitForSingleObject(log_mutex, INFINITE);
    }
#endif
    
    /* Check if rotation is needed */
    check_log_rotation();
    
    va_list args;
    char buffer[4096];
    char timestamp[32];
    time_t now;
    struct tm *tm_info;
    const char *level_str;

    /* Get timestamp */
    time(&now);
    tm_info = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

    /* Get level string */
    switch (level) {
        case LOG_DEBUG: level_str = "DEBUG"; break;
        case LOG_INFO:  level_str = "INFO";  break;
        case LOG_WARN:  level_str = "WARN";  break;
        case LOG_ERROR: level_str = "ERROR"; break;
        case LOG_FATAL: level_str = "FATAL"; break;
        default:        level_str = "UNKNOWN"; break;
    }

    /* Format message - use vsnprintf to prevent buffer overflow */
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    
    /* Ensure null termination */
    buffer[sizeof(buffer) - 1] = '\0';

    /* Output to file */
    if (log_file) {
        fprintf(log_file, "[%s] [%s] %s\n", timestamp, level_str, buffer);
        fflush(log_file);
    }

    /* Output to debug console (Windows) */
#ifdef WINDOWS
    {
        char debug_msg[4096];
        snprintf(debug_msg, sizeof(debug_msg), "[%s] [%s] %s\n", timestamp, level_str, buffer);
        debug_msg[sizeof(debug_msg) - 1] = '\0';
        OutputDebugString(debug_msg);
    }
#else
    /* Output to stderr for other platforms */
    fprintf(stderr, "[%s] [%s] %s\n", timestamp, level_str, buffer);
#endif

#ifdef WINDOWS
    /* Release mutex */
    if (log_mutex) {
        ReleaseMutex(log_mutex);
    }
#endif
}

