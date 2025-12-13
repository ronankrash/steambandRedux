/* File: src/tests/main_test.c */
#include "test_framework.h"

/* Forward declare tests */
int test_log_level_enum(void);
int test_log_level_filtering(void);
int test_message_formatting(void);
int test_printf_formatting(void);
int test_buffer_overflow_protection(void);
int test_convenience_macros(void);
int test_file_logging_creation(void);
int test_directory_creation(void);
int test_log_rotation(void);
int test_thread_safety_basic(void);
int test_logging_init(void);

int main(void) {
    RUN_TEST(test_log_level_enum);
    RUN_TEST(test_log_level_filtering);
    RUN_TEST(test_message_formatting);
    RUN_TEST(test_printf_formatting);
    RUN_TEST(test_buffer_overflow_protection);
    RUN_TEST(test_convenience_macros);
    RUN_TEST(test_file_logging_creation);
    RUN_TEST(test_directory_creation);
    RUN_TEST(test_log_rotation);
    RUN_TEST(test_thread_safety_basic);
    RUN_TEST(test_logging_init);
    
    TEST_REPORT();
    
    return (tests_run != tests_passed);
}

