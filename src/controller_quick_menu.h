/* File: controller_quick_menu.h */
#ifndef INCLUDED_CONTROLLER_QUICK_MENU_H
#define INCLUDED_CONTROLLER_QUICK_MENU_H

#ifdef WINDOWS
#include <windows.h>
#include <xinput.h>
#endif

/*
 * Compact controller menu for common in-game actions (fire, zap, search, etc.).
 * Opened with START while playing.
 */
void controller_quick_menu_init(void);
void controller_quick_menu_show(void);
void controller_quick_menu_hide(void);
int controller_quick_menu_is_active(void);
int controller_quick_menu_check(void);
int controller_quick_menu_get_command_count(void);

#endif /* INCLUDED_CONTROLLER_QUICK_MENU_H */
