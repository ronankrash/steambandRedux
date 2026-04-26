/* File: controller_menu.h */
#ifndef INCLUDED_CONTROLLER_MENU_H
#define INCLUDED_CONTROLLER_MENU_H

/*
 * Controller command menu system
 * Provides grid-based menu for accessing game commands via controller
 */

/*
 * Initialize controller menu system
 */
void controller_menu_init(void);

/*
 * Check if menu should be displayed and handle menu navigation
 * Returns TRUE if menu is active and input was handled
 */
int controller_menu_check(void);

/*
 * Show the controller command menu
 */
void controller_menu_show(void);

/*
 * Hide the controller command menu
 */
void controller_menu_hide(void);

/*
 * Check if menu is currently active
 */
int controller_menu_is_active(void);

/*
 * Test/support helpers for validating command-grid coverage without requiring
 * controller hardware.
 */
int controller_menu_get_command_count(void);
int controller_menu_has_command_key(int key_code);
const char *controller_menu_get_command_category(int index);
const char *controller_menu_get_command_name(int index);
void controller_menu_format_command_label(int index, char *buf, int buf_size);
int controller_menu_layout_fits_width(int term_width);

#endif /* INCLUDED_CONTROLLER_MENU_H */

