#define UNICODE
#define _UNICODE
#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include "blackout.h"

static HWND cover, owner_window, previous_focus;
static BOOL active, registered;

BOOL am_blackout_active(void) { return active; }

void am_blackout_hide(BOOL restore_focus)
{
    if (!active) return;
    active = FALSE;
    ShowWindow(cover, SW_HIDE);
    /* Restore the normal cursor without changing the global ShowCursor count. */
    SetCursor(LoadCursorW(NULL, IDC_ARROW));
    if (restore_focus && IsWindow(previous_focus)) SetForegroundWindow(previous_focus);
    previous_focus = NULL;
    PostMessageW(owner_window, AM_BLACKOUT_CHANGED, 0, 0);
}

static BOOL place_cover(void)
{
    int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    if (width <= 0 || height <= 0) return FALSE;
    /* Include negative coordinates and monitors above/left of the primary. */
    return SetWindowPos(cover, HWND_TOPMOST,
        GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN),
        width, height, SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

void am_blackout_refresh(void)
{
    if (active && !place_cover()) am_blackout_hide(FALSE);
}

BOOL am_blackout_toggle(void)
{
    if (active) { am_blackout_hide(TRUE); return TRUE; }
    /* Never cover the desktop unless the same hotkey is registered to exit. */
    if (!registered || !cover) return FALSE;
    previous_focus = GetForegroundWindow();
    active = TRUE;
    if (!place_cover() || !SetForegroundWindow(cover)) {
        am_blackout_hide(TRUE);
        return FALSE;
    }
    SetCursor(NULL);
    UpdateWindow(cover);
    PostMessageW(owner_window, AM_BLACKOUT_CHANGED, 0, 0);
    return TRUE;
}

static LRESULT CALLBACK cover_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_SETCURSOR: SetCursor(NULL); return TRUE;
    case WM_ERASEBKGND: {
        RECT rect;
        GetClientRect(hwnd, &rect);
        FillRect((HDC)wp, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
        return 1;
    }
    case WM_PAINT: {
        PAINTSTRUCT paint;
        HDC dc = BeginPaint(hwnd, &paint);
        FillRect(dc, &paint.rcPaint, (HBRUSH)GetStockObject(BLACK_BRUSH));
        EndPaint(hwnd, &paint);
        return 0;
    }
    case WM_DISPLAYCHANGE: am_blackout_refresh(); return 0;
    case WM_MOUSEACTIVATE: return MA_ACTIVATEANDEAT;
    /* Neither real nor synthetic mouse input, nor Escape, dismisses the cover.
       A normal close (e.g. Alt+F4) remains available as an emergency exit. */
    case WM_CLOSE: am_blackout_hide(TRUE); return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

BOOL am_blackout_init(HINSTANCE instance, HWND owner)
{
    WNDCLASSW wc = {0};
    owner_window = owner;
    wc.lpfnWndProc = cover_proc;
    wc.hInstance = instance;
    wc.lpszClassName = L"AwakeMini.Blackout.v1";
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    if (!RegisterClassW(&wc)) return FALSE;
    cover = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, wc.lpszClassName,
        L"Awake Mini", WS_POPUP, 0, 0, 0, 0, NULL, NULL, instance, NULL);
    if (!cover) return FALSE;
    registered = RegisterHotKey(owner, AM_BLACKOUT_HOTKEY, MOD_WIN | MOD_NOREPEAT, VK_BACK);
    return registered;
}

void am_blackout_cleanup(void)
{
    am_blackout_hide(FALSE);
    if (registered) UnregisterHotKey(owner_window, AM_BLACKOUT_HOTKEY);
    registered = FALSE;
    if (cover) DestroyWindow(cover);
    cover = NULL;
}
