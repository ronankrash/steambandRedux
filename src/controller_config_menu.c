/* File: controller_config_menu.c */
#include "controller_config_menu.h"
#include "controller.h"
#include "angband.h"
#include "logging.h"
#include <windows.h>
#include <xinput.h>
#include <string.h>
#include <stdio.h>

/*
 * Menu state
 */
static bool g_config_menu_active = FALSE;
static int g_config_menu_selected = 0;  /* Currently selected button index */
static DWORD g_config_menu_last_nav = 0; /* Last navigation time for rate limiting */
static bool g_config_menu_remapping = FALSE; /* True when waiting for button press to remap */
static int g_config_menu_remap_target = -1; /* Button index being remapped */

/* Use controller_get_button_display_name directly */

/*
 * Get key code display name
 */
static void get_key_display_name(int key_code, char *buf, int buf_size) {
    if (key_code >= 32 && key_code <= 126) {
        /* Printable ASCII character */
        snprintf(buf, buf_size, "'%c' (%d)", key_code, key_code);
    } else {
        /* Special key */
        switch (key_code) {
            case 13: strncpy(buf, "Enter", buf_size); break;
            case 27: strncpy(buf, "Escape", buf_size); break;
            default: snprintf(buf, buf_size, "Key %d", key_code); break;
        }
    }
}

/*
 * Display config menu
 */
static void config_menu_display(void) {
    int i, x, y;
    int menu_start_x = 5;
    int menu_start_y = 3;
    char key_buf[32];
    int count = controller_get_mapping_count();
    
    /* Draw menu title */
    Term_putstr(menu_start_x, menu_start_y - 1, 50, TERM_WHITE, "Controller Button Configuration");
    if (g_config_menu_remapping) {
        Term_putstr(menu_start_x, menu_start_y - 2, 50, TERM_YELLOW, "Press button to assign, or B to cancel");
    } else {
        Term_putstr(menu_start_x, menu_start_y - 2, 50, TERM_YELLOW, "A: Remap  B: Cancel/Save  D-Pad: Navigate");
    }
    
    /* Draw button mappings */
    for (i = 0; i < count; i++) {
        y = menu_start_y + i;
        x = menu_start_x;
        
        /* Highlight selected item */
        byte attr = (i == g_config_menu_selected) ? TERM_L_BLUE : TERM_WHITE;
        
        /* Draw button name */
        char line[80];
        WORD button = controller_get_mapping_button(i);
        int key_code = controller_get_mapping_key_code(i);
        const char *button_name = controller_get_button_display_name(button);
        get_key_display_name(key_code, key_buf, sizeof(key_buf));
        
        snprintf(line, sizeof(line), "%-15s -> %s", button_name, key_buf);
        Term_putstr(x, y, 50, attr, line);
        
        /* Draw selection indicator */
        if (i == g_config_menu_selected) {
            Term_putstr(x - 2, y, 1, TERM_YELLOW, ">");
        }
    }
    
    /* Draw save instruction */
    if (!g_config_menu_remapping) {
        Term_putstr(menu_start_x, menu_start_y + 12, 50, TERM_WHITE, "Press B to save and exit");
    }
}

/*
 * Initialize controller config menu system
 */
void controller_config_menu_init(void) {
    g_config_menu_active = FALSE;
    g_config_menu_selected = 0;
    g_config_menu_last_nav = 0;
    g_config_menu_remapping = FALSE;
    g_config_menu_remap_target = -1;
}

/*
 * Show the controller configuration menu
 */
void controller_config_menu_show(void) {
    g_config_menu_active = TRUE;
    g_config_menu_selected = 0;
    g_config_menu_remapping = FALSE;
    g_config_menu_remap_target = -1;
    config_menu_display();
}

/*
 * Hide the controller configuration menu
 */
void controller_config_menu_hide(void) {
    if (g_config_menu_active) {
        /* Clear menu area */
        int menu_start_x = 5;
        int menu_start_y = 1;
        int i;
        for (i = 0; i < 20; i++) {
            Term_erase(menu_start_x - 2, menu_start_y + i, 60);
        }
    }
    g_config_menu_active = FALSE;
    g_config_menu_remapping = FALSE;
    g_config_menu_remap_target = -1;
}

/*
 * Check if config menu is currently active
 */
int controller_config_menu_is_active(void) {
    return g_config_menu_active;
}

/*
 * Handle menu navigation
 */
static void config_menu_handle_navigation(XINPUT_STATE *state) {
    DWORD now = GetTickCount();
    int count = controller_get_mapping_count();
    
    /* Rate limit navigation */
    if (now - g_config_menu_last_nav < 200) {
        return;
    }
    
    /* Check D-Pad for navigation */
    if (state->Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) {
        g_config_menu_selected--;
        if (g_config_menu_selected < 0) g_config_menu_selected = 0;
        g_config_menu_last_nav = now;
        config_menu_display();
    } else if (state->Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) {
        g_config_menu_selected++;
        if (g_config_menu_selected >= count) g_config_menu_selected = count - 1;
        g_config_menu_last_nav = now;
        config_menu_display();
    }
}

/*
 * Handle menu selection and remapping
 */
static void config_menu_handle_selection(XINPUT_STATE *state) {
    int count = controller_get_mapping_count();
    int i;
    
    if (g_config_menu_remapping) {
        /* Waiting for button press to remap */
        /* Check all buttons to see which one was pressed */
        for (i = 0; i < count; i++) {
            WORD button = controller_get_mapping_button(i);
            if (state->Gamepad.wButtons & button) {
                /* Button pressed - use its key code for the target */
                if (g_config_menu_remap_target >= 0 && g_config_menu_remap_target < count) {
                    /* Don't allow remapping to the same button */
                    if (i != g_config_menu_remap_target) {
                        int key_code = controller_get_mapping_key_code(i);
                        controller_set_mapping_key_code(g_config_menu_remap_target, key_code);
                    }
                }
                g_config_menu_remapping = FALSE;
                g_config_menu_remap_target = -1;
                config_menu_display();
                return;
            }
        }
        
        /* B button cancels remapping */
        if (state->Gamepad.wButtons & XINPUT_GAMEPAD_B) {
            g_config_menu_remapping = FALSE;
            g_config_menu_remap_target = -1;
            config_menu_display();
        }
    } else {
        /* Normal menu mode */
        /* A button starts remapping */
        if (state->Gamepad.wButtons & XINPUT_GAMEPAD_A) {
            if (g_config_menu_selected < count) {
                g_config_menu_remapping = TRUE;
                g_config_menu_remap_target = g_config_menu_selected;
                config_menu_display();
            }
        }
        
        /* B button saves and exits */
        if (state->Gamepad.wButtons & XINPUT_GAMEPAD_B) {
            /* Save configuration */
            controller_save_config();
            /* Hide menu */
            controller_config_menu_hide();
        }
    }
}

/*
 * Check if config menu should be displayed and handle menu navigation
 * Returns TRUE if menu is active and input was handled
 */
int controller_config_menu_check(void) {
    XINPUT_STATE state;
    DWORD dwResult;
    
    if (!g_config_menu_active) {
        return FALSE;
    }
    
    /* Poll controller */
    dwResult = XInputGetState(0, &state);
    if (dwResult != ERROR_SUCCESS) {
        return FALSE;
    }
    
    /* Handle navigation */
    config_menu_handle_navigation(&state);
    
    /* Handle selection */
    config_menu_handle_selection(&state);
    
    return TRUE;
}

