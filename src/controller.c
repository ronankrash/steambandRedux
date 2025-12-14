/* File: controller.c */
#include "controller.h"
#include "controller_menu.h"
#include "controller_config_menu.h"
#include "angband.h"
#include "logging.h"
#include <windows.h>
#include <xinput.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>  /* For atan2() */
#include <ctype.h>  /* For isspace() */

/* Note: XInput library is linked via CMakeLists.txt, so pragma comment is not needed */

/*
 * Controller state
 */
static XINPUT_STATE g_state;
static bool g_connected = FALSE;
static bool g_previous_connected = FALSE; /* Track previous state for change detection */
static DWORD g_last_packet = 0;
static bool g_logging_enabled = TRUE; /* Controller logging enabled by default */
static DWORD g_back_button_press_time = 0; /* Track BACK button for menu activation */
static bool g_back_button_was_pressed = FALSE;

/*
 * Button mappings
 * We map Xbox buttons to Angband keys
 * 
 * Default button mapping design:
 * - Face buttons (A, B, X, Y): Primary actions and menu navigation
 * - D-Pad: Movement (with key repeat for smooth movement)
 * - Shoulder buttons (LB, RB): Secondary actions
 * - Start/Back: System functions (menu, map)
 * - Left thumbstick: 8-way diagonal movement (handled separately)
 * - Right thumbstick: Unmapped (available for future use)
 * - Triggers: Unmapped (available for future use)
 */
typedef struct {
    WORD button;
    int key_code;
    bool pressed;
    int repeat_delay;  /* Initial delay in ms before repeat starts (0 = no repeat) */
    int repeat_rate;   /* Delay in ms between repeats (0 = no repeat) */
    DWORD press_time;  /* Time when button was first pressed (for repeat delay) */
    DWORD last_repeat; /* Time of last repeat (for repeat rate) */
} button_map_t;

static button_map_t g_mapping[] = {
    /* Face Buttons - Primary Actions (no repeat - one-shot actions) */
    { XINPUT_GAMEPAD_A,          13,  FALSE, 0, 0, 0, 0 }, /* A -> Enter/Return (Confirm/Select in menus, also used for "use" actions) */
    { XINPUT_GAMEPAD_B,          27,  FALSE, 0, 0, 0, 0 }, /* B -> Escape (Cancel/Back in menus) */
    { XINPUT_GAMEPAD_X,          'i', FALSE, 0, 0, 0, 0 }, /* X -> Inventory list (very commonly used) */
    { XINPUT_GAMEPAD_Y,          'e', FALSE, 0, 0, 0, 0 }, /* Y -> Equipment list (very commonly used) */
    
    /* D-Pad - Movement (with key repeat for smooth movement) */
    { XINPUT_GAMEPAD_DPAD_UP,    '8', FALSE, 200, 50, 0, 0 }, /* Up - Movement north */
    { XINPUT_GAMEPAD_DPAD_DOWN,  '2', FALSE, 200, 50, 0, 0 }, /* Down - Movement south */
    { XINPUT_GAMEPAD_DPAD_LEFT,  '4', FALSE, 200, 50, 0, 0 }, /* Left - Movement west */
    { XINPUT_GAMEPAD_DPAD_RIGHT, '6', FALSE, 200, 50, 0, 0 }, /* Right - Movement east */
    
    /* System Buttons (no repeat - one-shot actions) */
    { XINPUT_GAMEPAD_START,      27,  FALSE, 0, 0, 0, 0 }, /* Start -> Escape (Main menu) */
    { XINPUT_GAMEPAD_BACK,       'M', FALSE, 0, 0, 0, 0 }, /* Back -> Full dungeon map */
    
    /* Shoulder Buttons - Secondary Actions (no repeat - one-shot actions) */
    { XINPUT_GAMEPAD_LEFT_SHOULDER,  'R', FALSE, 0, 0, 0, 0 }, /* LB -> Rest (common action) */
    { XINPUT_GAMEPAD_RIGHT_SHOULDER, 's', FALSE, 0, 0, 0, 0 }, /* RB -> Search for traps/doors (common action) */
    
    /* Terminator */
    { 0, 0, FALSE, 0, 0, 0, 0 }
};

