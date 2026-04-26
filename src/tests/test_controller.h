/* File: src/tests/test_controller.h */
#ifndef TEST_CONTROLLER_H
#define TEST_CONTROLLER_H

/* Forward declarations for controller tests */
void test_controller_default_mappings_accessible(void);
void test_controller_button_display_names(void);
void test_controller_mapping_key_code_get_set(void);
void test_controller_config_trailing_whitespace(void);
void test_controller_menu_init(void);
void test_controller_menu_show_hide(void);
void test_controller_config_menu_show_hide(void);
void test_controller_menu_mutual_exclusivity(void);
void test_controller_command_menu_core_fp_coverage(void);
void test_controller_mapping_count_consistency(void);
void test_controller_invalid_mapping_index(void);
void test_controller_back_single_delays_map_until_gesture_window(void);
void test_controller_back_double_opens_command_without_map(void);
void test_controller_back_triple_opens_config_immediately(void);
void test_controller_first_person_camera_relative_movement(void);

#endif /* TEST_CONTROLLER_H */

