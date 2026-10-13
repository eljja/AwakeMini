#ifndef AM_STATE_TEST_H
#define AM_STATE_TEST_H
#include <windows.h>
#define AM_TEST_MEDIA (WM_APP + 5)
#define AM_TEST_SIGNAL (WM_APP + 6)
void am_test_init(HINSTANCE instance, HWND owner);
void am_test_bind(HWND dialog);
void am_test_status(WCHAR *status); /* caller provides at least 160 WCHARs */
void am_test_signal_toggle(void);
void am_test_signal_stop(void);
BOOL am_test_signal_active(void);
void am_test_media_toggle(void);
BOOL am_test_media_active(void);
void am_test_stop(void);
void am_test_tick(BOOL allowed);
void am_test_shell_changed(void);
void am_test_cleanup(void);
#endif