/*
 * Initialize controller
 */
void controller_init(void) {
    DWORD dwResult;
    const char *env_log;
    
    ZeroMemory(&g_state, sizeof(XINPUT_STATE));
    
    /* Check environment variable for controller logging */
    env_log = getenv("STEAMBAND_CONTROLLER_LOG");
    if (env_log && strcmp(env_log, "0") == 0) {
        g_logging_enabled = FALSE;
    } else {
        g_logging_enabled = TRUE; /* Default to enabled */
    }
    
    /* Check if controller is connected */
    dwResult = XInputGetState(0, &g_state);
    g_connected = (dwResult == ERROR_SUCCESS);
    g_previous_connected = g_connected; /* Initialize previous state */
    
    /* Log initialization status */
    if (g_logging_enabled) {
        if (g_connected) {
            LOG_I("Controller initialized: Xbox 360 controller (port 0) connected");
        } else {
            LOG_I("Controller initialized: No controller detected (port 0)");
        }
    }
    
    /* Initialize menu systems */
    controller_menu_init();
    controller_config_menu_init();
}

/*
 * Handle stick movement for 8-way diagonal movement
 * Maps left thumbstick to numpad keys for full 8-way movement:
 *   7 8 9
 *   4   6
 *   1 2 3
 */
static void check_thumbsticks(void) {
    /* Deadzone thresholds */
    const int DEADZONE = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
    short lx = g_state.Gamepad.sThumbLX;
    short ly = g_state.Gamepad.sThumbLY;
    
    /* Check if outside deadzone (either axis) */
    if ((lx < -DEADZONE || lx > DEADZONE) || (ly < -DEADZONE || ly > DEADZONE)) {
        int key = 0;
        
        /* Calculate movement direction based on angle */
        /* Use atan2 to get angle in radians, then map to 8 directions */
        /* Note: XInput Y axis is inverted (positive Y is up, but we want positive Y = north = '8') */
        double angle = atan2(-(double)ly, (double)lx); /* Negate ly because Y is inverted */
        
        /* Normalize angle to 0-2*PI range */
        if (angle < 0) angle += 2.0 * 3.14159265358979323846;
        
        /* Divide circle into 8 sectors (45 degrees each = PI/4 radians) */
        /* Each sector maps to a numpad key */
        /* Sector boundaries: 0, PI/8, 3*PI/8, 5*PI/8, 7*PI/8, 9*PI/8, 11*PI/8, 13*PI/8, 15*PI/8 */
        /* Map to keys: 6 (right), 9 (up-right), 8 (up), 7 (up-left), 4 (left), 1 (down-left), 2 (down), 3 (down-right) */
        
        const double PI_8 = 3.14159265358979323846 / 8.0;
        
        if (angle < PI_8 || angle >= 15 * PI_8) {
            /* Right (0-22.5 degrees or 337.5-360 degrees) */
            key = '6';
        } else if (angle < 3 * PI_8) {
            /* Up-right (22.5-67.5 degrees) */
            key = '9';
        } else if (angle < 5 * PI_8) {
            /* Up (67.5-112.5 degrees) */
            key = '8';
        } else if (angle < 7 * PI_8) {
            /* Up-left (112.5-157.5 degrees) */
            key = '7';
        } else if (angle < 9 * PI_8) {
            /* Left (157.5-202.5 degrees) */
            key = '4';
        } else if (angle < 11 * PI_8) {
            /* Down-left (202.5-247.5 degrees) */
            key = '1';
        } else if (angle < 13 * PI_8) {
            /* Down (247.5-292.5 degrees) */
            key = '2';
        } else {
            /* Down-right (292.5-337.5 degrees) */
            key = '3';
        }
        
        /* Queue keypress if enough time passed since last move (rate limiting) */
        static DWORD last_move = 0;
        DWORD now = GetTickCount();
        if (now - last_move > 150) { /* 150ms delay prevents movement spam */
            Term_keypress(key);
            last_move = now;
        }
    }
}

