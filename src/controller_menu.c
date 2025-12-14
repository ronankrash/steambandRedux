/* File: controller_menu.c */
#include "controller_menu.h"
#include "angband.h"
#include "logging.h"
#include "controller.h"
#include <windows.h>
#include <xinput.h>
#include <string.h>

/*
 * Menu item structure
 */
typedef struct {
    const char *name;      /* Display name */
    int key_code;          /* Key code to send when selected */
    const char *category;  /* Category name */
} menu_item_t;

/*
 * Menu categories and items
 * Organized by command categories from dungeon.c
 */
static const menu_item_t g_menu_items[] = {
    /* Inventory Commands */
    { "Wear/Wield", 'w', "Inventory" },
    { "Take Off", 't', "Inventory" },
    { "Drop Item", 'd', "Inventory" },
    { "Destroy Item", 'k', "Inventory" },
    { "Equipment", 'e', "Inventory" },
    { "Inventory", 'i', "Inventory" },
    
    /* Actions */
    { "Open Door", 'o', "Actions" },
    { "Close Door", 'c', "Actions" },
    { "Search", 's', "Actions" },
    { "Rest", 'R', "Actions" },
    { "Look", 'l', "Actions" },
    { "Fire/Throw", 'f', "Actions" },
    
    /* Magic */
    { "Cast Spell", 'm', "Magic" },
    { "Pray", 'p', "Magic" },
    { "Browse Book", 'b', "Magic" },
    { "Study", 'G', "Magic" },
    
    /* Movement */
    { "Run", '.', "Movement" },
    { "Walk", ';', "Movement" },
    { "Stay", 'g', "Movement" },
    { "Go Up", '<', "Movement" },
    { "Go Down", '>', "Movement" },
    
    /* Objects */
    { "Use Tool", 'u', "Objects" },
    { "Quaff Potion", 'q', "Objects" },
    { "Activate", 'A', "Objects" },
    { "Eat Food", 'E', "Objects" },
    
    /* Traps/Doors */
    { "Disarm Trap", 'D', "Traps" },
    { "Bash Door", 'B', "Traps" },
    { "Jam Door", 'j', "Traps" },
    
    /* Information */
    { "Map", 'M', "Info" },
    { "Locate", 'L', "Info" },
    { "Help", '?', "Info" },
    { "Character", 'C', "Info" },
    
    /* Terminator */
    { NULL, 0, NULL }
};

/*
 * Menu state
 */
static bool g_menu_active = FALSE;
static int g_menu_selected = 0;  /* Currently selected item index */
static int g_menu_category = 0;   /* Currently selected category (0 = all) */
static DWORD g_menu_last_nav = 0; /* Last navigation time for rate limiting */

/*
 * Grid layout constants
 */
#define MENU_COLS 4  /* Number of columns in grid */
#define MENU_ROWS 8  /* Number of rows visible at once */

/*
 * Get number of menu items
 */
static int menu_item_count(void) {
    int count = 0;
    while (g_menu_items[count].name != NULL) {
        count++;
    }
    return count;
}

/*
 * Display menu on screen
 */
static void menu_display(void) {
    int i, x, y, col, row;
    int count = menu_item_count();
    int start_idx = (g_menu_selected / MENU_COLS) * MENU_COLS; /* Start of visible row */
    int end_idx = start_idx + (MENU_ROWS * MENU_COLS);
    if (end_idx > count) end_idx = count;
    
    /* Clear menu area (assume menu starts at row 5, col 10) */
    int menu_start_x = 10;
    int menu_start_y = 5;
    
    /* Draw menu title */
    Term_putstr(menu_start_x, menu_start_y - 1, 30, TERM_WHITE, "Controller Command Menu");
    Term_putstr(menu_start_x, menu_start_y - 2, 30, TERM_YELLOW, "D-Pad: Navigate  A: Select  B: Cancel");
    
    /* Draw menu items in grid */
    for (i = start_idx; i < end_idx && i < count; i++) {
        col = (i - start_idx) % MENU_COLS;
        row = (i - start_idx) / MENU_COLS;
        x = menu_start_x + (col * 20);
        y = menu_start_y + row;
        
        /* Highlight selected item */
        byte attr = (i == g_menu_selected) ? TERM_L_BLUE : TERM_WHITE;
        
        /* Draw menu item */
        Term_putstr(x, y, 18, attr, g_menu_items[i].name);
    }
    
    /* Draw selection indicator */
    if (g_menu_selected < count) {
        col = (g_menu_selected - start_idx) % MENU_COLS;
        row = (g_menu_selected - start_idx) / MENU_COLS;
        x = menu_start_x + (col * 20) - 2;
        y = menu_start_y + row;
        Term_putstr(x, y, 1, TERM_YELLOW, ">");
    }
}

