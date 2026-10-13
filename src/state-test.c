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

static HWND app_owner, settings_dialog;
static BOOL signal_on, media_on, runtime_ready, can_run;
static HRESULT runtime_result = E_UNEXPECTED, media_result = S_OK;
static HRESULT signal_result = S_OK;
static HRESULT query_result = E_PENDING;
static QUERY_USER_NOTIFICATION_STATE notification_state;
static ITaskbarList2 *taskbar;
static AmMedia *media;
static AmUpdater *updater;
static INT32 media_status;
static BYTE media_enabled;
static void refresh(void);

BOOL am_test_signal_active(void) { return signal_on; }
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
void am_test_signal_stop(void)
{
    HRESULT hr = S_OK;
    if (!signal_on) return;
    if (taskbar && IsWindow(settings_dialog))
        hr = ITaskbarList2_MarkFullscreenWindow(taskbar, settings_dialog, FALSE);
    signal_on = FALSE;
    if (FAILED(hr)) {
        signal_result = hr;
        /* If unmarking fails, destroying our own target removes its lifetime
           and avoids leaving a stale shell mark on a still-live window. */
        if (IsWindow(settings_dialog)) DestroyWindow(settings_dialog);
    }
    refresh();
}
void am_test_signal_toggle(void)
{
    if (signal_on) { signal_result = S_OK; am_test_signal_stop(); return; }
    if (!can_run) { signal_result = HRESULT_FROM_WIN32(ERROR_CANCELLED); refresh(); return; }
    signal_result = taskbar_connect();
    if (FAILED(signal_result)) { refresh(); return; }
    /* Only the small dialog is marked. No maximize, resize, fullscreen cover,
       topmost change, power request or synthetic input is performed here. */
    if (!IsWindow(settings_dialog)) signal_result = E_HANDLE;
    else signal_result = ITaskbarList2_MarkFullscreenWindow(taskbar, settings_dialog, TRUE);
    signal_on = SUCCEEDED(signal_result);
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
void am_test_status(WCHAR *status)
{
    WCHAR flag[64], playback[64];
    if (FAILED(signal_result)) wsprintfW(flag, AM_TEXT(L"신호 오류: 0x%08lX"), (DWORD)signal_result);
    else lstrcpyW(flag, !signal_on ? L"OFF" : GetForegroundWindow() == settings_dialog ?
        AM_TEXT(L"표시 설정됨 · 활성 창") : AM_TEXT(L"표시 설정됨 · 비활성/숨김"));
    if (FAILED(media_result)) wsprintfW(playback, AM_TEXT(L"미디어 오류: 0x%08lX"), (DWORD)media_result);
    else lstrcpyW(playback, media_on ? L"Playing" : L"OFF");
    wsprintfW(status, AM_TEXT(L"신호: %s / 미디어: %s"), flag, playback);
}
static void refresh(void)
{
    WCHAR q[160];
    query_result = SHQueryUserNotificationState(&notification_state);
    if (!settings_dialog) return;
    if (FAILED(query_result)) wsprintfW(q, L"QUNS: 0x%08lX", (DWORD)query_result);
    else wsprintfW(q, L"QUNS: %s (%u)", query_name(), (UINT)notification_state);
    CheckDlgButton(settings_dialog, 1306, signal_on ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(settings_dialog, 1302, media_on ? BST_CHECKED : BST_UNCHECKED);
    EnableWindow(GetDlgItem(settings_dialog, 1306), can_run);
    EnableWindow(GetDlgItem(settings_dialog, 1302), can_run);
    SetDlgItemTextW(settings_dialog, 1304, q);
}
void am_test_bind(HWND dialog)
{
    if (settings_dialog != dialog) am_test_signal_stop();
    settings_dialog = dialog;
    refresh();
}
void am_test_tick(BOOL allowed)
{
    can_run = allowed;
    if (!allowed) { am_test_signal_stop(); media_stop(); }
    if (media_on) {
        HRESULT hr = media->lpVtbl->get_status(media, &media_status);
        if (SUCCEEDED(hr)) hr = media->lpVtbl->get_enabled(media, &media_enabled);
        if (SUCCEEDED(hr) && (media_status != 3 || !media_enabled)) hr = E_FAIL;
        if (FAILED(hr)) { media_stop(); media_result = hr; }
    }
    if (settings_dialog && IsWindowVisible(settings_dialog)) refresh();
}
void am_test_shell_changed(void)
{
    if (taskbar) { ITaskbarList2_Release(taskbar); taskbar = NULL; }
    if (signal_on) {
        signal_result = taskbar_connect();
        if (SUCCEEDED(signal_result)) signal_result = ITaskbarList2_MarkFullscreenWindow(taskbar, settings_dialog, TRUE);
        if (FAILED(signal_result)) am_test_signal_stop();
    }
    refresh();
}
void am_test_init(HINSTANCE instance, HWND owner)
{
    (void)instance;
    app_owner = owner;
    runtime_result = RoInitialize(RO_INIT_SINGLETHREADED);
    runtime_ready = SUCCEEDED(runtime_result);
}
void am_test_stop(void)
{
    am_test_signal_stop(); media_stop(); refresh();
}
void am_test_cleanup(void)
{
    am_test_stop();
    settings_dialog = NULL;
    if (taskbar) { ITaskbarList2_Release(taskbar); taskbar = NULL; }
    if (runtime_ready) { RoUninitialize(); runtime_ready = FALSE; }
}
