/* File: logging.c */
#include "logging.h"
#include <stdio.h>
#include <time.h>
#include <string.h>

#ifdef WINDOWS
#include <windows.h>
#endif

static FILE *log_file = NULL;

void log_init(const char *filename) {
    if (filename) {
        log_file = fopen(filename, "w");
    }
}

void log_close(void) {
    if (log_file) {
        fclose(log_file);
        log_file = NULL;
    }
}

void log_msg(log_level_t level, const char *fmt, ...) {
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

    /* Format message */
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

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
        OutputDebugString(debug_msg);
    }
#else
    /* Output to stderr for other platforms */
    fprintf(stderr, "[%s] [%s] %s\n", timestamp, level_str, buffer);
#endif
}

