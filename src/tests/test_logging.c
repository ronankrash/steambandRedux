/* File: src/tests/test_logging.c */
#include "test_framework.h"
#include "../logging.h"
#include <stdio.h>
#include <string.h>

int test_logging_init(void) {
    const char *test_file = "test_log.txt";
    
    /* Remove if exists */
    remove(test_file);
    
    log_init(test_file);
    
    log_msg(LOG_INFO, "Test message %d", 123);
    
    log_close();
    
    /* Check if file exists and has content */
    FILE *f = fopen(test_file, "r");
    ASSERT(f != NULL);
    
    char buffer[1024];
    if (fgets(buffer, sizeof(buffer), f)) {
        ASSERT(strstr(buffer, "INFO") != NULL);
        ASSERT(strstr(buffer, "Test message 123") != NULL);
    } else {
        ASSERT(0); /* Empty file */
    }
    
    fclose(f);
    remove(test_file);
    
    return 1;
}

