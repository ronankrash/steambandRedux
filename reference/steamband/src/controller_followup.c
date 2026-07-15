/* File: controller_followup.c */
#include "controller_followup.h"
#include "controller_panel.h"
#include "controller.h"
#include "angband.h"
#include <stdio.h>
#include <string.h>

typedef enum {
    CONTROLLER_FOLLOWUP_IDLE = 0,
    CONTROLLER_FOLLOWUP_TARGET,
    CONTROLLER_FOLLOWUP_REST_MENU,
    CONTROLLER_FOLLOWUP_QUANTITY_MENU
} controller_followup_mode_t;

typedef struct {
    const char *label;
    int command_arg;
} controller_followup_option_t;

typedef void (*controller_followup_quantity_cb)(int qty);

#define CONTROLLER_FOLLOWUP_OPTIONS 8

static controller_followup_mode_t g_followup_mode = CONTROLLER_FOLLOWUP_IDLE;
static int g_followup_cursor = 0;
static int g_followup_option_count = 0;
static DWORD g_followup_last_nav = 0;
static WORD g_followup_prev_buttons = 0;
static controller_followup_option_t g_followup_options[CONTROLLER_FOLLOWUP_OPTIONS];

static int g_followup_qty_max = 1;
static controller_followup_quantity_cb g_followup_qty_cb = NULL;

static void controller_followup_clear_options(void) {
    memset(g_followup_options, 0, sizeof(g_followup_options));
    g_followup_option_count = 0;
    g_followup_cursor = 0;
}

static void controller_followup_add_option(const char *label, int command_arg) {
    if (g_followup_option_count >= CONTROLLER_FOLLOWUP_OPTIONS) return;
    g_followup_options[g_followup_option_count].label = label;
    g_followup_options[g_followup_option_count].command_arg = command_arg;
    g_followup_option_count++;
}

static bool controller_followup_button_edge(XINPUT_STATE *state, WORD button) {
    bool now = (state->Gamepad.wButtons & button) != 0;
    bool prev = (g_followup_prev_buttons & button) != 0;
    return now && !prev;
}

static void controller_followup_draw_menu(const char *title, const char *help) {
    int i;
    char line[40];

    prt(title, 0, CONTROLLER_PANEL_X);
    if (help) prt(help, 1, CONTROLLER_PANEL_X);

    for (i = 0; i < g_followup_option_count; i++) {
        snprintf(line, sizeof(line), "%s%s", (i == g_followup_cursor) ? "> " : "  ",
                 g_followup_options[i].label);
        line[sizeof(line) - 1] = '\0';
        c_put_str((i == g_followup_cursor) ? TERM_L_BLUE : TERM_WHITE,
                  line, i + 3, CONTROLLER_ACTION_X);
    }

    Term_fresh();
}

static void controller_followup_clear_panel(void) {
    int i;
    for (i = 0; i < CONTROLLER_FOLLOWUP_OPTIONS + 4; i++) {
        prt("", i, CONTROLLER_PANEL_X);
    }
}

static bool controller_followup_nav(int delta, DWORD now) {
    if (now - g_followup_last_nav < 180) return FALSE;
    if (g_followup_option_count <= 0) return FALSE;

    g_followup_cursor += delta;
    if (g_followup_cursor < 0) g_followup_cursor = 0;
    if (g_followup_cursor >= g_followup_option_count) {
        g_followup_cursor = g_followup_option_count - 1;
    }

    g_followup_last_nav = now;
    return TRUE;
}

static void controller_followup_handle_menu_nav(XINPUT_STATE *state, DWORD now) {
    int stick = controller_thumbstick_vertical_nav_delta(state->Gamepad.sThumbLY);
    bool moved = FALSE;

    if (controller_followup_button_edge(state, XINPUT_GAMEPAD_DPAD_UP) || stick < 0) {
        moved = controller_followup_nav(-1, now);
    } else if (controller_followup_button_edge(state, XINPUT_GAMEPAD_DPAD_DOWN) || stick > 0) {
        moved = controller_followup_nav(1, now);
    }

    if (moved) {
        if (g_followup_mode == CONTROLLER_FOLLOWUP_REST_MENU) {
            controller_followup_draw_menu("-- Rest --", "A=Choose B=Cancel");
        } else if (g_followup_mode == CONTROLLER_FOLLOWUP_QUANTITY_MENU) {
            controller_followup_draw_menu("-- Quantity --", "A=Choose B=Back");
        }
    }
}

static void controller_followup_end_menu(void) {
    controller_followup_clear_panel();
    g_followup_mode = CONTROLLER_FOLLOWUP_IDLE;
    g_followup_qty_cb = NULL;
    g_followup_prev_buttons = 0;
    controller_followup_clear_options();
}

void controller_followup_reset(void) {
    controller_followup_end_menu();
    controller_followup_end_target_assist();
}

int controller_followup_is_active(void) {
    return g_followup_mode != CONTROLLER_FOLLOWUP_IDLE ? TRUE : FALSE;
}

int controller_followup_overlay_active(void) {
    return g_followup_mode == CONTROLLER_FOLLOWUP_REST_MENU ||
           g_followup_mode == CONTROLLER_FOLLOWUP_QUANTITY_MENU ? TRUE : FALSE;
}

