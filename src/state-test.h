#ifndef AM_STATE_TEST_H
#define AM_STATE_TEST_H
#include <windows.h>
#define AM_TEST_FULLSCREEN (WM_APP + 4)
#define AM_TEST_MEDIA (WM_APP + 5)
void am_test_init(HINSTANCE instance, HWND owner);
void am_test_show(void);
void am_test_full_toggle(void);
void am_test_media_toggle(void);
BOOL am_test_full_active(void);
BOOL am_test_media_active(void);
void am_test_full_stop(BOOL restore);
void am_test_stop(void);
void am_test_tick(BOOL allowed);
void am_test_display_changed(void);
void am_test_shell_changed(void);
BOOL am_test_message(MSG *msg);
void am_test_cleanup(void);
#endif
