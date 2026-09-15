#define UNICODE
#define _UNICODE
#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include "blackout.h"
#include "blackout-keys.h"

static HWND cover, owner_window, previous_focus;
static BOOL active, registered, panel_topmost;
static HWND panel;
static AM_BLACKOUT_KEYS keys;

BOOL am_blackout_active(void) { return active; }
BOOL am_blackout_shortcut_ready(void) { return registered; }
BOOL am_blackout_testing(void) { return active && panel != NULL; }

void am_blackout_input_reset(void)
{
    unsigned i;
    ZeroMemory(&keys, sizeof(keys));
    for (i = 0; i < 256; ++i) keys.down[i] = (GetAsyncKeyState(i) & 0x8000) != 0;
}

void am_blackout_raw_input(LPARAM input)
{
    RAWINPUT raw;
    UINT bytes = sizeof(raw);
    if (GetRawInputData((HRAWINPUT)input, RID_INPUT, &raw, &bytes, sizeof(RAWINPUTHEADER)) == (UINT)-1 ||
        bytes < sizeof(RAWINPUTHEADER) + sizeof(RAWKEYBOARD) || raw.header.dwType != RIM_TYPEKEYBOARD) return;
    if (raw.data.keyboard.VKey == 255) return;
    if (am_blackout_key(&keys, raw.data.keyboard.VKey, (raw.data.keyboard.Flags & RI_KEY_BREAK) != 0))
        PostMessageW(owner_window, AM_BLACKOUT_TOGGLE, 0, 0);
}


void am_blackout_hide(BOOL restore_focus)
{
    if (!active) return;
    active = FALSE;
    ShowWindow(cover, SW_HIDE);
    if (IsWindow(panel) && !panel_topmost)
        SetWindowPos(panel, HWND_NOTOPMOST, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    panel = NULL;
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
    if (!SetWindowPos(cover, HWND_TOPMOST,
        GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN),
        width, height, SWP_NOACTIVATE | SWP_SHOWWINDOW)) return FALSE;
    if (IsWindow(panel) &&
        !SetWindowPos(panel, HWND_TOPMOST, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW)) return FALSE;
    return TRUE;
}

void am_blackout_refresh(void)
{
    if (active && !place_cover()) am_blackout_hide(FALSE);
}

BOOL am_blackout_toggle(HWND controls)
{
    if (active) { am_blackout_hide(TRUE); return TRUE; }
    /* Button mode works even if global input registration is unavailable. */
    if (!cover || (!registered && !IsWindow(controls))) return FALSE;
    previous_focus = GetForegroundWindow();
    panel = IsWindow(controls) ? controls : NULL;
    panel_topmost = panel && (GetWindowLongPtrW(panel, GWL_EXSTYLE) & WS_EX_TOPMOST);
    active = TRUE;
    if (!place_cover()) {
        am_blackout_hide(TRUE);
        return FALSE;
    }
    /* A denied focus request must not immediately undo a visible cover. */
    SetForegroundWindow(panel ? panel : cover);
    if (!panel) SetCursor(NULL);
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
    RAWINPUTDEVICE device = {0x01, 0x06, RIDEV_INPUTSINK, owner};
    owner_window = owner;
    wc.lpfnWndProc = cover_proc;
    wc.hInstance = instance;
    wc.lpszClassName = L"AwakeMini.Blackout.v1";
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    if (!RegisterClassW(&wc)) return FALSE;
    cover = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW, wc.lpszClassName,
        L"Awake Mini", WS_POPUP, 0, 0, 0, 0, NULL, NULL, instance, NULL);
    if (!cover) return FALSE;
    /* Background raw keyboard input avoids reserved Win-key registration.
       Do not suppress or inject keys. No keys are recorded or transmitted. */
    registered = RegisterRawInputDevices(&device, 1, sizeof(device));
    am_blackout_input_reset();
    return TRUE;
}

void am_blackout_cleanup(void)
{
    am_blackout_hide(FALSE);
    if (registered) {
        RAWINPUTDEVICE device = {0x01, 0x06, RIDEV_REMOVE, NULL};
        RegisterRawInputDevices(&device, 1, sizeof(device));
    }
    registered = FALSE;
    if (cover) DestroyWindow(cover);
    cover = NULL;
}