void controller_followup_show_rest_menu(void) {
    controller_followup_reset();
    g_followup_mode = CONTROLLER_FOLLOWUP_REST_MENU;
    g_followup_prev_buttons = 0;

    controller_followup_add_option("Until done (&)", -2);
    controller_followup_add_option("Until HP/SP (*)", -1);
    controller_followup_add_option("10 turns", 10);
    controller_followup_add_option("50 turns", 50);
    controller_followup_add_option("100 turns", 100);
    controller_followup_add_option("500 turns", 500);

    controller_followup_draw_menu("-- Rest --", "A=Choose B=Cancel");
}

void controller_followup_show_quantity_menu(int max_qty,
    void (*on_selected)(int qty)) {
    controller_followup_reset();
    if (!on_selected || max_qty <= 0) return;

    g_followup_qty_max = max_qty;
    g_followup_qty_cb = on_selected;
    g_followup_mode = CONTROLLER_FOLLOWUP_QUANTITY_MENU;
    g_followup_prev_buttons = 0;

    controller_followup_add_option("1", 1);
    if (max_qty >= 5) controller_followup_add_option("5", 5);
    if (max_qty >= 10) controller_followup_add_option("10", 10);
    if (max_qty > 1) controller_followup_add_option("Max", max_qty);

    controller_followup_draw_menu("-- Quantity --", "A=Choose B=Back");
}

void controller_followup_begin_target_assist(void) {
    g_followup_mode = CONTROLLER_FOLLOWUP_TARGET;
    g_followup_prev_buttons = 0;
    prt("Target: D-pad=move A=Target B=Cancel", 0, CONTROLLER_PANEL_X);
    Term_fresh();
}

void controller_followup_end_target_assist(void) {
    if (g_followup_mode == CONTROLLER_FOLLOWUP_TARGET) {
        prt("", 0, CONTROLLER_PANEL_X);
        g_followup_mode = CONTROLLER_FOLLOWUP_IDLE;
        g_followup_prev_buttons = 0;
    }
}

static void controller_followup_poll_target(XINPUT_STATE *state, DWORD now) {
    short lx = state->Gamepad.sThumbLX;
    short ly = state->Gamepad.sThumbLY;
    int key = controller_thumbstick_to_movement_key(lx, ly);
    static DWORD last_move = 0;

    if (controller_followup_button_edge(state, XINPUT_GAMEPAD_DPAD_UP)) {
        Term_keypress('8');
    } else if (controller_followup_button_edge(state, XINPUT_GAMEPAD_DPAD_DOWN)) {
        Term_keypress('2');
    } else if (controller_followup_button_edge(state, XINPUT_GAMEPAD_DPAD_LEFT)) {
        Term_keypress('4');
    } else if (controller_followup_button_edge(state, XINPUT_GAMEPAD_DPAD_RIGHT)) {
        Term_keypress('6');
    } else if (key != 0) {
        if (now - last_move > 150) {
            Term_keypress(key);
            last_move = now;
        }
    }

    if (controller_followup_button_edge(state, XINPUT_GAMEPAD_A)) {
        Term_keypress('t');
        controller_followup_end_target_assist();
        controller_absorb_mapped_button_states(state);
        return;
    }

    if (controller_followup_button_edge(state, XINPUT_GAMEPAD_RIGHT_SHOULDER)) {
        Term_keypress('+');
    }

    if (controller_followup_button_edge(state, XINPUT_GAMEPAD_LEFT_SHOULDER)) {
        Term_keypress('-');
    }

    if (controller_followup_button_edge(state, XINPUT_GAMEPAD_B)) {
        Term_keypress('q');
        controller_followup_end_target_assist();
        controller_absorb_mapped_button_states(state);
    }
}

static void controller_followup_poll_menu(XINPUT_STATE *state, DWORD now) {
    controller_followup_handle_menu_nav(state, now);

    if (controller_followup_button_edge(state, XINPUT_GAMEPAD_A)) {
        int arg = g_followup_options[g_followup_cursor].command_arg;

        if (g_followup_mode == CONTROLLER_FOLLOWUP_REST_MENU) {
            p_ptr->command_arg = arg;
            controller_followup_end_menu();
            Term_keypress('R');
            controller_absorb_mapped_button_states(state);
            return;
        }

        if (g_followup_mode == CONTROLLER_FOLLOWUP_QUANTITY_MENU && g_followup_qty_cb) {
            controller_followup_quantity_cb cb = g_followup_qty_cb;
            controller_followup_end_menu();
            cb(arg);
            controller_absorb_mapped_button_states(state);
            return;
        }
    }

    if (controller_followup_button_edge(state, XINPUT_GAMEPAD_B)) {
        controller_followup_end_menu();
        controller_absorb_mapped_button_states(state);
    }
}

int controller_followup_poll(XINPUT_STATE *state, DWORD now) {
    if (!state) return FALSE;

    if (g_followup_mode == CONTROLLER_FOLLOWUP_TARGET) {
        controller_followup_poll_target(state, now);
        g_followup_prev_buttons = state->Gamepad.wButtons;
        return TRUE;
    }

    if (g_followup_mode == CONTROLLER_FOLLOWUP_REST_MENU ||
        g_followup_mode == CONTROLLER_FOLLOWUP_QUANTITY_MENU) {
        controller_followup_poll_menu(state, now);
        g_followup_prev_buttons = state->Gamepad.wButtons;
        return TRUE;
    }

    return FALSE;
}
