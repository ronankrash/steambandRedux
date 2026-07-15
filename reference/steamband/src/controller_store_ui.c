/* File: controller_store_ui.c */
#include "controller_store_ui.h"
#include "controller_followup.h"
#include "controller_panel.h"
#include "controller.h"
#include "angband.h"
#include <stdio.h>
#include <string.h>

#define CONTROLLER_STORE_SLOTS 12

typedef enum {
    CONTROLLER_STORE_IDLE = 0,
    CONTROLLER_STORE_BROWSE,
    CONTROLLER_STORE_ACTION_PICK
} controller_store_mode_t;

typedef enum {
    CONTROLLER_STORE_VIEW_STOCK = 0,
    CONTROLLER_STORE_VIEW_SELL
} controller_store_view_t;

typedef struct {
    int index;
    char label;
    char desc[80];
    int display_row;
    byte color;
    int max_qty;
} controller_store_slot_t;

typedef struct {
    const char *label;
    int action;
} controller_store_action_t;

#define CONTROLLER_STORE_ACTIONS 6

static controller_store_mode_t g_store_mode = CONTROLLER_STORE_IDLE;
static controller_store_view_t g_store_view = CONTROLLER_STORE_VIEW_STOCK;
static int g_store_slot_count = 0;
static int g_store_cursor = 0;
static int g_store_action_count = 0;
static int g_store_action_cursor = 0;
static DWORD g_store_last_nav = 0;
static WORD g_store_prev_buttons = 0;
static char g_store_pending_label = 0;
static controller_store_slot_t g_store_slots[CONTROLLER_STORE_SLOTS];
static controller_store_action_t g_store_actions[CONTROLLER_STORE_ACTIONS];

enum {
    STORE_UI_ACT_PURCHASE = 1,
    STORE_UI_ACT_EXAMINE = 2,
    STORE_UI_ACT_SELL = 3,
    STORE_UI_ACT_NEXT_PAGE = 4,
    STORE_UI_ACT_PREV_PAGE = 5,
    STORE_UI_ACT_LEAVE = 6
};

static void controller_store_ui_clear_slots(void) {
    memset(g_store_slots, 0, sizeof(g_store_slots));
    g_store_slot_count = 0;
    g_store_cursor = 0;
}

static void controller_store_ui_clear_actions(void) {
    memset(g_store_actions, 0, sizeof(g_store_actions));
    g_store_action_count = 0;
    g_store_action_cursor = 0;
}

static void controller_store_ui_close(void) {
    g_store_mode = CONTROLLER_STORE_IDLE;
    g_store_prev_buttons = 0;
    controller_store_ui_clear_slots();
    controller_store_ui_clear_actions();
    prt("", 0, CONTROLLER_PANEL_X);
}

static void controller_store_ui_add_action(const char *label, int action) {
    if (g_store_action_count >= CONTROLLER_STORE_ACTIONS) return;
    g_store_actions[g_store_action_count].label = label;
    g_store_actions[g_store_action_count].action = action;
    g_store_action_count++;
}

static void controller_store_ui_build_stock_slots(void) {
    int i, k, end;
    object_type *o_ptr;
    char o_name[80];

    controller_store_ui_clear_slots();
    end = store_get_page_top() + 12;
    if (end > store_get_stock_count()) end = store_get_stock_count();

    for (i = store_get_page_top(), k = 0; i < end; i++) {
        o_ptr = store_get_stock_item(i);
        if (!o_ptr || !o_ptr->k_idx) continue;
        if (k >= CONTROLLER_STORE_SLOTS) break;

        object_desc(o_name, o_ptr, TRUE, 3);
        o_name[72] = '\0';
        g_store_slots[k].index = i;
        g_store_slots[k].label = store_get_item_label(i);
        g_store_slots[k].display_row = (i % 12) + 6;
        g_store_slots[k].color = tval_to_attr[o_ptr->tval & 0x7F];
        g_store_slots[k].max_qty = o_ptr->number;
        strcpy(g_store_slots[k].desc, o_name);
        k++;
    }
    g_store_slot_count = k;
}

