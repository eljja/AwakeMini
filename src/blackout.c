#define UNICODE
#define _UNICODE
#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include "blackout.h"
#include "language.h"

#define ID_RELEASE 1201
static HWND cover, release_button, owner_window, previous_focus, previous_settings;
static BOOL active;

BOOL am_blackout_active(void) { return active; }

void am_blackout_hide(BOOL restore_focus)
{
    HWND target;
    if (!active) return;
    active = FALSE;
    ShowWindow(cover, SW_HIDE);
    SetCursor(LoadCursorW(NULL, IDC_ARROW));
    /* Only restore a settings window that was visible before blackout. */
    target = IsWindow(previous_settings) ? previous_settings : previous_focus;
    if (restore_focus && IsWindow(previous_settings)) ShowWindow(previous_settings, SW_SHOWNORMAL);
    if (restore_focus && IsWindow(target)) SetForegroundWindow(target);
    previous_settings = previous_focus = NULL;
    PostMessageW(owner_window, AM_BLACKOUT_CHANGED, 0, 0);
}

static BOOL place_cover(void)
{
    int left = GetSystemMetrics(SM_XVIRTUALSCREEN), top = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int width = GetSystemMetrics(SM_CXVIRTUALSCREEN), height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    int primary_width = GetSystemMetrics(SM_CXSCREEN), primary_height = GetSystemMetrics(SM_CYSCREEN);
    UINT dpi = GetDpiForWindow(cover);
    int button_width = MulDiv(56, dpi ? (int)dpi : 96, 96);
    int button_height = MulDiv(24, dpi ? (int)dpi : 96, 96);
    int margin = MulDiv(12, dpi ? (int)dpi : 96, 96);
    int x, y;
    if (width <= 0 || height <= 0 || primary_width <= 0 || primary_height <= 0) return FALSE;
    /* The primary monitor starts at (0,0); convert its bottom-right corner
       into the virtual-desktop cover's client coordinates. */
    x = -left + primary_width - button_width - margin;
    y = -top + primary_height - button_height - margin;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    if (x + button_width > width || y + button_height > height) return FALSE;
    /* The release control must be placed before exposing the cover. */
    if (!SetWindowPos(release_button, NULL, x, y, button_width, button_height,
            SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW)) return FALSE;
    return SetWindowPos(cover, HWND_TOPMOST, left, top, width, height,
        SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

void am_blackout_refresh(void)
{
    if (active && !place_cover()) am_blackout_hide(TRUE);
}

BOOL am_blackout_toggle(HWND settings)
{
    if (active) { am_blackout_hide(TRUE); return TRUE; }
    if (!cover || !release_button) return FALSE;
    previous_focus = GetForegroundWindow();
    previous_settings = IsWindow(settings) && IsWindowVisible(settings) ? settings : NULL;
    active = TRUE;
    if (!place_cover()) { am_blackout_hide(TRUE); return FALSE; }
    if (previous_settings) ShowWindow(previous_settings, SW_HIDE);
    /* A denied focus request does not cancel a visible, clickable cover. */
    SetForegroundWindow(cover);
    SetFocus(release_button);
    SetCursor(NULL);
    UpdateWindow(cover);
    PostMessageW(owner_window, AM_BLACKOUT_CHANGED, 0, 0);
    return TRUE;
}

static LRESULT CALLBACK cover_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg) {
    case WM_COMMAND:
        if (LOWORD(wp) == ID_RELEASE && HIWORD(wp) == BN_CLICKED) {
            am_blackout_hide(TRUE);
            return 0;
        }
        break;
    case WM_SETCURSOR:
        SetCursor((HWND)wp == release_button ? LoadCursorW(NULL, IDC_ARROW) : NULL);
        return TRUE;
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
    /* The first click on Release must work even if another app has focus. */
    case WM_MOUSEACTIVATE: return MA_ACTIVATE;
    /* Mouse movement and Escape do not dismiss the cover; Alt+F4 still can. */
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
        L"Awake Mini", WS_POPUP | WS_CLIPCHILDREN, 0, 0, 0, 0, NULL, NULL, instance, NULL);
    if (!cover) return FALSE;
    release_button = CreateWindowExW(0, L"Button", AM_TEXT(L"해제"),
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        0, 0, 0, 0, cover, (HMENU)(UINT_PTR)ID_RELEASE, instance, NULL);
    if (!release_button) { DestroyWindow(cover); cover = NULL; return FALSE; }
    SendMessageW(release_button, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), TRUE);
    return TRUE;
}

void am_blackout_cleanup(void)
{
    am_blackout_hide(FALSE);
    if (cover) DestroyWindow(cover); /* Also destroys the child button. */
    cover = release_button = NULL;
}
