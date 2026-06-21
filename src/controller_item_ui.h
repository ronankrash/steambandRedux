/* File: controller_item_ui.h */
#ifndef INCLUDED_CONTROLLER_ITEM_UI_H
#define INCLUDED_CONTROLLER_ITEM_UI_H

#ifdef WINDOWS
#include <windows.h>
#include <xinput.h>
#endif

/*
 * Controller-first inventory/equipment browser with D-pad selection.
 * Opens from the default X (inventory) and Y (equipment) mappings.
 */
void controller_item_ui_open_inventory(void);
void controller_item_ui_open_equipment(void);

/*
 * Returns TRUE while the overlay is active and consumed controller input.
 */
int controller_item_ui_poll(XINPUT_STATE *state, DWORD now);

/*
 * TRUE when movement keys should not be sent to the dungeon.
 */
int controller_item_ui_is_active(void);

void controller_item_ui_reset(void);

#endif /* INCLUDED_CONTROLLER_ITEM_UI_H */