/*
 * Check controller input
 */
int controller_check(void) {
    DWORD dwResult;
    XINPUT_STATE state;
    int i;
    bool handled = FALSE;
    DWORD now = GetTickCount();

    /* Poll controller 0 */
    dwResult = XInputGetState(0, &state);

    if (dwResult == ERROR_SUCCESS) {
        /* Controller is connected */
        g_connected = TRUE;
        
        /* Log connection event if state changed from disconnected to connected */
        if (!g_previous_connected && g_logging_enabled) {
            LOG_I("Controller connected: Xbox 360 controller (port 0)");
        }
        g_previous_connected = TRUE;
        
        /* Check packet number to see if state changed */
        if (state.dwPacketNumber != g_last_packet) {
            g_last_packet = state.dwPacketNumber;
            g_state = state;
        }
        
        /* Check if config menu is active - handle config menu input first */
        if (controller_config_menu_is_active()) {
            if (controller_config_menu_check()) {
                return TRUE; /* Config menu handled input */
            }
        }
        
        /* Check if command menu is active - handle menu input */
        if (controller_menu_is_active()) {
            if (controller_menu_check()) {
                return TRUE; /* Menu handled input */
            }
        }
        
        /* Check for config menu activation: BACK button triple-press (within 500ms each) */
        static DWORD g_back_press_times[3] = {0, 0, 0};
        static int g_back_press_count = 0;
        bool back_pressed = (state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) != 0;
        if (back_pressed && !g_back_button_was_pressed) {
            /* Back button just pressed */
            DWORD time_since_last = (g_back_press_count > 0) ? (now - g_back_press_times[g_back_press_count - 1]) : 1000;
            if (time_since_last < 500 && g_back_press_count > 0) {
                g_back_press_times[g_back_press_count++] = now;
                if (g_back_press_count >= 3) {
                    /* Triple-press detected - toggle config menu */
                    if (controller_config_menu_is_active()) {
                        controller_config_menu_hide();
                    } else {
                        controller_config_menu_show();
                    }
                    g_back_press_count = 0;
                    g_back_button_press_time = 0;
                }
            } else {
                /* Reset counter */
                g_back_press_count = 1;
                g_back_press_times[0] = now;
            }
            g_back_button_was_pressed = TRUE;
        } else if (!back_pressed && g_back_button_was_pressed) {
            /* Back button released */
            g_back_button_was_pressed = FALSE;
            /* Reset counter after delay */
            if (g_back_press_count > 0 && (now - g_back_press_times[g_back_press_count - 1]) > 500) {
                g_back_press_count = 0;
            }
        }
        
        /* Check for command menu activation: BACK button double-press (within 500ms) */
        if (!controller_config_menu_is_active()) {
            back_pressed = (state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) != 0;
            if (back_pressed && !g_back_button_was_pressed) {
                /* Back button just pressed */
                DWORD time_since_last = now - g_back_button_press_time;
                if (time_since_last < 500 && g_back_button_press_time > 0) {
                    /* Double-press detected - toggle command menu */
                    if (controller_menu_is_active()) {
                        controller_menu_hide();
                    } else {
                        controller_menu_show();
                    }
                    g_back_button_press_time = 0; /* Reset */
                } else {
                    g_back_button_press_time = now;
                }
                g_back_button_was_pressed = TRUE;
            } else if (!back_pressed && g_back_button_was_pressed) {
                /* Back button released */
                g_back_button_was_pressed = FALSE;
            }
        }
        
        /* If any menu is active, don't process normal button mappings */
        if (controller_menu_is_active() || controller_config_menu_is_active()) {
            return FALSE;
        }
        
        /* Check buttons */
        for (i = 0; g_mapping[i].button != 0; i++) {
            bool current_pressed = (state.Gamepad.wButtons & g_mapping[i].button) != 0;
            DWORD time_since_press = now - g_mapping[i].press_time;
            DWORD time_since_repeat = now - g_mapping[i].last_repeat;
            
            if (current_pressed && !g_mapping[i].pressed) {
                /* Button Down Event - Initial press */
                Term_keypress(g_mapping[i].key_code);
                g_mapping[i].pressed = TRUE;
                g_mapping[i].press_time = now;
                g_mapping[i].last_repeat = now; /* Initialize last_repeat for repeat timing */
                handled = TRUE;
            } else if (!current_pressed && g_mapping[i].pressed) {
                /* Button Up Event */
                g_mapping[i].pressed = FALSE;
                g_mapping[i].press_time = 0;
                g_mapping[i].last_repeat = 0;
            } else if (current_pressed && g_mapping[i].pressed && g_mapping[i].repeat_rate > 0) {
                /* Button is held down and repeat is enabled - check for repeat */
                /* First check if initial repeat delay has elapsed */
                if (time_since_press >= (DWORD)g_mapping[i].repeat_delay) {
                    /* Initial delay has passed, check if it's time for next repeat */
                    if (time_since_repeat >= (DWORD)g_mapping[i].repeat_rate) {
                        /* Time for another repeat */
                        Term_keypress(g_mapping[i].key_code);
                        g_mapping[i].last_repeat = now;
                        handled = TRUE;
                    }
                }
            }
        }
        
        /* Check thumbsticks */
        check_thumbsticks();
        
    } else {
        /* Controller is disconnected */
        g_connected = FALSE;
        
        /* Log disconnection event if state changed from connected to disconnected */
        if (g_previous_connected && g_logging_enabled) {
            if (dwResult == ERROR_DEVICE_NOT_CONNECTED) {
                LOG_W("Controller disconnected: Xbox 360 controller (port 0)");
            } else {
                LOG_W("Controller error: XInputGetState returned error code %lu", dwResult);
            }
        }
        g_previous_connected = FALSE;
    }

    return handled;
}

