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

#endif /* INCLUDED_CONTROLLER_MENU_H */

