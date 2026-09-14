#ifndef AM_LANGUAGE_H
#define AM_LANGUAGE_H
#include <windows.h>
void am_language_init(void);
const WCHAR *am_text(const WCHAR *source);
void am_translate_window(HWND window);
#define AM_TEXT(x) am_text(x)
#endif
