#ifndef AM_WIN_B_HOOK_H
#define AM_WIN_B_HOOK_H
#include <windows.h>
#define AM_WIN_B_TOGGLE (WM_APP + 3)
void am_keys_start(HINSTANCE instance, HWND owner);
void am_keys_reset(void);
void am_keys_cleanup(void);
void am_keys_status(WCHAR *text); /* caller supplies at least 160 WCHARs */
#endif
