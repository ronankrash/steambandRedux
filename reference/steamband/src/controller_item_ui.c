/* File: controller_item_ui.c */
#include "controller_item_ui.h"
#include "controller_quick_menu.h"
#include "controller_panel.h"
#include "controller.h"
#include "angband.h"
#include "logging.h"
#include <string.h>

#define CONTROLLER_ITEM_UI_SLOTS 32
#define CONTROLLER_ITEM_UI_NAV_MS 180
#define CONTROLLER_ITEM_UI_ACTIONS 8
#define CONTROLLER_ITEM_UI_ACTION_COL CONTROLLER_ACTION_X

typedef enum {
    CONTROLLER_ITEM_UI_IDLE = 0,
    CONTROLLER_ITEM_UI_BROWSE,
    CONTROLLER_ITEM_UI_ACTION_PICK
} controller_item_ui_mode_t;

typedef struct {
    int slot_index;
    char label;
    char desc[80];
    int display_row;
    int desc_col;
    byte color;
} controller_item_slot_t;

typedef struct {
    const char *label;
    int key_code;
} controller_item_action_t;

static controller_item_ui_mode_t g_item_ui_mode = CONTROLLER_ITEM_UI_IDLE;
static int g_item_ui_work = 0;
static int g_item_ui_slot_count = 0;
static int g_item_ui_cursor = 0;
static int g_item_ui_list_col = 0;
static int g_item_ui_action_count = 0;
static int g_item_ui_action_cursor = 0;
static DWORD g_item_ui_last_nav = 0;
static WORD g_item_ui_prev_buttons = 0;
static controller_item_slot_t g_item_ui_slots[CONTROLLER_ITEM_UI_SLOTS];
static controller_item_action_t g_item_ui_actions[CONTROLLER_ITEM_UI_ACTIONS];

static void controller_item_ui_clear_slots(void) {
    memset(g_item_ui_slots, 0, sizeof(g_item_ui_slots));
    g_item_ui_slot_count = 0;
    g_item_ui_cursor = 0;
    g_item_ui_list_col = 0;
}

static void controller_item_ui_clear_actions(void) {
    memset(g_item_ui_actions, 0, sizeof(g_item_ui_actions));
    g_item_ui_action_count = 0;
    g_item_ui_action_cursor = 0;
}

static void controller_item_ui_close(void) {
    if (g_item_ui_mode != CONTROLLER_ITEM_UI_IDLE) {
        screen_load();
    }
    g_item_ui_mode = CONTROLLER_ITEM_UI_IDLE;
    g_item_ui_work = 0;
    g_item_ui_prev_buttons = 0;
    controller_item_ui_clear_slots();
    controller_item_ui_clear_actions();
}

static void controller_item_ui_add_action(const char *label, int key_code) {
    if (g_item_ui_action_count >= CONTROLLER_ITEM_UI_ACTIONS) return;
    g_item_ui_actions[g_item_ui_action_count].label = label;
    g_item_ui_actions[g_item_ui_action_count].key_code = key_code;
    g_item_ui_action_count++;
}

static void controller_item_ui_build_actions(void) {
    object_type *o_ptr;
    int slot_index;
    int tval;

    controller_item_ui_clear_actions();

    if (g_item_ui_cursor < 0 || g_item_ui_cursor >= g_item_ui_slot_count) return;

    slot_index = g_item_ui_slots[g_item_ui_cursor].slot_index;
    o_ptr = &inventory[slot_index];
    tval = o_ptr->tval;

    if (g_item_ui_work == USE_INVEN) {
        if (tval == TV_FOOD) controller_item_ui_add_action("Eat", 'E');
        if (tval == TV_FLASK || tval == TV_TONIC) controller_item_ui_add_action("Quaff", 'q');
        if (tval == TV_TOOL) controller_item_ui_add_action("Use", 'u');
        if (wield_slot(o_ptr) >= INVEN_WIELD) controller_item_ui_add_action("Wear", 'w');
        controller_item_ui_add_action("Drop", 'd');
        controller_item_ui_add_action("Destroy", 'k');
        controller_item_ui_add_action("Examine", 'I');
    } else {
        controller_item_ui_add_action("Take off", 't');
        if (tval == TV_TOOL) controller_item_ui_add_action("Use", 'u');
        if (tval == TV_FLASK || tval == TV_TONIC) controller_item_ui_add_action("Quaff", 'q');
        controller_item_ui_add_action("Drop", 'd');
        controller_item_ui_add_action("Examine", 'I');
    }
}

