/* File: controller.c */
#include "controller.h"
#include "angband.h"
#include "logging.h"
#include <windows.h>
#include <xinput.h>
#include <stdlib.h>
#include <string.h>

/* Note: XInput library is linked via CMakeLists.txt, so pragma comment is not needed */

/*
 * Controller state
 */
static XINPUT_STATE g_state;
static bool g_connected = FALSE;
static bool g_previous_connected = FALSE; /* Track previous state for change detection */
static DWORD g_last_packet = 0;
static bool g_logging_enabled = TRUE; /* Controller logging enabled by default */

/*
 * Button mappings
 * We map Xbox buttons to Angband keys
 */
typedef struct {
    WORD button;
    int key_code;
    bool pressed;
    int repeat_delay;
    int repeat_rate;
} button_map_t;

static button_map_t g_mapping[] = {
    { XINPUT_GAMEPAD_A,          't', FALSE, 0, 0 }, /* A -> Take off? No, usually Confirm/Enter/Fire */
    { XINPUT_GAMEPAD_B,          'f', FALSE, 0, 0 }, /* B -> Fire/Throw? Or Cancel? */
    { XINPUT_GAMEPAD_X,          'w', FALSE, 0, 0 }, /* X -> Wear/Wield? */
    { XINPUT_GAMEPAD_Y,          'i', FALSE, 0, 0 }, /* Y -> Inventory? */
    { XINPUT_GAMEPAD_DPAD_UP,    '8', FALSE, 200, 50 }, /* Up */
    { XINPUT_GAMEPAD_DPAD_DOWN,  '2', FALSE, 200, 50 }, /* Down */
    { XINPUT_GAMEPAD_DPAD_LEFT,  '4', FALSE, 200, 50 }, /* Left */
    { XINPUT_GAMEPAD_DPAD_RIGHT, '6', FALSE, 200, 50 }, /* Right */
    { XINPUT_GAMEPAD_START,      27,  FALSE, 0, 0 }, /* Start -> ESC (Menu) */
    { XINPUT_GAMEPAD_BACK,       'M', FALSE, 0, 0 }, /* Back -> Map? */
    { XINPUT_GAMEPAD_LEFT_SHOULDER, 'R', FALSE, 0, 0 }, /* LB -> Rest? */
    { XINPUT_GAMEPAD_RIGHT_SHOULDER, 's', FALSE, 0, 0 }, /* RB -> Search? */
    { 0, 0, FALSE, 0, 0 }
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
}

/*
 * Handle stick movement for movement
 */
static void check_thumbsticks(void) {
    /* Deadzone thresholds */
    const int DEADZONE = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
    short lx = g_state.Gamepad.sThumbLX;
    short ly = g_state.Gamepad.sThumbLY;
    
    /* Check if outside deadzone */
    if ((lx < -DEADZONE || lx > DEADZONE) || (ly < -DEADZONE || ly > DEADZONE)) {
        /* Determine direction */
        /* Simple 8-way directional mapping */
        /* 
           7 8 9
           4   6
           1 2 3 
        */
        int key = 0;
        
        /* Calculate angle or dominant axis */
        if (abs(lx) > abs(ly)) {
            /* Horizontal dominant */
            if (lx > 0) key = '6';
            else key = '4';
        } else {
            /* Vertical dominant */
            if (ly > 0) key = '8';
            else key = '2';
        }
        
        /* Queue keypress if enough time passed since last move (to control speed) */
        /* For now, just queue it. The game loop might handle speed. */
        /* Actually, we need to rate limit this or it will spam movement */
        /* Hack: Use static timer */
        static DWORD last_move = 0;
        if (GetTickCount() - last_move > 150) { /* 150ms delay */
            Term_keypress(key);
            last_move = GetTickCount();
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
        
        /* Check buttons */
        for (i = 0; g_mapping[i].button != 0; i++) {
            bool current_pressed = (state.Gamepad.wButtons & g_mapping[i].button) != 0;
            
            if (current_pressed && !g_mapping[i].pressed) {
                /* Button Down Event */
                Term_keypress(g_mapping[i].key_code);
                g_mapping[i].pressed = TRUE;
                handled = TRUE;
            } else if (!current_pressed && g_mapping[i].pressed) {
                /* Button Up Event */
                g_mapping[i].pressed = FALSE;
            } else if (current_pressed && g_mapping[i].repeat_rate > 0) {
                /* Key repeat logic could go here */
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

