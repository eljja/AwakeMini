#ifndef AM_BLACKOUT_H
#define AM_BLACKOUT_H
#include <windows.h>
#define AM_BLACKOUT_TOGGLE (WM_APP + 3)
#define AM_BLACKOUT_CHANGED (WM_APP + 2)
BOOL am_blackout_init(HINSTANCE instance, HWND owner);
BOOL am_blackout_active(void);
BOOL am_blackout_toggle(HWND controls);
BOOL am_blackout_shortcut_ready(void);
BOOL am_blackout_testing(void);
void am_blackout_raw_input(LPARAM input);
void am_blackout_input_reset(void);
void am_blackout_hide(BOOL restore_focus);
void am_blackout_refresh(void);
void am_blackout_cleanup(void);
#endif