static int controller_item_ui_measure_column(int use_mode) {
    int i, z = 0, l;
    int len = 79 - 50;
    int lim = 79 - 3;
    object_type *o_ptr;
    char o_name[80];

    if (show_weights) lim -= 9;
    if (use_mode == USE_EQUIP && show_labels) lim -= (14 + 2);

    item_tester_full = TRUE;

    if (use_mode == USE_INVEN) {
        for (i = 0; i < INVEN_PACK; i++) {
            if (inventory[i].k_idx) z = i + 1;
        }
        for (i = 0; i < z; i++) {
            o_ptr = &inventory[i];
            if (!o_ptr->k_idx) continue;
            if (!item_tester_okay(o_ptr)) continue;

            object_desc(o_name, o_ptr, TRUE, 3);
            o_name[lim] = '\0';
            l = (int)strlen(o_name) + 5;
            if (show_weights) l += 9;
            if (l > len) len = l;
        }
    } else if (use_mode == USE_EQUIP) {
        for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {
            o_ptr = &inventory[i];
            if (!o_ptr->k_idx) continue;
            if (!item_tester_okay(o_ptr)) continue;

            object_desc(o_name, o_ptr, TRUE, 3);
            o_name[lim] = '\0';
            l = (int)strlen(o_name) + (2 + 3);
            if (show_labels) l += (14 + 2);
            if (show_weights) l += 9;
            if (l > len) len = l;
        }
    }

    item_tester_full = FALSE;

    return (len > 76) ? 0 : (79 - len);
}

static void controller_item_ui_build_slots(int use_mode) {
    int i, k, z = 0;
    int lim = 79 - 3;
    object_type *o_ptr;
    char o_name[80];
    int col;
    int desc_col;

    controller_item_ui_clear_slots();

    col = controller_item_ui_measure_column(use_mode);
    g_item_ui_list_col = col;
    desc_col = col + 3;
    if (use_mode == USE_EQUIP && show_labels) {
        desc_col = col + 3 + 14 + 2;
    }

    item_tester_full = TRUE;

    if (use_mode == USE_INVEN) {
        for (i = 0; i < INVEN_PACK; i++) {
            if (inventory[i].k_idx) z = i + 1;
        }
        for (i = 0; i < z; i++) {
            o_ptr = &inventory[i];
            if (!o_ptr->k_idx) continue;
            if (!item_tester_okay(o_ptr)) continue;
            if (g_item_ui_slot_count >= CONTROLLER_ITEM_UI_SLOTS) break;

            object_desc(o_name, o_ptr, TRUE, 3);
            o_name[lim] = '\0';
            k = g_item_ui_slot_count++;
            g_item_ui_slots[k].slot_index = i;
            g_item_ui_slots[k].label = index_to_label(i);
            g_item_ui_slots[k].display_row = k + 1;
            g_item_ui_slots[k].desc_col = desc_col;
            g_item_ui_slots[k].color = tval_to_attr[o_ptr->tval & 0x7F];
            strcpy(g_item_ui_slots[k].desc, o_name);
        }
    } else if (use_mode == USE_EQUIP) {
        for (i = INVEN_WIELD; i < INVEN_TOTAL; i++) {
            o_ptr = &inventory[i];
            if (!o_ptr->k_idx) continue;
            if (!item_tester_okay(o_ptr)) continue;
            if (g_item_ui_slot_count >= CONTROLLER_ITEM_UI_SLOTS) break;

            object_desc(o_name, o_ptr, TRUE, 3);
            o_name[lim] = '\0';
            k = g_item_ui_slot_count++;
            g_item_ui_slots[k].slot_index = i;
            g_item_ui_slots[k].label = index_to_label(i);
            g_item_ui_slots[k].display_row = k + 1;
            g_item_ui_slots[k].desc_col = desc_col;
            g_item_ui_slots[k].color = tval_to_attr[o_ptr->tval & 0x7F];
            strcpy(g_item_ui_slots[k].desc, o_name);
        }
    }

    item_tester_full = FALSE;
}

static void controller_item_ui_clear_action_column(void) {
    int i;
    for (i = 0; i < CONTROLLER_ITEM_UI_ACTIONS + 1; i++) {
        prt("", i, CONTROLLER_ITEM_UI_ACTION_COL);
    }
}

