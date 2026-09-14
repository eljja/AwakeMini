#ifndef AM_UPDATE_PAUSE_H
#define AM_UPDATE_PAUSE_H
#include <windows.h>
void up_tick(BOOL enabled, BOOL force);
const WCHAR *up_status(void);
BOOL up_ok(void);
void up_open_settings(HWND owner);
#endif