/*
 * Get button name from button code
 * Returns button name string for config file
 */
static const char* get_button_name_internal(WORD button) {
    switch (button) {
        case XINPUT_GAMEPAD_A: return "A";
        case XINPUT_GAMEPAD_B: return "B";
        case XINPUT_GAMEPAD_X: return "X";
        case XINPUT_GAMEPAD_Y: return "Y";
        case XINPUT_GAMEPAD_DPAD_UP: return "DPAD_UP";
        case XINPUT_GAMEPAD_DPAD_DOWN: return "DPAD_DOWN";
        case XINPUT_GAMEPAD_DPAD_LEFT: return "DPAD_LEFT";
        case XINPUT_GAMEPAD_DPAD_RIGHT: return "DPAD_RIGHT";
        case XINPUT_GAMEPAD_START: return "START";
        case XINPUT_GAMEPAD_BACK: return "BACK";
        case XINPUT_GAMEPAD_LEFT_SHOULDER: return "LB";
        case XINPUT_GAMEPAD_RIGHT_SHOULDER: return "RB";
        default: return NULL;
    }
}

/*
 * Get button code from button name
 * Returns button code, or 0 if not found
 */
static WORD get_button_code_internal(const char* name) {
    if (streq(name, "A")) return XINPUT_GAMEPAD_A;
    if (streq(name, "B")) return XINPUT_GAMEPAD_B;
    if (streq(name, "X")) return XINPUT_GAMEPAD_X;
    if (streq(name, "Y")) return XINPUT_GAMEPAD_Y;
    if (streq(name, "DPAD_UP")) return XINPUT_GAMEPAD_DPAD_UP;
    if (streq(name, "DPAD_DOWN")) return XINPUT_GAMEPAD_DPAD_DOWN;
    if (streq(name, "DPAD_LEFT")) return XINPUT_GAMEPAD_DPAD_LEFT;
    if (streq(name, "DPAD_RIGHT")) return XINPUT_GAMEPAD_DPAD_RIGHT;
    if (streq(name, "START")) return XINPUT_GAMEPAD_START;
    if (streq(name, "BACK")) return XINPUT_GAMEPAD_BACK;
    if (streq(name, "LB")) return XINPUT_GAMEPAD_LEFT_SHOULDER;
    if (streq(name, "RB")) return XINPUT_GAMEPAD_RIGHT_SHOULDER;
    return 0;
}

