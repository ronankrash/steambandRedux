/* File: controller_store_ui.h */
#ifndef INCLUDED_CONTROLLER_STORE_UI_H
#define INCLUDED_CONTROLLER_STORE_UI_H

#ifdef WINDOWS
#include <windows.h>
#include <xinput.h>
#endif

void controller_store_ui_on_shop_enter(void);
void controller_store_ui_on_shop_leave(void);
void controller_store_ui_refresh(void);
int controller_store_ui_is_active(void);
int controller_store_ui_poll(XINPUT_STATE *state, DWORD now);

#endif /* INCLUDED_CONTROLLER_STORE_UI_H */