static void controller_item_ui_draw_help(void) {
    if (g_item_ui_mode == CONTROLLER_ITEM_UI_ACTION_PICK) {
        prt("Item actions: Up/Down  A=Do  B=Back", 0, 0);
    } else if (g_item_ui_work == USE_EQUIP) {
        prt("Equipment: Up/Down  A=Select  B=Close", 0, 0);
    } else {
        prt("Inventory: Up/Down  A=Select  B=Close", 0, 0);
    }
}

static void controller_item_ui_draw_row(int slot_index, bool selected) {
    controller_item_slot_t *slot;
    char tmp_val[80];
    byte attr;
    int col = g_item_ui_list_col;

    if (slot_index < 0 || slot_index >= g_item_ui_slot_count) return;

    slot = &g_item_ui_slots[slot_index];
    attr = selected ? TERM_YELLOW : slot->color;

    sprintf(tmp_val, "%c)", slot->label);
    put_str(tmp_val, slot->display_row, col);

    if (g_item_ui_work == USE_EQUIP && show_labels) {
        sprintf(tmp_val, "%-14s: ", mention_use(slot->slot_index));
        put_str(tmp_val, slot->display_row, col + 3);
    }

    c_put_str(attr, slot->desc, slot->display_row, slot->desc_col);
}

static void controller_item_ui_draw_action_menu(void) {
    int i;
    char line[32];

    controller_item_ui_clear_action_column();
    prt("-- Actions --", 1, CONTROLLER_ITEM_UI_ACTION_COL);

    for (i = 0; i < g_item_ui_action_count; i++) {
        snprintf(line, sizeof(line), "%s%s",
                 (i == g_item_ui_action_cursor) ? "> " : "  ",
                 g_item_ui_actions[i].label);
        line[sizeof(line) - 1] = '\0';
        c_put_str((i == g_item_ui_action_cursor) ? TERM_L_BLUE : TERM_WHITE,
                  line, i + 2, CONTROLLER_ITEM_UI_ACTION_COL);
    }
}

static void controller_item_ui_redraw(void) {
    int i;

    item_tester_full = TRUE;
    if (g_item_ui_work == USE_INVEN) {
        show_inven();
    } else if (g_item_ui_work == USE_EQUIP) {
        show_equip();
    }
    item_tester_full = FALSE;

    for (i = 0; i < g_item_ui_slot_count; i++) {
        controller_item_ui_draw_row(i, i == g_item_ui_cursor);
    }

    if (g_item_ui_mode == CONTROLLER_ITEM_UI_ACTION_PICK) {
        controller_item_ui_draw_action_menu();
    } else {
        controller_item_ui_clear_action_column();
    }

    controller_item_ui_draw_help();
    Term_fresh();
}

static void controller_item_ui_begin_browse(int use_mode) {
    if (controller_quick_menu_is_active()) {
        controller_quick_menu_hide();
    }
    g_item_ui_work = use_mode;
    screen_save();
    controller_item_ui_build_slots(use_mode);
    if (g_item_ui_slot_count == 0) {
        if (use_mode == USE_EQUIP) {
            msg_print("You are not wearing anything.");
        } else {
            msg_print("You are carrying nothing.");
        }
        screen_load();
        return;
    }
    g_item_ui_mode = CONTROLLER_ITEM_UI_BROWSE;
    controller_item_ui_redraw();
}

static void controller_item_ui_enter_action_menu(void) {
    controller_item_ui_build_actions();
    if (g_item_ui_action_count == 0) {
        bell("No actions for that item.");
        return;
    }
    g_item_ui_action_cursor = 0;
    g_item_ui_mode = CONTROLLER_ITEM_UI_ACTION_PICK;
    controller_item_ui_redraw();
}

static void controller_item_ui_execute_action(void) {
    char label;
    int action_key;
    int work;

    if (g_item_ui_action_cursor < 0 || g_item_ui_action_cursor >= g_item_ui_action_count) {
        bell("No action selected.");
        return;
    }

    if (g_item_ui_cursor < 0 || g_item_ui_cursor >= g_item_ui_slot_count) {
        bell("No item selected.");
        return;
    }

    label = g_item_ui_slots[g_item_ui_cursor].label;
    action_key = g_item_ui_actions[g_item_ui_action_cursor].key_code;
    work = g_item_ui_work;
    controller_item_ui_close();

    /* Tell get_item() which list the queued label refers to. */
    p_ptr->command_wrk = work;
    p_ptr->command_see = TRUE;

    Term_keypress(action_key);
    Term_keypress(label);
}

