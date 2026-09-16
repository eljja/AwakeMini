#define UNICODE
#define _UNICODE
#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <shellapi.h>
#include <wtsapi32.h>
#include "policy.h"
#include "update-pause.h"
#include "language.h"
#include "startup.h"
#include "blackout.h"
#include "win-b-hook.h"

#define APP L"Awake Mini"
#define CLASS L"AwakeMini.Window.v1"
#define KEY L"Software\\AwakeMini"
#define WM_TRAY (WM_APP + 1)
#define ID_SLEEP 101
#define ID_DISPLAY 102
#define ID_MOUSE 103
#define ID_PAUSE 104
#define ID_ABOUT 105
#define ID_EXIT 106
#define ID_SETTINGS 107
#define ID_UPDATE_SETTINGS 108
#define ID_BLACKOUT 109
#define ID_REHOOK 110
#define ID_INTERVAL 200

static const DWORD intervals[] = {60, 120, 300, 600};
static const WCHAR *labels[] = {L"1분", L"2분 (기본)", L"5분", L"10분"};
static HWND window;
static HWND settings;
static HINSTANCE instance;
static HANDLE mutex;
static BOOL sleep_on = TRUE, display_on = TRUE, mouse_on = TRUE;
static BOOL update_on;
static BOOL paused, locked, available = TRUE, tray_added, power_ok = TRUE;
static BOOL saver_blocked, saver_known, blackout_ready;
static DWORD interval_s = 120, last_attempt, applied = 0xffffffffu;
static UINT taskbar_created;
static HICON icons[3];
static NOTIFYICONDATAW tray;
static const WCHAR *mouse_status = L"대기 중";
static void show_settings(void);