static void controller_store_ui_build_sell_slots(void) {
    int i, k;
    object_type *o_ptr;
    char o_name[80];

    controller_store_ui_clear_slots();

    for (i = 0, k = 0; i < INVEN_PACK; i++) {
        o_ptr = &inventory[i];
        if (!o_ptr->k_idx) continue;
        if (!store_item_can_sell(o_ptr)) continue;
        if (k >= CONTROLLER_STORE_SLOTS) break;

        object_desc(o_name, o_ptr, TRUE, 3);
        o_name[72] = '\0';
        g_store_slots[k].index = i;
        g_store_slots[k].label = index_to_label(i);
        g_store_slots[k].display_row = k + 1;
        g_store_slots[k].color = tval_to_attr[o_ptr->tval & 0x7F];
        g_store_slots[k].max_qty = o_ptr->number;
        strcpy(g_store_slots[k].desc, o_name);
        k++;
    }
    g_store_slot_count = k;
}

static void controller_store_ui_build_slots(void) {
    if (g_store_view == CONTROLLER_STORE_VIEW_SELL) {
        controller_store_ui_build_sell_slots();
    } else {
        controller_store_ui_build_stock_slots();
    }
}

static void controller_store_ui_draw_help(void) {
    if (g_store_mode == CONTROLLER_STORE_ACTION_PICK) {
        prt("Shop action: Up/Down A=Do B=Back", 0, CONTROLLER_PANEL_X);
    } else if (g_store_view == CONTROLLER_STORE_VIEW_SELL) {
        prt("Sell list: Up/Down A=Select LB=Shop B=Leave", 0, CONTROLLER_PANEL_X);
    } else {
        prt("Shop stock: Up/Down A=Select LB=Sell B=Leave", 0, CONTROLLER_PANEL_X);
    }
}

static void controller_store_ui_highlight_row(int slot_index, bool selected) {
    controller_store_slot_t *slot;
    char tmp[80];
    byte attr;

    if (slot_index < 0 || slot_index >= g_store_slot_count) return;

    slot = &g_store_slots[slot_index];
    attr = selected ? TERM_YELLOW : slot->color;

    if (g_store_view == CONTROLLER_STORE_VIEW_STOCK) {
        sprintf(tmp, "%c) ", slot->label);
        prt(tmp, slot->display_row, 0);
        c_put_str(attr, slot->desc, slot->display_row, 3);
    } else {
        sprintf(tmp, "%c)", slot->label);
        put_str(tmp, slot->display_row, 29);
        c_put_str(attr, slot->desc, slot->display_row, 32);
    }
}

static void controller_store_ui_draw_action_menu(void) {
    int i;
    char line[40];

    prt("-- Actions --", 2, CONTROLLER_ACTION_X);
    for (i = 0; i < g_store_action_count; i++) {
        snprintf(line, sizeof(line), "%s%s",
                 (i == g_store_action_cursor) ? "> " : "  ",
                 g_store_actions[i].label);
        line[sizeof(line) - 1] = '\0';
        c_put_str((i == g_store_action_cursor) ? TERM_L_BLUE : TERM_WHITE,
                  line, i + 3, CONTROLLER_ACTION_X);
    }
}

static void controller_store_ui_redraw(void) {
    int i;

    controller_store_ui_build_slots();

    for (i = 0; i < g_store_slot_count; i++) {
        controller_store_ui_highlight_row(i, i == g_store_cursor);
    }

    if (g_store_mode == CONTROLLER_STORE_ACTION_PICK) {
        controller_store_ui_draw_action_menu();
    }

    controller_store_ui_draw_help();
    Term_fresh();
}