static bool controller_item_ui_button_edge(XINPUT_STATE *state, WORD button) {
    bool now = (state->Gamepad.wButtons & button) != 0;
    bool prev = (g_item_ui_prev_buttons & button) != 0;
    return now && !prev;
}

static bool controller_item_ui_nav_list(int delta, DWORD now) {
    if (now - g_item_ui_last_nav < CONTROLLER_ITEM_UI_NAV_MS) return FALSE;

    if (g_item_ui_mode == CONTROLLER_ITEM_UI_ACTION_PICK) {
        if (g_item_ui_action_count <= 0) return FALSE;
        g_item_ui_action_cursor += delta;
        if (g_item_ui_action_cursor < 0) g_item_ui_action_cursor = 0;
        if (g_item_ui_action_cursor >= g_item_ui_action_count) {
            g_item_ui_action_cursor = g_item_ui_action_count - 1;
        }
    } else {
        if (g_item_ui_slot_count <= 0) return FALSE;
        g_item_ui_cursor += delta;
        if (g_item_ui_cursor < 0) g_item_ui_cursor = 0;
        if (g_item_ui_cursor >= g_item_ui_slot_count) {
            g_item_ui_cursor = g_item_ui_slot_count - 1;
        }
    }

    g_item_ui_last_nav = now;
    controller_item_ui_redraw();
    return TRUE;
}

static void controller_item_ui_handle_nav(XINPUT_STATE *state, DWORD now) {
    short ly = state->Gamepad.sThumbLY;
    int stick = controller_thumbstick_vertical_nav_delta(ly);

    if (controller_item_ui_button_edge(state, XINPUT_GAMEPAD_DPAD_UP) || stick < 0) {
        controller_item_ui_nav_list(-1, now);
    } else if (controller_item_ui_button_edge(state, XINPUT_GAMEPAD_DPAD_DOWN) || stick > 0) {
        controller_item_ui_nav_list(1, now);
    }
}

static void controller_item_ui_handle_actions(XINPUT_STATE *state) {
    if (controller_item_ui_button_edge(state, XINPUT_GAMEPAD_A)) {
        if (g_item_ui_mode == CONTROLLER_ITEM_UI_BROWSE) {
            controller_item_ui_enter_action_menu();
        } else {
            controller_item_ui_execute_action();
        }
        return;
    }

    if (controller_item_ui_button_edge(state, XINPUT_GAMEPAD_B)) {
        if (g_item_ui_mode == CONTROLLER_ITEM_UI_ACTION_PICK) {
            g_item_ui_mode = CONTROLLER_ITEM_UI_BROWSE;
            controller_item_ui_clear_actions();
            controller_item_ui_redraw();
        } else {
            controller_item_ui_close();
        }
        return;
    }

    if (controller_item_ui_button_edge(state, XINPUT_GAMEPAD_BACK)) {
        controller_item_ui_close();
    }
}

void controller_item_ui_open_inventory(void) {
    if (!p_ptr || !character_generated || !game_in_progress) return;
    controller_item_ui_reset();
    controller_item_ui_begin_browse(USE_INVEN);
}

void controller_item_ui_open_equipment(void) {
    if (!p_ptr || !character_generated || !game_in_progress) return;
    controller_item_ui_reset();
    controller_item_ui_begin_browse(USE_EQUIP);
}

int controller_item_ui_is_active(void) {
    return g_item_ui_mode != CONTROLLER_ITEM_UI_IDLE ? TRUE : FALSE;
}

void controller_item_ui_reset(void) {
    controller_item_ui_close();
}

int controller_item_ui_poll(XINPUT_STATE *state, DWORD now) {
    if (!state || !p_ptr || !character_generated) return FALSE;
    if (g_item_ui_mode == CONTROLLER_ITEM_UI_IDLE) return FALSE;

    controller_item_ui_handle_nav(state, now);
    controller_item_ui_handle_actions(state);
    g_item_ui_prev_buttons = state->Gamepad.wButtons;

    return TRUE;
}