/*
 * Load controller button mappings from config file
 * Format: button_name:key_code:repeat_delay:repeat_rate
 * Comments start with #, blank lines are ignored
 */
void controller_load_config(void) {
    FILE *fp;
    char buf[1024];
    char fname[1024];
    int i;
    extern cptr ANGBAND_DIR_USER;  /* From variable.c */
    
    /* Build config file path */
    if (!ANGBAND_DIR_USER) {
        /* Paths not initialized yet, use defaults */
        if (g_logging_enabled) {
            LOG_I("Controller config: ANGBAND_DIR_USER not initialized, using default mappings");
        }
        return;
    }
    
    path_build(fname, sizeof(fname), ANGBAND_DIR_USER, "controller.prf");
    
    /* Open config file */
    fp = my_fopen(fname, "r");
    if (!fp) {
        /* Config file doesn't exist, use defaults */
        if (g_logging_enabled) {
            LOG_I("Controller config: Config file not found, using default mappings");
        }
        return;
    }
    
    /* Parse config file */
    while (0 == my_fgets(fp, buf, sizeof(buf))) {
        char *line = buf;
        char *button_name, *key_str, *delay_str, *rate_str;
        WORD button_code;
        int key_code, repeat_delay, repeat_rate;
        
        /* Skip leading whitespace */
        while (isspace(*line)) line++;
        
        /* Skip empty lines */
        if (!*line) continue;
        
        /* Skip comments */
        if (*line == '#') continue;
        
        /* Parse line: button_name:key_code:repeat_delay:repeat_rate */
        button_name = line;
        
        /* Find colon separators */
        key_str = strchr(line, ':');
        if (!key_str) continue;
        *key_str++ = '\0';
        
        delay_str = strchr(key_str, ':');
        if (!delay_str) continue;
        *delay_str++ = '\0';
        
        rate_str = strchr(delay_str, ':');
        if (!rate_str) continue;
        *rate_str++ = '\0';
        
        /* Trim whitespace */
        while (isspace(*button_name)) button_name++;
        while (isspace(*key_str)) key_str++;
        while (isspace(*delay_str)) delay_str++;
        while (isspace(*rate_str)) rate_str++;
        
        /* Get button code from name */
        button_code = get_button_code_internal(button_name);
        if (!button_code) continue;  /* Unknown button name */
        
        /* Parse values */
        key_code = atoi(key_str);
        repeat_delay = atoi(delay_str);
        repeat_rate = atoi(rate_str);
        
        /* Find matching button in mapping array and update it */
        for (i = 0; g_mapping[i].button != 0; i++) {
            if (g_mapping[i].button == button_code) {
                g_mapping[i].key_code = key_code;
                g_mapping[i].repeat_delay = repeat_delay;
                g_mapping[i].repeat_rate = repeat_rate;
                break;
            }
        }
    }
    
    /* Close file */
    my_fclose(fp);
    
    if (g_logging_enabled) {
        LOG_I("Controller config: Loaded mappings from %s", fname);
    }
}

/*
 * Save controller button mappings to config file
 * Format: button_name:key_code:repeat_delay:repeat_rate
 */
