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

int controller_is_connected(void);

#define CONTROLLER_BACK_ACTION_NONE    0
#define CONTROLLER_BACK_ACTION_MAP     1
#define CONTROLLER_BACK_ACTION_COMMAND 2
#define CONTROLLER_BACK_ACTION_CONFIG  3
#define CONTROLLER_PLAYABILITY_HINT_LINES 3

/*
 * Short launch-screen hints for controller-first playability.
 * first_person_available should be TRUE only when the SDL first-person
 * renderer is present in the current build.
 */
const char* controller_get_playability_hint(int line, int first_person_available);

/*
 * Update the Back-button gesture state machine.
 * Single Back resolves to map after the double/triple window expires;
 * double Back resolves to command menu; triple Back resolves to config.
 */
int controller_back_gesture_update(int back_pressed, DWORD now);

/*
 * Reset Back-button gesture state, primarily for initialization and tests.
 */
void controller_back_gesture_reset(void);

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

/*
 * Get normalized right-stick horizontal look value (-1.0 to 1.0), or 0.0
 * inside deadzone / when unavailable.
 */
double controller_get_look_x(void);

/*
 * Consume a simultaneous left-stick + right-stick click chord for toggling
 * the SDL first-person prototype. Returns TRUE exactly once per chord press.
 */
int controller_consume_first_person_toggle(void);

/*
 * Consume B/cancel as a first-person exit action. Returns TRUE once per press
 * and does not fire while controller menus are active.
 */
int controller_consume_first_person_cancel(void);

/*
 * Feed first-person camera state to controller movement transforms. When
 * active, D-pad/left-stick movement becomes camera-relative for the SDL view.
 */
void controller_set_first_person_camera(int active, double dir_x, double dir_y,
                                        double plane_x, double plane_y);
int controller_transform_movement_key(int key_code);
int controller_thumbstick_to_movement_key(short lx, short ly);
int controller_thumbstick_vertical_nav_delta(short ly);

/*
 * Mark mapped buttons that are currently held so a later controller_check()
 * pass does not treat them as a fresh press (used by overlay UIs).
 */
void controller_absorb_mapped_button_states(XINPUT_STATE *state);

#endif /* INCLUDED_CONTROLLER_H */

