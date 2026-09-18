#define UNICODE
#define _UNICODE
#define COBJMACROS
#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <shellapi.h>
#include <shobjidl.h>
#include <roapi.h>
#include <winstring.h>
#include <systemmediatransportcontrolsinterop.h>
#include "media-abi.h"
#include "state-test.h"
#include "language.h"

static HINSTANCE app_instance;
static HWND app_owner, test_dialog, full_window, release_button, full_label, previous_focus;
static BOOL full_on, media_on, runtime_ready, can_run, restore_dialog;
static HRESULT runtime_result = E_UNEXPECTED, full_result = S_OK, media_result = S_OK;
static HRESULT query_result = E_PENDING;
static QUERY_USER_NOTIFICATION_STATE notification_state;
static ITaskbarList2 *taskbar;
static AmMedia *media;
static AmUpdater *updater;
static INT32 media_status;
static BYTE media_enabled;
static void refresh(void);

BOOL am_test_full_active(void) { return full_on; }
BOOL am_test_media_active(void) { return media_on; }

static HRESULT make_string(const WCHAR *text, HSTRING *value)
{
    return WindowsCreateString(text, (UINT32)lstrlenW(text), value);
}
static void media_stop(void)
{
    HRESULT hr, first = S_OK;
    if (media) {
        hr = media->lpVtbl->put_status(media, 0); /* Closed */
        if (FAILED(hr)) first = hr;
        hr = media->lpVtbl->put_enabled(media, 0);
        if (FAILED(hr) && SUCCEEDED(first)) first = hr;
    }
    if (updater) {
        hr = updater->lpVtbl->clear(updater);
        if (FAILED(hr) && SUCCEEDED(first)) first = hr;
        IInspectable_Release((IInspectable *)updater); updater = NULL;
    }
    if (media) { IInspectable_Release((IInspectable *)media); media = NULL; }
    media_on = FALSE; media_status = 0; media_enabled = 0;
    if (FAILED(first)) media_result = first;
}
void am_test_media_toggle(void)
{
    ISystemMediaTransportControlsInterop *factory = NULL;
    AmVideo *video = NULL;
    HSTRING name = NULL, title = NULL;
    HRESULT hr;
    if (media_on) { media_result = S_OK; media_stop(); refresh(); return; }
    if (!can_run) { media_result = HRESULT_FROM_WIN32(ERROR_CANCELLED); refresh(); return; }
    if (!runtime_ready) { media_result = runtime_result; refresh(); return; }
    hr = make_string(L"Windows.Media.SystemMediaTransportControls", &name);
    if (SUCCEEDED(hr)) hr = RoGetActivationFactory(name, &AM_IID_SMTC_INTEROP, (void **)&factory);
    if (SUCCEEDED(hr)) hr = ISystemMediaTransportControlsInterop_GetForWindow(factory, app_owner, &AM_IID_SMTC, (void **)&media);
    if (SUCCEEDED(hr)) hr = media->lpVtbl->get_updater(media, &updater);
    if (SUCCEEDED(hr)) hr = updater->lpVtbl->put_type(updater, 2); /* Video */
    if (SUCCEEDED(hr)) hr = updater->lpVtbl->get_video(updater, &video);
    if (SUCCEEDED(hr)) hr = make_string(AM_TEXT(L"Awake Mini · 미디어 상태 시험 (영상·소리 없음)"), &title);
    if (SUCCEEDED(hr)) hr = video->lpVtbl->put_title(video, title);
    if (SUCCEEDED(hr)) hr = updater->lpVtbl->update(updater);
    if (SUCCEEDED(hr)) hr = media->lpVtbl->put_enabled(media, 1);
    if (SUCCEEDED(hr)) hr = media->lpVtbl->put_status(media, 3); /* Playing; state-only experiment */
    if (SUCCEEDED(hr)) hr = media->lpVtbl->get_status(media, &media_status);
    if (SUCCEEDED(hr)) hr = media->lpVtbl->get_enabled(media, &media_enabled);
    if (SUCCEEDED(hr) && (media_status != 3 || !media_enabled)) hr = E_FAIL;
    if (video) IInspectable_Release((IInspectable *)video);
    if (factory) ISystemMediaTransportControlsInterop_Release(factory);
    WindowsDeleteString(title); WindowsDeleteString(name);
    if (FAILED(hr)) media_stop();
    media_result = hr; media_on = SUCCEEDED(hr);
    refresh();
}
static HRESULT taskbar_connect(void)
{
    HRESULT hr;
    if (!runtime_ready) return runtime_result;
    if (taskbar) return S_OK;
    hr = CoCreateInstance(&CLSID_TaskbarList, NULL, CLSCTX_INPROC_SERVER, &IID_ITaskbarList2, (void **)&taskbar);
    if (SUCCEEDED(hr)) hr = ITaskbarList2_HrInit(taskbar);
    if (FAILED(hr) && taskbar) { ITaskbarList2_Release(taskbar); taskbar = NULL; }
    return hr;
}
static BOOL place_full(void)
{
    MONITORINFO info = {sizeof(info), {0}, {0}, 0};
    HMONITOR monitor = MonitorFromWindow(full_window, MONITOR_DEFAULTTOPRIMARY);
    UINT dpi = GetDpiForWindow(full_window);
    int margin = MulDiv(12, dpi ? (int)dpi : 96, 96);
    int width, height, bw = MulDiv(80, dpi ? (int)dpi : 96, 96), bh = MulDiv(28, dpi ? (int)dpi : 96, 96);
    if (!GetMonitorInfoW(monitor, &info)) return FALSE;
    width = info.rcMonitor.right - info.rcMonitor.left; height = info.rcMonitor.bottom - info.rcMonitor.top;
    if (width < bw + 2*margin || height < bh + 2*margin) return FALSE;
    if (!SetWindowPos(release_button, NULL, width-bw-margin, height-bh-margin, bw, bh, SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW)) return FALSE;
    if (!SetWindowPos(full_label, NULL, margin, margin, width-2*margin, bh*3, SWP_NOZORDER | SWP_NOACTIVATE | SWP_SHOWWINDOW)) return FALSE;
    return SetWindowPos(full_window, HWND_TOPMOST, info.rcMonitor.left, info.rcMonitor.top, width, height, SWP_SHOWWINDOW);
}
void am_test_full_stop(BOOL restore)
{
    if (!full_on) return;
    if (taskbar) {
        HRESULT hr = ITaskbarList2_MarkFullscreenWindow(taskbar, full_window, FALSE);
        if (FAILED(hr)) full_result = hr;
    }
    full_on = FALSE;
    ShowWindow(full_window, SW_HIDE);
    if (restore) {
        if (restore_dialog && IsWindow(test_dialog)) { ShowWindow(test_dialog, SW_SHOWNORMAL); SetForegroundWindow(test_dialog); }
        else if (IsWindow(previous_focus)) SetForegroundWindow(previous_focus);
    }
    restore_dialog = FALSE; previous_focus = NULL;
    refresh();
}
void am_test_full_toggle(void)
{
    if (full_on) { full_result = S_OK; am_test_full_stop(TRUE); return; }
    if (!can_run) { full_result = HRESULT_FROM_WIN32(ERROR_CANCELLED); refresh(); return; }
    if (!full_window || !release_button || !full_label) { full_result = E_HANDLE; refresh(); return; }
    full_result = taskbar_connect();
    if (FAILED(full_result)) { refresh(); return; }
    previous_focus = GetForegroundWindow();
    restore_dialog = test_dialog && IsWindowVisible(test_dialog);
    full_on = TRUE;
    full_result = ITaskbarList2_MarkFullscreenWindow(taskbar, full_window, TRUE);
    if (SUCCEEDED(full_result) && !place_full()) full_result = E_FAIL;
    if (FAILED(full_result)) { am_test_full_stop(TRUE); return; }
    if (restore_dialog) ShowWindow(test_dialog, SW_HIDE);
    SetForegroundWindow(full_window); SetFocus(release_button);
    refresh();
}
static const WCHAR *query_name(void)
{
    switch (notification_state) {
    case QUNS_NOT_PRESENT: return L"Not present";
    case QUNS_BUSY: return L"Busy / fullscreen";
    case QUNS_RUNNING_D3D_FULL_SCREEN: return L"D3D fullscreen";
    case QUNS_PRESENTATION_MODE: return L"Presentation mode";
    case QUNS_ACCEPTS_NOTIFICATIONS: return L"Accepts notifications";
    case QUNS_QUIET_TIME: return L"Quiet time";
    case QUNS_APP: return L"Store app";
    default: return L"Unknown";
    }
}
static void refresh(void)
{
    WCHAR f[160], m[160], q[160], caption[400];
    query_result = SHQueryUserNotificationState(&notification_state);
    if (FAILED(full_result)) wsprintfW(f, AM_TEXT(L"전체화면 오류: 0x%08lX"), (DWORD)full_result);
    else lstrcpyW(f, full_on ? AM_TEXT(L"전체화면 알림 ON") : L"OFF");
    if (FAILED(media_result)) wsprintfW(m, AM_TEXT(L"미디어 오류: 0x%08lX"), (DWORD)media_result);
    else if (media_on) wsprintfW(m, L"Enabled=%u / Playing=%ld", (UINT)media_enabled, (LONG)media_status);
    else lstrcpyW(m, L"OFF");
    if (FAILED(query_result)) wsprintfW(q, L"QUNS: 0x%08lX", (DWORD)query_result);
    else wsprintfW(q, L"QUNS: %s (%u)", query_name(), (UINT)notification_state);
    if (test_dialog) {
        CheckDlgButton(test_dialog, 1301, full_on ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(test_dialog, 1302, media_on ? BST_CHECKED : BST_UNCHECKED);
        EnableWindow(GetDlgItem(test_dialog, 1301), can_run);
        EnableWindow(GetDlgItem(test_dialog, 1302), can_run);
        SetDlgItemTextW(test_dialog, 1303, f); SetDlgItemTextW(test_dialog, 1304, q); SetDlgItemTextW(test_dialog, 1305, m);
    }
    if (full_on) {
        wsprintfW(caption, L"%s\r\n%s\r\n%s", AM_TEXT(L"전체화면 시험 · Esc 또는 해제로 종료"), q, m);
        SetWindowTextW(full_label, caption);
    }
}
void am_test_tick(BOOL allowed)
{
    can_run = allowed;
    if (!allowed) { am_test_full_stop(FALSE); media_stop(); }
    if (media_on) {
        HRESULT hr = media->lpVtbl->get_status(media, &media_status);
        if (SUCCEEDED(hr)) hr = media->lpVtbl->get_enabled(media, &media_enabled);
        if (SUCCEEDED(hr) && (media_status != 3 || !media_enabled)) hr = E_FAIL;
        if (FAILED(hr)) { media_stop(); media_result = hr; }
    }
    if (full_on || (test_dialog && IsWindowVisible(test_dialog))) refresh();
}
void am_test_display_changed(void)
{
    if (full_on && !place_full()) { full_result = E_FAIL; am_test_full_stop(FALSE); }
}
void am_test_shell_changed(void)
{
    if (taskbar) { ITaskbarList2_Release(taskbar); taskbar = NULL; }
    if (full_on) {
        full_result = taskbar_connect();
        if (SUCCEEDED(full_result)) full_result = ITaskbarList2_MarkFullscreenWindow(taskbar, full_window, TRUE);
        if (FAILED(full_result)) am_test_full_stop(FALSE);
    }
}
static LRESULT CALLBACK full_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    if (msg == WM_CLOSE || (msg == WM_COMMAND && LOWORD(wp) == 1310)) { am_test_full_stop(TRUE); return 0; }
    if (msg == WM_CTLCOLORSTATIC) {
        SetTextColor((HDC)wp, RGB(200,200,200)); SetBkColor((HDC)wp, RGB(0,0,0));
        return (LRESULT)GetStockObject(BLACK_BRUSH);
    }
    if (msg == WM_DISPLAYCHANGE) { am_test_display_changed(); return 0; }
    return DefWindowProcW(hwnd, msg, wp, lp);
}
static INT_PTR CALLBACK test_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    (void)lp;
    switch (msg) {
    case WM_INITDIALOG: test_dialog = hwnd; am_translate_window(hwnd); refresh(); return TRUE;
    case WM_COMMAND:
        if (LOWORD(wp) == 1301) { PostMessageW(app_owner, AM_TEST_FULLSCREEN, 0, 0); return TRUE; }
        if (LOWORD(wp) == 1302) { PostMessageW(app_owner, AM_TEST_MEDIA, 0, 0); return TRUE; }
        if (LOWORD(wp) == IDCANCEL) { ShowWindow(hwnd, SW_HIDE); return TRUE; }
        break;
    case WM_CLOSE: ShowWindow(hwnd, SW_HIDE); return TRUE;
    case WM_DESTROY: test_dialog = NULL; return TRUE;
    }
    return FALSE;
}
void am_test_show(void)
{
    if (!test_dialog) test_dialog = CreateDialogParamW(app_instance, MAKEINTRESOURCEW(1300), app_owner, test_proc, 0);
    if (test_dialog) { refresh(); ShowWindow(test_dialog, SW_SHOWNORMAL); SetForegroundWindow(test_dialog); }
}
BOOL am_test_message(MSG *msg)
{
    if (full_on && msg->message == WM_KEYDOWN && msg->wParam == VK_ESCAPE) { am_test_full_stop(TRUE); return TRUE; }
    return test_dialog && IsWindowVisible(test_dialog) && IsDialogMessageW(test_dialog, msg);
}
void am_test_init(HINSTANCE instance, HWND owner)
{
    WNDCLASSW wc = {0};
    app_instance = instance; app_owner = owner;
    runtime_result = RoInitialize(RO_INIT_SINGLETHREADED);
    runtime_ready = SUCCEEDED(runtime_result);
    wc.lpfnWndProc = full_proc; wc.hInstance = instance;
    wc.lpszClassName = L"AwakeMini.StateTest.Fullscreen.v1";
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH); wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
    if (!RegisterClassW(&wc)) { full_result = HRESULT_FROM_WIN32(GetLastError()); return; }
    full_window = CreateWindowExW(WS_EX_TOOLWINDOW, wc.lpszClassName, L"Awake Mini - Fullscreen test", WS_POPUP | WS_CLIPCHILDREN, 0,0,0,0, NULL,NULL,instance,NULL);
    if (!full_window) { full_result = HRESULT_FROM_WIN32(GetLastError()); return; }
    release_button = CreateWindowExW(0,L"Button",AM_TEXT(L"해제"),WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,0,0,0,0,full_window,(HMENU)(UINT_PTR)1310,instance,NULL);
    full_label = CreateWindowExW(0,L"Static",L"",WS_CHILD | WS_VISIBLE,0,0,0,0,full_window,NULL,instance,NULL);
    if (!release_button || !full_label) { full_result = E_HANDLE; DestroyWindow(full_window); full_window = release_button = full_label = NULL; return; }
    SendMessageW(release_button,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),TRUE);
    SendMessageW(full_label,WM_SETFONT,(WPARAM)GetStockObject(DEFAULT_GUI_FONT),TRUE);
}
void am_test_stop(void)
{
    am_test_full_stop(FALSE); media_stop(); refresh();
}
void am_test_cleanup(void)
{
    am_test_stop();
    if (test_dialog) DestroyWindow(test_dialog);
    if (full_window) DestroyWindow(full_window);
    full_window = release_button = full_label = NULL;
    if (taskbar) { ITaskbarList2_Release(taskbar); taskbar = NULL; }
    if (runtime_ready) { RoUninitialize(); runtime_ready = FALSE; }
}