/*
 * Initialize controller menu system
 */
void controller_menu_init(void) {
    g_menu_active = FALSE;
    g_menu_selected = 0;
    g_menu_category = 0;
    g_menu_last_nav = 0;
}

/*
 * Show the controller command menu
 */
void controller_menu_show(void) {
    g_menu_active = TRUE;
    g_menu_selected = 0;
    menu_display();
}

/*
 * Hide the controller command menu
 */
void controller_menu_hide(void) {
    if (g_menu_active) {
        /* Clear menu area */
        int menu_start_x = 10;
        int menu_start_y = 3;
        int i;
        for (i = 0; i < MENU_ROWS + 3; i++) {
            Term_erase(menu_start_x - 2, menu_start_y + i, 80);
        }
    }
    g_menu_active = FALSE;
}

/*
 * Check if menu is currently active
 */
int controller_menu_is_active(void) {
    return g_menu_active;
}

/*
 * Handle menu navigation
 */
static void menu_handle_navigation(XINPUT_STATE *state) {
    DWORD now = GetTickCount();
    int count = menu_item_count();
    bool nav_pressed = FALSE;
    
    /* Rate limit navigation */
    if (now - g_menu_last_nav < 200) {
        return; /* Too soon since last navigation */
    }
    
    /* Check D-Pad for navigation */
    if (state->Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) {
        g_menu_selected -= MENU_COLS;
        if (g_menu_selected < 0) g_menu_selected = 0;
        nav_pressed = TRUE;
    } else if (state->Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) {
        g_menu_selected += MENU_COLS;
        if (g_menu_selected >= count) g_menu_selected = count - 1;
        nav_pressed = TRUE;
    } else if (state->Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) {
        g_menu_selected--;
        if (g_menu_selected < 0) g_menu_selected = 0;
        nav_pressed = TRUE;
    } else if (state->Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) {
        g_menu_selected++;
        if (g_menu_selected >= count) g_menu_selected = count - 1;
        nav_pressed = TRUE;
    }
    
    if (nav_pressed) {
        g_menu_last_nav = now;
        menu_display();
    }
}

/*
 * Handle menu selection
 */
static void menu_handle_selection(XINPUT_STATE *state) {
    int count = menu_item_count();
    
    /* A button selects */
    if (state->Gamepad.wButtons & XINPUT_GAMEPAD_A) {
        if (g_menu_selected < count) {
            /* Send keypress for selected command */
            Term_keypress(g_menu_items[g_menu_selected].key_code);
            /* Hide menu */
            controller_menu_hide();
        }
    }
    
    /* B button cancels */
    if (state->Gamepad.wButtons & XINPUT_GAMEPAD_B) {
        controller_menu_hide();
    }
}

/*
 * Check if menu should be displayed and handle menu navigation
 * Returns TRUE if menu is active and input was handled
 */
int controller_menu_check(void) {
    XINPUT_STATE state;
    DWORD dwResult;
    
    if (!g_menu_active) {
        return FALSE;
    }
    
    /* Poll controller */
    dwResult = XInputGetState(0, &state);
    if (dwResult != ERROR_SUCCESS) {
        return FALSE;
    }
    
    /* Handle navigation */
    menu_handle_navigation(&state);
    
    /* Handle selection */
    menu_handle_selection(&state);
    
    return TRUE;
}

