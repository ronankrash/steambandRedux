/* File: controller_followup.h */
#ifndef INCLUDED_CONTROLLER_FOLLOWUP_H
#define INCLUDED_CONTROLLER_FOLLOWUP_H

#ifdef WINDOWS
#include <windows.h>
#include <xinput.h>
#endif

/*
 * Secondary controller overlays: rest duration, quantities, and in-target
 * key injection while legacy command prompts are active.
 */
void controller_followup_reset(void);
int controller_followup_is_active(void);
int controller_followup_overlay_active(void);

void controller_followup_show_rest_menu(void);
void controller_followup_show_quantity_menu(int max_qty,
    void (*on_selected)(int qty));

void controller_followup_begin_target_assist(void);
void controller_followup_end_target_assist(void);

int controller_followup_poll(XINPUT_STATE *state, DWORD now);

#endif /* INCLUDED_CONTROLLER_FOLLOWUP_H */
