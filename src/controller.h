/* File: controller.h */
#ifndef INCLUDED_CONTROLLER_H
#define INCLUDED_CONTROLLER_H

#ifdef WINDOWS
#include <windows.h>
#include <xinput.h>
#endif

/*
 * Initialize controller support
 */
void controller_init(void);

/*
 * Check for controller input and queue keypresses
 * Returns TRUE if input was handled
 */
int controller_check(void);

/*
 * Load controller button mappings from config file
 * Called automatically after controller_init() if ANGBAND_DIR_USER is available
 */
void controller_load_config(void);

/*
 * Save controller button mappings to config file
 * Called when mappings are changed via in-game menu (future)
 */
void controller_save_config(void);

/*
 * Get number of button mappings
 */
int controller_get_mapping_count(void);

/*
 * Get button mapping at index (for config menu)
 * Returns button code, or 0 if index is invalid
 */
WORD controller_get_mapping_button(int index);

/*
 * Get key code for button mapping at index
 */
int controller_get_mapping_key_code(int index);

/*
 * Set key code for button mapping at index
 */
void controller_set_mapping_key_code(int index, int key_code);

/*
 * Get button display name
 */
const char* controller_get_button_display_name(WORD button);

#endif /* INCLUDED_CONTROLLER_H */