static void controller_store_ui_build_actions(void) {
    controller_store_ui_clear_actions();

    if (g_store_view == CONTROLLER_STORE_VIEW_STOCK) {
        controller_store_ui_add_action("Purchase", STORE_UI_ACT_PURCHASE);
        controller_store_ui_add_action("Examine", STORE_UI_ACT_EXAMINE);
        if (store_get_stock_count() > 12 && store_get_page_top() == 0) {
            controller_store_ui_add_action("Next page", STORE_UI_ACT_NEXT_PAGE);
        }
        if (store_get_stock_count() > 12 && store_get_page_top() > 0) {
            controller_store_ui_add_action("Prev page", STORE_UI_ACT_PREV_PAGE);
        }
    } else {
        controller_store_ui_add_action("Sell", STORE_UI_ACT_SELL);
        controller_store_ui_add_action("Examine", STORE_UI_ACT_EXAMINE);
    }
}

static void controller_store_ui_on_quantity_selected(int qty) {
    char label = g_store_pending_label;
    g_store_pending_label = 0;
    p_ptr->command_arg = qty;
    if (g_store_view == CONTROLLER_STORE_VIEW_SELL) {
        Term_keypress('d');
    } else {
        Term_keypress('g');
    }
    Term_keypress(label);
    controller_store_ui_redraw();
}

static void controller_store_ui_run_action(int action) {
    controller_store_slot_t *slot;

    if (g_store_cursor < 0 || g_store_cursor >= g_store_slot_count) return;

    slot = &g_store_slots[g_store_cursor];

    switch (action) {
        case STORE_UI_ACT_LEAVE:
            controller_store_ui_close();
            Term_keypress(ESCAPE);
            return;

        case STORE_UI_ACT_NEXT_PAGE:
            Term_keypress(' ');
            controller_store_ui_redraw();
            return;

        case STORE_UI_ACT_PREV_PAGE:
            Term_keypress(' ');
            controller_store_ui_redraw();
            return;

        case STORE_UI_ACT_EXAMINE:
            if (g_store_view == CONTROLLER_STORE_VIEW_SELL) {
                p_ptr->command_wrk = USE_INVEN;
                p_ptr->command_see = TRUE;
                Term_keypress('I');
                Term_keypress(slot->label);
            } else {
                Term_keypress('l');
                Term_keypress(slot->label);
            }
            controller_store_ui_redraw();
            return;

        case STORE_UI_ACT_PURCHASE:
        case STORE_UI_ACT_SELL:
            g_store_pending_label = slot->label;
            g_store_mode = CONTROLLER_STORE_BROWSE;
            controller_store_ui_clear_actions();
            controller_followup_show_quantity_menu(slot->max_qty,
                controller_store_ui_on_quantity_selected);
            return;
    }
}

static bool controller_store_ui_button_edge(XINPUT_STATE *state, WORD button) {
    bool now = (state->Gamepad.wButtons & button) != 0;
    bool prev = (g_store_prev_buttons & button) != 0;
    return now && !prev;
}

static bool controller_store_ui_nav(int delta, DWORD now) {
    if (now - g_store_last_nav < 180) return FALSE;

    if (g_store_mode == CONTROLLER_STORE_ACTION_PICK) {
        if (g_store_action_count <= 0) return FALSE;
        g_store_action_cursor += delta;
        if (g_store_action_cursor < 0) g_store_action_cursor = 0;
        if (g_store_action_cursor >= g_store_action_count) {
            g_store_action_cursor = g_store_action_count - 1;
        }
    } else {
        if (g_store_slot_count <= 0) return FALSE;
        g_store_cursor += delta;
        if (g_store_cursor < 0) g_store_cursor = 0;
        if (g_store_cursor >= g_store_slot_count) {
            g_store_cursor = g_store_slot_count - 1;
        }
    }

    g_store_last_nav = now;
    controller_store_ui_redraw();
    return TRUE;
}