void controller_save_config(void) {
    FILE *fp;
    char fname[1024];
    int i;
    extern cptr ANGBAND_DIR_USER;  /* From variable.c */
    
    /* Build config file path */
    if (!ANGBAND_DIR_USER) {
        if (g_logging_enabled) {
            LOG_W("Controller config: ANGBAND_DIR_USER not initialized, cannot save config");
        }
        return;
    }
    
    path_build(fname, sizeof(fname), ANGBAND_DIR_USER, "controller.prf");
    
    /* Open config file for writing */
    fp = my_fopen(fname, "w");
    if (!fp) {
        if (g_logging_enabled) {
            LOG_W("Controller config: Failed to open %s for writing", fname);
        }
        return;
    }
    
    /* Write header comment */
    fprintf(fp, "# Controller button mapping configuration file\n");
    fprintf(fp, "# Format: button_name:key_code:repeat_delay:repeat_rate\n");
    fprintf(fp, "# Comments start with #\n");
    fprintf(fp, "# Blank lines are ignored\n");
    fprintf(fp, "# Key codes: ASCII character codes (e.g., 'i' = 105, 'e' = 101, 27 = Escape, 13 = Enter)\n");
    fprintf(fp, "# Repeat delay: Initial delay in ms before repeat starts (0 = no repeat)\n");
    fprintf(fp, "# Repeat rate: Delay in ms between repeats (0 = no repeat)\n");
    fprintf(fp, "\n");
    
    /* Write mappings */
    for (i = 0; g_mapping[i].button != 0; i++) {
        const char *button_name = get_button_name_internal(g_mapping[i].button);
        if (button_name) {
            fprintf(fp, "%s:%d:%d:%d\n", 
                    button_name, 
                    g_mapping[i].key_code,
                    g_mapping[i].repeat_delay,
                    g_mapping[i].repeat_rate);
        }
    }
    
    /* Close file */
    my_fclose(fp);
    
    if (g_logging_enabled) {
        LOG_I("Controller config: Saved mappings to %s", fname);
    }
}

/*
 * Get number of button mappings
 */
int controller_get_mapping_count(void) {
    int count = 0;
    while (g_mapping[count].button != 0) {
        count++;
    }
    return count;
}

/*
 * Get button mapping at index (for config menu)
 * Returns button code, or 0 if index is invalid
 */
WORD controller_get_mapping_button(int index) {
    int count = controller_get_mapping_count();
    if (index < 0 || index >= count) {
        return 0;
    }
    return g_mapping[index].button;
}

/*
 * Get key code for button mapping at index
 */
int controller_get_mapping_key_code(int index) {
    int count = controller_get_mapping_count();
    if (index < 0 || index >= count) {
        return 0;
    }
    return g_mapping[index].key_code;
}

/*
 * Set key code for button mapping at index
 */
void controller_set_mapping_key_code(int index, int key_code) {
    int count = controller_get_mapping_count();
    if (index < 0 || index >= count) {
        return;
    }
    g_mapping[index].key_code = key_code;
}

/*
 * Get button display name (exported version)
 */
const char* controller_get_button_display_name(WORD button) {
    switch (button) {
        case XINPUT_GAMEPAD_A: return "A Button";
        case XINPUT_GAMEPAD_B: return "B Button";
        case XINPUT_GAMEPAD_X: return "X Button";
        case XINPUT_GAMEPAD_Y: return "Y Button";
        case XINPUT_GAMEPAD_DPAD_UP: return "D-Pad Up";
        case XINPUT_GAMEPAD_DPAD_DOWN: return "D-Pad Down";
        case XINPUT_GAMEPAD_DPAD_LEFT: return "D-Pad Left";
        case XINPUT_GAMEPAD_DPAD_RIGHT: return "D-Pad Right";
        case XINPUT_GAMEPAD_START: return "Start";
        case XINPUT_GAMEPAD_BACK: return "Back";
        case XINPUT_GAMEPAD_LEFT_SHOULDER: return "Left Bumper";
        case XINPUT_GAMEPAD_RIGHT_SHOULDER: return "Right Bumper";
        default: return "Unknown";
    }
}
