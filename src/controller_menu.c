/* File: controller_menu.c */
#include "controller_menu.h"
#include "controller_config_menu.h"
#include "angband.h"
#include "logging.h"
#include "controller.h"
#ifdef STEAMBAND_HAS_SDL2
#include "renderer.h"
#endif
#include <windows.h>
#include <xinput.h>
#include <stdio.h>
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
    { "Search Mode", 'S', "Actions" },
    { "Rest", 'R', "Actions" },
    { "Look", 'l', "Actions" },
    { "Target", '*', "Actions" },
    { "Fire", 'f', "Actions" },
    { "Aim Ray", 'a', "Actions" },
    { "Throw", 'v', "Actions" },

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

    /* System / title-screen bridge commands */
    { "Options", '=', "System" },
    { "Save Game", KTRL('S'), "System" },
    { "New Game", 'N', "System" },
    { "Load Game", 'O', "System" },

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
 * Grid layout constants. Keep the command menu inside the legacy 80-column
 * terminal so handheld players do not lose the right-most column.
 */
#define MENU_COLS 3
#define MENU_ROWS 9
#define MENU_START_X 4
#define MENU_START_Y 6
#define MENU_COL_WIDTH 25
#define MENU_ITEM_WIDTH 23
#define MENU_CLEAR_Y 2
#define MENU_CLEAR_WIDTH 78
#define MENU_CLEAR_HEIGHT (MENU_ROWS + 8)

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

static const char *menu_category_short_name(const char *category) {
    if (!category) return "Cmd";
    if (strcmp(category, "Inventory") == 0) return "Inv";
    if (strcmp(category, "Actions") == 0) return "Act";
    if (strcmp(category, "Magic") == 0) return "Mag";
    if (strcmp(category, "Movement") == 0) return "Move";
    if (strcmp(category, "Objects") == 0) return "Obj";
    if (strcmp(category, "Traps") == 0) return "Door";
    if (strcmp(category, "Info") == 0) return "Info";
    if (strcmp(category, "System") == 0) return "Sys";
    return category;
}

void controller_menu_format_command_label(int index, char *buf, int buf_size) {
    if (!buf || buf_size <= 0) return;

    if (index < 0 || index >= menu_item_count()) {
        buf[0] = '\0';
        return;
    }

    snprintf(buf, buf_size, "%s: %s",
             menu_category_short_name(g_menu_items[index].category),
             g_menu_items[index].name);
    buf[buf_size - 1] = '\0';
}

int controller_menu_layout_fits_width(int term_width) {
    if (term_width <= 0) return FALSE;
    return (MENU_START_X + ((MENU_COLS - 1) * MENU_COL_WIDTH) + MENU_ITEM_WIDTH) <= term_width;
}

static void menu_update_first_person_hint(void) {
#ifdef STEAMBAND_HAS_SDL2
    char hint[80];
    if (!g_menu_active || !g_menu_items[g_menu_selected].name) {
        renderer_set_overlay_message(NULL);
        return;
    }
    snprintf(hint, sizeof(hint), "Command: %s / %s (A select, B back)",
             g_menu_items[g_menu_selected].category,
             g_menu_items[g_menu_selected].name);
    hint[sizeof(hint) - 1] = '\0';
    renderer_set_overlay_message(hint);
#endif
}

int controller_menu_get_command_count(void) {
    return menu_item_count();
}

int controller_menu_has_command_key(int key_code) {
    int i;

    for (i = 0; g_menu_items[i].name != NULL; i++) {
        if (g_menu_items[i].key_code == key_code) return TRUE;
    }

    return FALSE;
}

const char *controller_menu_get_command_category(int index) {
    if (index < 0 || index >= menu_item_count()) return "";
    return g_menu_items[index].category;
}

const char *controller_menu_get_command_name(int index) {
    if (index < 0 || index >= menu_item_count()) return "";
    return g_menu_items[index].name;
}

static void menu_clear_area(void) {
    int i;

    for (i = 0; i < MENU_CLEAR_HEIGHT; i++) {
        Term_erase(MENU_START_X - 2, MENU_CLEAR_Y + i, MENU_CLEAR_WIDTH);
    }
}

/*
 * Display menu on screen
 */
static void menu_display(void) {
    int i, x, y, col, row;
    int count = menu_item_count();
    int page_size = MENU_ROWS * MENU_COLS;
    int start_idx = (g_menu_selected / page_size) * page_size;
    int end_idx = start_idx + (MENU_ROWS * MENU_COLS);
    char line[64];
    if (end_idx > count) end_idx = count;

    menu_clear_area();

    /* Draw menu title */
    Term_putstr(MENU_START_X, MENU_CLEAR_Y, 40, TERM_WHITE, "Controller Command Menu");
    Term_putstr(MENU_START_X, MENU_CLEAR_Y + 1, 62, TERM_YELLOW,
                "D-Pad: Navigate  A: Select  B/Back: Close");
    snprintf(line, sizeof(line), "Selected: %s / %s",
             g_menu_items[g_menu_selected].category,
             g_menu_items[g_menu_selected].name);
    line[sizeof(line) - 1] = '\0';
    Term_putstr(MENU_START_X, MENU_CLEAR_Y + 2, 70, TERM_L_BLUE, line);
    Term_putstr(MENU_START_X, MENU_CLEAR_Y + 3, 68, TERM_WHITE,
                "Double Back: commands  Triple Back: button config  ?: help");

    /* Draw menu items in grid */
    for (i = start_idx; i < end_idx && i < count; i++) {
        col = (i - start_idx) % MENU_COLS;
        row = (i - start_idx) / MENU_COLS;
        x = MENU_START_X + (col * MENU_COL_WIDTH);
        y = MENU_START_Y + row;

        /* Highlight selected item */
        byte attr = (i == g_menu_selected) ? TERM_L_BLUE : TERM_WHITE;

        /* Draw menu item */
        controller_menu_format_command_label(i, line, sizeof(line));
        Term_putstr(x, y, MENU_ITEM_WIDTH, attr, line);
    }

    /* Draw selection indicator */
    if (g_menu_selected < count) {
        col = (g_menu_selected - start_idx) % MENU_COLS;
        row = (g_menu_selected - start_idx) / MENU_COLS;
        x = MENU_START_X + (col * MENU_COL_WIDTH) - 2;
        y = MENU_START_Y + row;
        Term_putstr(x, y, 1, TERM_YELLOW, ">");
    }

    snprintf(line, sizeof(line), "Page %d/%d",
             (start_idx / page_size) + 1,
             ((count - 1) / page_size) + 1);
    line[sizeof(line) - 1] = '\0';
    Term_putstr(MENU_START_X, MENU_START_Y + MENU_ROWS + 1, 20, TERM_SLATE, line);

    menu_update_first_person_hint();
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
    if (controller_config_menu_is_active()) {
        controller_config_menu_hide();
    }
    g_menu_active = TRUE;
    g_menu_selected = 0;
    menu_display();
}

/*
 * Hide the controller command menu
 */
void controller_menu_hide(void) {
    if (g_menu_active) {
        menu_clear_area();
    }
    g_menu_active = FALSE;
    menu_update_first_person_hint();
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

    /* B or BACK cancels */
    if (state->Gamepad.wButtons & (XINPUT_GAMEPAD_B | XINPUT_GAMEPAD_BACK)) {
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

