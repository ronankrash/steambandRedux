/* File: src/tests/unity_test_runner.c
 * Unity test runner - runs Unity framework tests
 * 
 * This file provides a main() function for running Unity tests.
 * It can be compiled as a separate executable or integrated into main_test.c
 */

#include "unity.h"

/* Forward declarations for Unity infrastructure tests */
extern void test_unity_assertions(void);
extern void test_unity_runner(void);
extern void test_unity_suite_organization(void);
extern void test_test_helpers_temp_file(void);
extern void test_unity_fixtures_available(void);

/* Forward declarations for logging tests */
extern void test_log_level_enum(void);
extern void test_log_level_filtering(void);
extern void test_message_formatting(void);
extern void test_printf_formatting(void);
extern void test_buffer_overflow_protection(void);
extern void test_convenience_macros(void);
extern void test_file_logging_creation(void);
extern void test_directory_creation(void);
extern void test_log_rotation(void);
extern void test_thread_safety_basic(void);
extern void test_logging_init(void);
extern void test_log_level_get_set(void);
extern void test_null_filename_handling(void);

/* Forward declarations for z-util.c tests */
extern void test_streq_basic(void);
extern void test_streq_null_handling(void);
extern void test_prefix_basic(void);
extern void test_prefix_edge_cases(void);
extern void test_suffix_basic(void);
extern void test_suffix_edge_cases(void);
extern void test_my_strcpy_basic(void);
extern void test_my_strcpy_buffer_overflow(void);
extern void test_my_strcpy_zero_buffer(void);
extern void test_my_strcpy_exact_buffer_size(void);

/* Forward declarations for controller tests */
extern void test_controller_default_mappings_accessible(void);
extern void test_controller_button_display_names(void);
extern void test_controller_mapping_key_code_get_set(void);
extern void test_controller_config_trailing_whitespace(void);
extern void test_controller_menu_init(void);
extern void test_controller_menu_show_hide(void);
extern void test_controller_config_menu_show_hide(void);
extern void test_controller_menu_mutual_exclusivity(void);
extern void test_controller_mapping_count_consistency(void);
extern void test_controller_invalid_mapping_index(void);

int main(void) {
    UNITY_BEGIN();
    
    /* Run Unity infrastructure tests */
    RUN_TEST(test_unity_assertions);
    RUN_TEST(test_unity_runner);
    RUN_TEST(test_unity_suite_organization);
    RUN_TEST(test_test_helpers_temp_file);
    RUN_TEST(test_unity_fixtures_available);
    
    /* Run logging system tests */
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
    RUN_TEST(test_log_level_get_set);
    RUN_TEST(test_null_filename_handling);
    
    /* Run z-util.c tests */
    RUN_TEST(test_streq_basic);
    RUN_TEST(test_streq_null_handling);
    RUN_TEST(test_prefix_basic);
    RUN_TEST(test_prefix_edge_cases);
    RUN_TEST(test_suffix_basic);
    RUN_TEST(test_suffix_edge_cases);
    RUN_TEST(test_my_strcpy_basic);
    RUN_TEST(test_my_strcpy_buffer_overflow);
    RUN_TEST(test_my_strcpy_zero_buffer);
    RUN_TEST(test_my_strcpy_exact_buffer_size);
    
    /* Run controller tests */
    RUN_TEST(test_controller_default_mappings_accessible);
    RUN_TEST(test_controller_button_display_names);
    RUN_TEST(test_controller_mapping_key_code_get_set);
    RUN_TEST(test_controller_config_trailing_whitespace);
    RUN_TEST(test_controller_menu_init);
    RUN_TEST(test_controller_menu_show_hide);
    RUN_TEST(test_controller_config_menu_show_hide);
    RUN_TEST(test_controller_menu_mutual_exclusivity);
    RUN_TEST(test_controller_mapping_count_consistency);
    RUN_TEST(test_controller_invalid_mapping_index);
    
    return UNITY_END();
}

