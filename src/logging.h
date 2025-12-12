/* File: logging.h */
#ifndef INCLUDED_LOGGING_H
#define INCLUDED_LOGGING_H

#include <stdarg.h>

/*
 * Log levels
 */
typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_FATAL
} log_level_t;

/*
 * Initialize logging
 * @param filename: Path to log file (NULL for no file output)
 */
void log_init(const char *filename);

/*
 * Close logging
 */
void log_close(void);

/*
 * Log a message
 */
void log_msg(log_level_t level, const char *fmt, ...);

/*
 * Convenience macros
 */
#define LOG_D(fmt, ...) log_msg(LOG_DEBUG, fmt, ##__VA_ARGS__)
#define LOG_I(fmt, ...) log_msg(LOG_INFO, fmt, ##__VA_ARGS__)
#define LOG_W(fmt, ...) log_msg(LOG_WARN, fmt, ##__VA_ARGS__)
#define LOG_E(fmt, ...) log_msg(LOG_ERROR, fmt, ##__VA_ARGS__)
#define LOG_F(fmt, ...) log_msg(LOG_FATAL, fmt, ##__VA_ARGS__)

#endif /* INCLUDED_LOGGING_H */