static void controller_store_ui_handle_nav(XINPUT_STATE *state, DWORD now) {
    int stick = controller_thumbstick_vertical_nav_delta(state->Gamepad.sThumbLY);

    if (controller_store_ui_button_edge(state, XINPUT_GAMEPAD_DPAD_UP) || stick < 0) {
        controller_store_ui_nav(-1, now);
    } else if (controller_store_ui_button_edge(state, XINPUT_GAMEPAD_DPAD_DOWN) || stick > 0) {
        controller_store_ui_nav(1, now);
    }
}

static void controller_store_ui_enter_action_menu(void) {
    controller_store_ui_build_actions();
    if (g_store_action_count == 0) {
        bell("No actions for that item.");
        return;
    }
    g_store_action_cursor = 0;
    g_store_mode = CONTROLLER_STORE_ACTION_PICK;
    controller_store_ui_redraw();
}

static void controller_store_ui_handle_actions(XINPUT_STATE *state) {
    if (controller_followup_overlay_active()) return;

    if (controller_store_ui_button_edge(state, XINPUT_GAMEPAD_LEFT_SHOULDER)) {
        g_store_view = (g_store_view == CONTROLLER_STORE_VIEW_STOCK) ?
            CONTROLLER_STORE_VIEW_SELL : CONTROLLER_STORE_VIEW_STOCK;
        g_store_cursor = 0;
        g_store_mode = CONTROLLER_STORE_BROWSE;
        controller_store_ui_clear_actions();
        controller_store_ui_redraw();
        return;
    }

    if (controller_store_ui_button_edge(state, XINPUT_GAMEPAD_A)) {
        if (g_store_mode == CONTROLLER_STORE_BROWSE) {
            if (g_store_slot_count > 0) {
                controller_store_ui_enter_action_menu();
            }
        } else if (g_store_action_cursor >= 0 &&
                   g_store_action_cursor < g_store_action_count) {
            controller_store_ui_run_action(
                g_store_actions[g_store_action_cursor].action);
        }
        return;
    }

    if (controller_store_ui_button_edge(state, XINPUT_GAMEPAD_B)) {
        if (g_store_mode == CONTROLLER_STORE_ACTION_PICK) {
            g_store_mode = CONTROLLER_STORE_BROWSE;
            controller_store_ui_clear_actions();
            controller_store_ui_redraw();
        } else {
            controller_store_ui_run_action(STORE_UI_ACT_LEAVE);
        }
        return;
    }
}

void controller_store_ui_on_shop_enter(void) {
    if (!controller_is_connected()) return;
    if (!store_is_shopping()) return;

    g_store_view = CONTROLLER_STORE_VIEW_STOCK;
    g_store_mode = CONTROLLER_STORE_BROWSE;
    g_store_cursor = 0;
    g_store_prev_buttons = 0;
    controller_store_ui_redraw();
}

void controller_store_ui_on_shop_leave(void) {
    controller_store_ui_close();
    controller_followup_reset();
}

int controller_store_ui_is_active(void) {
    return g_store_mode != CONTROLLER_STORE_IDLE ? TRUE : FALSE;
}

void controller_store_ui_refresh(void) {
    if (g_store_mode != CONTROLLER_STORE_IDLE) {
        controller_store_ui_redraw();
    }
}

int controller_store_ui_poll(XINPUT_STATE *state, DWORD now) {
    if (!state || !store_is_shopping()) {
        controller_store_ui_close();
        return FALSE;
    }

    if (controller_followup_overlay_active()) {
        controller_followup_poll(state, now);
        controller_absorb_mapped_button_states(state);
        return TRUE;
    }

    if (g_store_mode == CONTROLLER_STORE_IDLE) {
        controller_store_ui_on_shop_enter();
        if (g_store_mode == CONTROLLER_STORE_IDLE) return FALSE;
    }

    controller_store_ui_handle_nav(state, now);
    controller_store_ui_handle_actions(state);
    g_store_prev_buttons = state->Gamepad.wButtons;

    return TRUE;
}