static DWORD read_value(HKEY key, const WCHAR *name, DWORD fallback)
{
    DWORD value, type, bytes = sizeof(value);
    if (RegQueryValueExW(key, name, NULL, &type, (BYTE *)&value, &bytes) == ERROR_SUCCESS &&
        type == REG_DWORD && bytes == sizeof(value)) return value;
    return fallback;
}
static void load_settings(void)
{
    HKEY key;
    DWORD value;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, KEY, 0, KEY_QUERY_VALUE, &key) != ERROR_SUCCESS) return;
    sleep_on = read_value(key, L"Sleep", 1) != 0;
    display_on = read_value(key, L"Display", 1) != 0;
    mouse_on = read_value(key, L"Mouse", 1) != 0;
    update_on = read_value(key, L"UpdatePause", 0) != 0;
    value = read_value(key, L"IntervalSeconds", 120);
    /* Migrate the old one-minute default once; preserve custom intervals. */
    if (read_value(key, L"SettingsVersion", 0) < 101 && value == 60) value = 120;
    if (value >= 60 && value <= 86400 && value % 60 == 0) interval_s = value;
    RegCloseKey(key);
}
static BOOL save_settings(void)
{
    HKEY key;
    DWORD values[] = {(DWORD)sleep_on, (DWORD)display_on, (DWORD)mouse_on, interval_s, 110, (DWORD)update_on};
    const WCHAR *names[] = {L"Sleep", L"Display", L"Mouse", L"IntervalSeconds", L"SettingsVersion", L"UpdatePause"};
    unsigned i;
    BOOL ok = TRUE;
    if (RegCreateKeyExW(HKEY_CURRENT_USER, KEY, 0, NULL, 0, KEY_SET_VALUE, NULL, &key, NULL) != ERROR_SUCCESS) return FALSE;
    for (i = 0; i < 6; ++i) {
        if (RegSetValueExW(key, names[i], 0, REG_DWORD, (BYTE *)&values[i], sizeof(DWORD)) != ERROR_SUCCESS) ok = FALSE;
    }
    RegCloseKey(key);
    return ok;
}
/* Only inspect desktop state. Never switch desktops or alter lock policy. */
static BOOL desktop_available(void)
{
    HDESK desk;
    WCHAR name[128];
    DWORD needed;
    BOOL ok;
    if (locked) return FALSE;
    desk = OpenInputDesktop(0, FALSE, DESKTOP_READOBJECTS);
    if (!desk) return FALSE;
    ok = GetUserObjectInformationW(desk, UOI_NAME, name, sizeof(name), &needed);
    CloseDesktop(desk);
    return ok && lstrcmpiW(name, L"Default") == 0;
}
static void inspect_saver(void)
{
    BOOL blocked = FALSE;
    saver_known = SystemParametersInfoW(SPI_GETBLOCKSENDINPUTRESETS, 0, &blocked, 0);
    saver_blocked = saver_known && blocked;
}
static void update_power(void)
{
    BOOL black = am_blackout_active();
    DWORD flags = am_power(sleep_on || black, display_on || black, paused && !black, available);
    if (flags != applied) {
        power_ok = SetThreadExecutionState(ES_CONTINUOUS | flags) != 0;
        if (power_ok) applied = flags;
    }
}
static void update_tray(void)
{
    BOOL black = am_blackout_active();
    BOOL active = available && (black || (!paused && (sleep_on || mouse_on || update_on)));
    int icon = active ? 0 : 1;
    const WCHAR *state = black ? AM_TEXT(L"검은 화면") : paused ? AM_TEXT(L"일시정지") : (!available ? AM_TEXT(L"잠금 / 세션 대기") : (active ? AM_TEXT(L"실행 중") : AM_TEXT(L"모두 꺼짐")));
    if (active && (!power_ok || ((mouse_on || black) && saver_blocked) || (update_on && !up_ok()))) icon = 2;
    tray.hIcon = icons[icon];
    wsprintfW(tray.szTip, AM_TEXT(L"Awake Mini — %s\n절전 %s / 화면 %s / 마우스 %s (%lu분)\n더블클릭: 설정"), state,
        (sleep_on || black) ? L"ON" : L"OFF", (black || (sleep_on && display_on)) ? L"ON" : L"OFF",
        (mouse_on || black) ? L"ON" : L"OFF", interval_s / 60);
    if (tray_added) {
        if (!Shell_NotifyIconW(NIM_MODIFY, &tray)) tray_added = FALSE;
    }
    if (!tray_added) tray_added = Shell_NotifyIconW(NIM_ADD, &tray);
}
static BOOL any_key_down(void)
{
    int key;
    /* Includes held mouse buttons, modifiers and keys; do not disturb a drag. */
    for (key = 1; key < 256; ++key) if (GetAsyncKeyState(key) & 0x8000) return TRUE;
    return FALSE;
}
static LONG normalize(LONG p, LONG origin, LONG span)
{
    /* Pixel centre, virtual desktop; 64-bit intermediate supports wide layouts. */
    return (LONG)(((LONGLONG)(p - origin) * 65536 + 32768) / span);
}
static void try_mouse(void)
{
    LASTINPUTINFO last = {sizeof(last), 0};
    INPUT input[2] = {{0}};
    POINT from, to;
    RECT clip;
    DWORD now;
    LONG x, y, w, h;
    UINT sent;
    BOOL black = am_blackout_active();
    if ((!mouse_on && !black) || (paused && !black) || !available) return;
    if (saver_blocked) { mouse_status = AM_TEXT(L"모의 입력이 화면보호기 설정에 의해 제한됨"); return; }
    if (!GetLastInputInfo(&last)) { mouse_status = AM_TEXT(L"유휴 시간 확인 실패"); return; }
    now = GetTickCount();
    if (!am_due(now, last.dwTime, last_attempt, interval_s * 1000u,
                mouse_on || black, paused && !black, available, 0, saver_blocked)) return;
    if (any_key_down() || !GetCursorPos(&from) || !GetClipCursor(&clip)) return;
    to = from;
    if (from.x + 1 < clip.right) ++to.x;
    else if (from.x > clip.left) --to.x;
    else if (from.y + 1 < clip.bottom) ++to.y;
    else if (from.y > clip.top) --to.y;
    else return;
    x = GetSystemMetrics(SM_XVIRTUALSCREEN); y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    w = GetSystemMetrics(SM_CXVIRTUALSCREEN); h = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    if (w <= 0 || h <= 0) return;
    input[0].type = input[1].type = INPUT_MOUSE;
    input[0].mi.dwFlags = input[1].mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK;
    input[0].mi.dx = normalize(to.x, x, w); input[0].mi.dy = normalize(to.y, y, h);
    input[1].mi.dx = normalize(from.x, x, w); input[1].mi.dy = normalize(from.y, y, h);
    last_attempt = now;
    /* Submit move + return together, without a sleep that could interrupt a user. */
    sent = SendInput(2, input, sizeof(INPUT));
    mouse_status = sent == 2 ? AM_TEXT(L"최근 이동 입력 전송 성공") : AM_TEXT(L"이동 입력 전송 실패 / 일부 전송");
    /* Success means input was queued; it does not prove a policy timer was reset. */
}
static void show_menu(void)
{
    HMENU menu = CreatePopupMenu(), timing = CreatePopupMenu();
    POINT point;
    WCHAR status[160];
    UINT command;
    unsigned i;
    inspect_saver();
    if (!menu || !timing) { if (menu) DestroyMenu(menu); if (timing) DestroyMenu(timing); return; }
    AppendMenuW(menu, MF_STRING | MF_GRAYED, 0, paused ? AM_TEXT(L"Awake Mini · 일시정지") : AM_TEXT(L"Awake Mini · 트레이 모드"));
    AppendMenuW(menu, MF_STRING, ID_SETTINGS, AM_TEXT(L"설정 열기 (더블클릭)"));
    AppendMenuW(menu, MF_STRING | (blackout_ready ? 0 : MF_GRAYED) |
        (am_blackout_active() ? MF_CHECKED : 0), ID_BLACKOUT,
        blackout_ready ? AM_TEXT(L"검은화면") :
        AM_TEXT(L"검은 화면 사용 불가 · 창 생성 실패"));
    am_keys_status(status);
    AppendMenuW(menu, MF_STRING | MF_GRAYED, 0, status);
    AppendMenuW(menu, MF_STRING | (blackout_ready ? 0 : MF_GRAYED), ID_REHOOK, AM_TEXT(L"Win+B 훅 다시 연결"));
    AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(menu, MF_STRING | (sleep_on ? MF_CHECKED : 0), ID_SLEEP, AM_TEXT(L"절전 방지 · 무기한 유지"));
    AppendMenuW(menu, MF_STRING | (display_on ? MF_CHECKED : 0) | (!sleep_on ? MF_GRAYED : 0), ID_DISPLAY, AM_TEXT(L"화면 켜기 유지"));
    AppendMenuW(menu, MF_STRING | (mouse_on ? MF_CHECKED : 0), ID_MOUSE, AM_TEXT(L"화면보호기 방지 · 마우스 미세 이동"));
    for (i = 0; i < 4; ++i) AppendMenuW(timing, MF_STRING | (interval_s == intervals[i] ? MF_CHECKED : 0), ID_INTERVAL + i, AM_TEXT(labels[i]));
    AppendMenuW(menu, MF_POPUP | MF_STRING, (UINT_PTR)timing, AM_TEXT(L"마우스 이동 전 유휴 시간"));
    AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(menu, MF_STRING | (paused ? MF_CHECKED : 0), ID_PAUSE, AM_TEXT(L"전체 일시정지 / 다시 시작"));
    if (!available) AppendMenuW(menu, MF_STRING | MF_GRAYED, 0, AM_TEXT(L"현재 세션에서는 동작 대기 중"));
    if (!power_ok) AppendMenuW(menu, MF_STRING | MF_GRAYED, 0, AM_TEXT(L"전원 유지 요청 실패"));
    if (mouse_on) {
        wsprintfW(status, AM_TEXT(L"마우스: %s"), saver_blocked ? AM_TEXT(L"모의 입력 제한 설정 감지") : mouse_status);
        AppendMenuW(menu, MF_STRING | MF_GRAYED, 0, status);
    }
    AppendMenuW(menu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(menu, MF_STRING, ID_UPDATE_SETTINGS, AM_TEXT(L"업데이트 설정"));
    AppendMenuW(menu, MF_STRING, ID_ABOUT, AM_TEXT(L"사용 안내 / 정보"));
    AppendMenuW(menu, MF_STRING, ID_EXIT, AM_TEXT(L"종료"));
    GetCursorPos(&point);
    SetForegroundWindow(window);
    command = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_RIGHTBUTTON, point.x, point.y, 0, window, NULL);
    PostMessageW(window, WM_NULL, 0, 0);
    DestroyMenu(menu);
    if (command) SendMessageW(window, WM_COMMAND, command, 0);
}
static void refresh_blackout_button(void)
{
    if (!settings) return;
    EnableWindow(GetDlgItem(settings, 1012), blackout_ready);
    SetDlgItemTextW(settings, 1012,
        am_blackout_active() ? AM_TEXT(L"검은화면 해제") : AM_TEXT(L"검은화면"));
}
static void refresh_settings(void)
{
    if (!settings) return;
    refresh_blackout_button();
    CheckDlgButton(settings, 1001, sleep_on ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(settings, 1002, display_on ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(settings, 1003, mouse_on ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(settings, 1011, am_startup_registered() ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(settings, 1008, update_on ? BST_CHECKED : BST_UNCHECKED);
    SetDlgItemTextW(settings, 1009, up_status());
    CheckDlgButton(settings, 1005, paused ? BST_CHECKED : BST_UNCHECKED);
    SetDlgItemInt(settings, 1004, interval_s / 60, FALSE);
    EnableWindow(GetDlgItem(settings, 1002), sleep_on);
    EnableWindow(GetDlgItem(settings, 1004), mouse_on);
    SetDlgItemTextW(settings, IDOK, AM_TEXT(L"적용"));
}
static INT_PTR CALLBACK dialog_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    (void)lp;
    switch (msg) {
    case WM_INITDIALOG:
        settings = hwnd;
        am_translate_window(hwnd);
        SendMessageW(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)icons[0]);
        SendDlgItemMessageW(hwnd, 1004, EM_SETLIMITTEXT, 4, 0);
        refresh_settings();
        return TRUE;
    case WM_COMMAND:
        if (((LOWORD(wp) >= 1001 && LOWORD(wp) <= 1005) || LOWORD(wp) == 1008 || LOWORD(wp) == 1011) &&
            (HIWORD(wp) == BN_CLICKED || HIWORD(wp) == EN_CHANGE))
            SetDlgItemTextW(hwnd, IDOK, AM_TEXT(L"적용"));
        switch (LOWORD(wp)) {
        case 1012:
            available = desktop_available();
            if (available) am_blackout_toggle(hwnd);
            update_power(); update_tray(); refresh_blackout_button();
            return TRUE;
        case 1001:
            EnableWindow(GetDlgItem(hwnd, 1002), IsDlgButtonChecked(hwnd, 1001) == BST_CHECKED);
            return TRUE;
        case 1003:
            EnableWindow(GetDlgItem(hwnd, 1004), IsDlgButtonChecked(hwnd, 1003) == BST_CHECKED);
            return TRUE;
        case IDOK: {
            BOOL valid, saved, want_startup;
            LONG startup_result = ERROR_SUCCESS;
            UINT minutes = GetDlgItemInt(hwnd, 1004, &valid, FALSE);
            if (!valid || minutes < 1 || minutes > 1440) {
                MessageBoxW(hwnd, AM_TEXT(L"대기 시간은 1~1440분의 정수를 입력하세요."), APP, MB_OK | MB_ICONINFORMATION);
                SetFocus(GetDlgItem(hwnd, 1004));
                SendDlgItemMessageW(hwnd, 1004, EM_SETSEL, 0, -1);
                return TRUE;
            }
            sleep_on = IsDlgButtonChecked(hwnd, 1001) == BST_CHECKED;
            display_on = IsDlgButtonChecked(hwnd, 1002) == BST_CHECKED;
            mouse_on = IsDlgButtonChecked(hwnd, 1003) == BST_CHECKED;
            update_on = IsDlgButtonChecked(hwnd, 1008) == BST_CHECKED;
            paused = IsDlgButtonChecked(hwnd, 1005) == BST_CHECKED;
            interval_s = minutes * 60;
            last_attempt = GetTickCount(); mouse_status = AM_TEXT(L"대기 중");
            saved = save_settings(); inspect_saver(); update_power();
            want_startup = IsDlgButtonChecked(hwnd, 1011) == BST_CHECKED;
            if (want_startup || am_startup_registered()) startup_result = am_startup_set(want_startup);
            if (startup_result != ERROR_SUCCESS)
                CheckDlgButton(hwnd, 1011, am_startup_registered() ? BST_CHECKED : BST_UNCHECKED);
            up_tick(update_on && !paused, TRUE);
            SetDlgItemTextW(hwnd, 1009, up_status());
            update_tray();
            /* Update status is already visible in the dialog; never repeat
               an update/admin-status failure in a blocking message box. */
            if (saved && power_ok && startup_result == ERROR_SUCCESS) {
                SetDlgItemTextW(hwnd, IDOK, update_on && !paused && !up_ok() ? AM_TEXT(L"설정 저장됨") : AM_TEXT(L"적용 완료"));
            } else {
                WCHAR error[160];
                SetDlgItemTextW(hwnd, IDOK, AM_TEXT(L"적용"));
                if (startup_result != ERROR_SUCCESS) {
                    wsprintfW(error, AM_TEXT(L"시작 시 실행 설정을 저장하지 못했습니다. (오류 %lu)"), (DWORD)startup_result);
                    MessageBoxW(hwnd, error, APP, MB_OK | MB_ICONWARNING);
                } else {
                    MessageBoxW(hwnd, saved ? AM_TEXT(L"전원 유지 요청을 적용하지 못했습니다.") :
                        AM_TEXT(L"현재 실행에는 반영했지만 설정을 저장하지 못했습니다."), APP, MB_OK | MB_ICONWARNING);
                }
            }
            return TRUE;
        }
        case IDCANCEL:
            ShowWindow(hwnd, SW_HIDE); return TRUE;
        case 1010: up_open_settings(hwnd); return TRUE;
        case 1007: SendMessageW(window, WM_COMMAND, ID_EXIT, 0); return TRUE;
        }
        break;
    case WM_CLOSE:
        ShowWindow(hwnd, SW_HIDE); return TRUE;
    case WM_DESTROY: settings = NULL; return TRUE;
    }
    return FALSE;
}
static void show_settings(void)
{
    if (!settings) settings = CreateDialogParamW(instance, MAKEINTRESOURCEW(1000), window, dialog_proc, 0);
    if (settings) { ShowWindow(settings, SW_SHOWNORMAL); SetForegroundWindow(settings); }
}
static void cleanup(void)
{
    KillTimer(window, 1);
    am_keys_cleanup();
    am_blackout_cleanup();
    SetThreadExecutionState(ES_CONTINUOUS);
    WTSUnRegisterSessionNotification(window);
    if (tray_added) Shell_NotifyIconW(NIM_DELETE, &tray);
    tray_added = FALSE;
}
static LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    unsigned command;
    if (taskbar_created && msg == taskbar_created) { tray_added = FALSE; update_tray(); return 0; }
    switch (msg) {
    case AM_WIN_B_TOGGLE:
        available = desktop_available();
        if (blackout_ready && available) am_blackout_toggle(settings);
        update_power(); update_tray(); return 0;
    case AM_BLACKOUT_CHANGED:
        refresh_blackout_button(); update_power(); update_tray(); return 0;
    case WM_DISPLAYCHANGE:
        am_blackout_refresh(); return 0;
    case WM_TIMER: {
        BOOL before = available;
        available = desktop_available();
        if (before != available) am_keys_reset();
        if (!available) am_blackout_hide(FALSE);
        inspect_saver(); update_power(); try_mouse();
        up_tick(update_on && !paused, FALSE);
        if (settings && IsWindowVisible(settings)) SetDlgItemTextW(settings, 1009, up_status());
        update_tray(); return 0;
    }
    case WM_TRAY:
        if (lp == WM_RBUTTONUP || lp == WM_CONTEXTMENU) show_menu();
        else if (lp == WM_LBUTTONDBLCLK) show_settings();
        return 0;
    case WM_COMMAND:
        command = LOWORD(wp);
        if (command == ID_REHOOK) {
            if (blackout_ready) am_keys_start(instance, window);
            return 0;
        }
        if (command == ID_BLACKOUT) {
            available = desktop_available();
            if (available) am_blackout_toggle(settings);
            update_power(); update_tray(); return 0;
        }
        if (command == ID_UPDATE_SETTINGS) { up_open_settings(hwnd); return 0; }
        if (command == ID_SETTINGS) { show_settings(); return 0; }
        if (command == ID_EXIT) { DestroyWindow(hwnd); return 0; }
        if (command == ID_ABOUT) {
            MessageBoxW(hwnd,
                AM_TEXT(L"Awake Mini 1.3.3-winb1\n\n트레이 더블클릭: 설정 / 우클릭: 메뉴\n절전 방지 · 화면 유지 · 마우스 유휴 입력\n검은화면 버튼 / 트레이 메뉴: 실행 · 화면의 해제 버튼: 복귀\n검은 화면에서는 절전·화면 유지·기존 간격 마우스 입력 활성화\n\nWindows 시작 시 실행: 로그인 후 일반 권한으로 실행합니다.\n업데이트 연장에는 관리자 권한이 필요합니다.\nEXE 이동 후 자동 실행을 체크하고 적용하여 경로를 갱신하세요.\n\n업데이트 연장은 시험 기능입니다.\n24시간마다 오늘+7일, 최초 중지일부터 최대 35일을 적용합니다.\n더 짧은 기간 정책이 있으면 해당 상한을 따릅니다.\nOFF / 전체 일시정지 / 종료 시 남은 중지 기간을 유지합니다.\n실제 중지 여부는 Windows 설정에서 확인하세요.\n\n언어: Windows 표시 언어 자동 선택 (한국어 / 영어)."),
                APP, MB_OK | MB_ICONINFORMATION);
            return 0;
        }
        if (command == ID_SLEEP) sleep_on = !sleep_on;
        else if (command == ID_DISPLAY && sleep_on) display_on = !display_on;
        else if (command == ID_MOUSE) mouse_on = !mouse_on;
        else if (command == ID_PAUSE) paused = !paused;
        else if (command >= ID_INTERVAL && command < ID_INTERVAL + 4) interval_s = intervals[command - ID_INTERVAL];
        else return 0;
        last_attempt = GetTickCount(); mouse_status = AM_TEXT(L"대기 중");
        save_settings(); update_power(); up_tick(update_on && !paused, TRUE);
        update_tray(); refresh_settings(); return 0;
    case WM_WTSSESSION_CHANGE:
        am_keys_reset();
        if (wp == WTS_SESSION_LOCK || wp == WTS_CONSOLE_DISCONNECT || wp == WTS_REMOTE_DISCONNECT) locked = TRUE;
        if (wp == WTS_SESSION_UNLOCK || wp == WTS_CONSOLE_CONNECT || wp == WTS_REMOTE_CONNECT) locked = FALSE;
        available = desktop_available(); last_attempt = GetTickCount();
        if (!available) am_blackout_hide(FALSE);
        update_power(); update_tray(); return 0;
    case WM_POWERBROADCAST:
        am_keys_reset();
        if (wp == PBT_APMSUSPEND) {
            am_blackout_hide(FALSE);
            SetThreadExecutionState(ES_CONTINUOUS); applied = 0xffffffffu;
        } else if (wp == PBT_APMRESUMEAUTOMATIC || wp == PBT_APMRESUMESUSPEND) {
            last_attempt = GetTickCount(); available = desktop_available();
                applied = 0xffffffffu; update_power(); update_tray();
        }
        return TRUE;
    case WM_QUERYENDSESSION: return TRUE;
    case WM_ENDSESSION: if (wp) cleanup(); return 0;
    case WM_CLOSE: DestroyWindow(hwnd); return 0;
    case WM_DESTROY: cleanup(); PostQuitMessage(0); return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}
int WINAPI WinMain(HINSTANCE current, HINSTANCE previous, LPSTR arguments, int show)
{
    WNDCLASSW wc = {0};
    MSG msg;
    BOOL result;
    unsigned i;
    (void)previous; (void)arguments; (void)show;
    mutex = CreateMutexW(NULL, FALSE, L"Local\\AwakeMini.Singleton.v1");
    if (!mutex) return 1;
    if (GetLastError() == ERROR_ALREADY_EXISTS) { CloseHandle(mutex); return 0; }
    am_language_init();
    mouse_status = AM_TEXT(mouse_status);
    instance = current;
    SetProcessDPIAware();
    for (i = 0; i < 3; ++i) icons[i] = LoadIconW(instance, MAKEINTRESOURCEW(1 + i));
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = window_proc; wc.hInstance = instance; wc.lpszClassName = CLASS; wc.hIcon = icons[0];
    if (!RegisterClassW(&wc)) { CloseHandle(mutex); return 2; }
    window = CreateWindowExW(WS_EX_TOOLWINDOW, CLASS, APP, WS_POPUP, 0, 0, 0, 0, NULL, NULL, instance, NULL);
    if (!window) { CloseHandle(mutex); return 3; }
    blackout_ready = am_blackout_init(instance, window);
    taskbar_created = RegisterWindowMessageW(L"TaskbarCreated");
    load_settings();
    available = desktop_available();
    last_attempt = GetTickCount();
    tray.cbSize = sizeof(tray); tray.hWnd = window; tray.uID = 1;
    tray.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP; tray.uCallbackMessage = WM_TRAY;
    WTSRegisterSessionNotification(window, NOTIFY_FOR_THIS_SESSION);
    inspect_saver(); update_tray();
    if (!tray_added) {
        MessageBoxW(NULL, AM_TEXT(L"트레이 아이콘을 등록하지 못했습니다. 잠시 후 다시 실행해 주세요."), APP, MB_OK | MB_ICONERROR);
        DestroyWindow(window); CloseHandle(mutex); return 4;
    }
    if (!SetTimer(window, 1, 1000, NULL)) { DestroyWindow(window); CloseHandle(mutex); return 5; }
    if (blackout_ready) am_keys_start(instance, window);
    update_power();
    up_tick(update_on && !paused, TRUE);
    while ((result = GetMessageW(&msg, NULL, 0, 0)) > 0) {
        if (settings && IsWindowVisible(settings) && IsDialogMessageW(settings, &msg)) continue;
        TranslateMessage(&msg); DispatchMessageW(&msg);
    }
    if (result == -1) cleanup();
    CloseHandle(mutex);
    return result == -1 ? 6 : 0;
}
