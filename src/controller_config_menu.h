/* File: controller_config_menu.h */
#ifndef INCLUDED_CONTROLLER_CONFIG_MENU_H
#define INCLUDED_CONTROLLER_CONFIG_MENU_H

/*
 * Controller button configuration menu
 * Allows in-game remapping of controller buttons
 */

/*
 * Initialize controller config menu system
 */
void controller_config_menu_init(void);

/*
 * Show the controller configuration menu
 */
void controller_config_menu_show(void);

/*
 * Hide the controller configuration menu
 */
void controller_config_menu_hide(void);

/*
 * Check if config menu is currently active
 */
int controller_config_menu_is_active(void);

/*
 * Check if config menu should be displayed and handle menu navigation
 * Returns TRUE if menu is active and input was handled
 */
int controller_config_menu_check(void);

#endif /* INCLUDED_CONTROLLER_CONFIG_MENU_H */

