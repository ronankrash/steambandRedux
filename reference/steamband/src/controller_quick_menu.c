/* File: controller_quick_menu.c */
#include "controller_quick_menu.h"
#include "controller_panel.h"
#include "controller_followup.h"
#include "controller.h"
#include "controller_menu.h"
#include "controller_item_ui.h"
#include "angband.h"
#include <windows.h>
#include <xinput.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    const char *name;
    int key_code;
} quick_menu_item_t;

static const quick_menu_item_t g_quick_items[] = {
    { "Fire", 'f' },
    { "Aim Ray", 'a' },
    { "Zap Apparatus", 'z' },
    { "Throw", 'v' },
    { "Search", 's' },
    { "Rest", 'R' },
    { "Look", 'l' },
    { "Target", '*' },
    { "Cast Spell", 'm' },
    { "Pray", 'p' },
    { NULL, 0 }
};

static bool g_quick_active = FALSE;
static int g_quick_selected = 0;
static DWORD g_quick_last_nav = 0;
static WORD g_quick_prev_buttons = 0;

static int quick_item_count(void) {
    int count = 0;
    while (g_quick_items[count].name != NULL) count++;
    return count;
}

static void quick_display(void) {
    int i, count = quick_item_count();
    char line[40];

    c_put_str(TERM_WHITE, "Quick Commands", 2, CONTROLLER_PANEL_X);
    c_put_str(TERM_YELLOW, "Up/Down A=Select B=Close", 3, CONTROLLER_PANEL_X);

    for (i = 0; i < count; i++) {
        byte attr = (i == g_quick_selected) ? TERM_L_BLUE : TERM_WHITE;
        snprintf(line, sizeof(line), "%s%s", (i == g_quick_selected) ? "> " : "  ",
                 g_quick_items[i].name);
        line[sizeof(line) - 1] = '\0';
        c_put_str(attr, line, i + 5, CONTROLLER_ACTION_X);
    }

    Term_fresh();
}

static void quick_execute_command(int key_code) {
    controller_quick_menu_hide();

    if (key_code == '*') {
        controller_followup_begin_target_assist();
        Term_keypress('*');
    } else if (key_code == 'l') {
        controller_followup_begin_target_assist();
        Term_keypress('l');
    } else if (key_code == 'R') {
        controller_followup_show_rest_menu();
    } else {
        Term_keypress(key_code);
    }
}

static bool quick_button_edge(XINPUT_STATE *state, WORD button) {
    bool now = (state->Gamepad.wButtons & button) != 0;
    bool prev = (g_quick_prev_buttons & button) != 0;
    return now && !prev;
}

static void quick_handle_nav(XINPUT_STATE *state, DWORD now) {
    int count = quick_item_count();
    int stick = controller_thumbstick_vertical_nav_delta(state->Gamepad.sThumbLY);

    if (now - g_quick_last_nav < 180) return;

    if (quick_button_edge(state, XINPUT_GAMEPAD_DPAD_UP) || stick < 0) {
        g_quick_selected--;
        if (g_quick_selected < 0) g_quick_selected = 0;
        g_quick_last_nav = now;
        quick_display();
    } else if (quick_button_edge(state, XINPUT_GAMEPAD_DPAD_DOWN) || stick > 0) {
        g_quick_selected++;
        if (g_quick_selected >= count) g_quick_selected = count - 1;
        g_quick_last_nav = now;
        quick_display();
    }
}

static void quick_handle_selection(XINPUT_STATE *state) {
    int count = quick_item_count();

    if (quick_button_edge(state, XINPUT_GAMEPAD_A)) {
        if (g_quick_selected >= 0 && g_quick_selected < count) {
            quick_execute_command(g_quick_items[g_quick_selected].key_code);
            controller_absorb_mapped_button_states(state);
        }
        return;
    }

    if (quick_button_edge(state, XINPUT_GAMEPAD_B) ||
        quick_button_edge(state, XINPUT_GAMEPAD_BACK)) {
        controller_quick_menu_hide();
        controller_absorb_mapped_button_states(state);
    }
}

void controller_quick_menu_init(void) {
    g_quick_active = FALSE;
    g_quick_selected = 0;
    g_quick_last_nav = 0;
    g_quick_prev_buttons = 0;
}

void controller_quick_menu_show(void) {
    if (!character_generated || !game_in_progress || !p_ptr) return;

    if (controller_item_ui_is_active()) {
        controller_item_ui_reset();
    }
    if (controller_menu_is_active()) {
        controller_menu_hide();
    }

    screen_save();
    g_quick_active = TRUE;
    g_quick_selected = 0;
    g_quick_prev_buttons = 0;
    quick_display();
}

void controller_quick_menu_hide(void) {
    if (g_quick_active) {
        screen_load();
    }
    g_quick_active = FALSE;
    g_quick_prev_buttons = 0;
}

int controller_quick_menu_is_active(void) {
    return g_quick_active ? TRUE : FALSE;
}

int controller_quick_menu_get_command_count(void) {
    return quick_item_count();
}

int controller_quick_menu_check(void) {
    XINPUT_STATE state;
    DWORD dwResult, now;

    if (!g_quick_active) return FALSE;

    dwResult = XInputGetState(0, &state);
    if (dwResult != ERROR_SUCCESS) return FALSE;

    now = GetTickCount();
    quick_handle_nav(&state, now);
    quick_handle_selection(&state);
    g_quick_prev_buttons = state.Gamepad.wButtons;

    return TRUE;
}
