#define UNICODE
#define _UNICODE
#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include <shellapi.h>
#include <ktmw32.h>
#include <stdint.h>
#include "update-pause.h"
#include "language.h"
#include "update-plan.h"

/* EXPERIMENTAL: UX registry values are not a documented Windows Update API.
   A committed transaction proves only that dates were stored, not that the
   update orchestrator accepted a pause. Do not report reboot protection.
   We do not modify policies, services, tasks, permissions or pause limits. */
#define UX L"SOFTWARE\\Microsoft\\WindowsUpdate\\UX\\Settings"
#define OWN L"SOFTWARE\\AwakeMini\\UpdatePauseV1"
static const WCHAR *date_names[] = {
    L"PauseUpdatesStartTime", L"PauseUpdatesExpiryTime",
    L"PauseFeatureUpdatesStartTime", L"PauseFeatureUpdatesEndTime",
    L"PauseQualityUpdatesStartTime", L"PauseQualityUpdatesEndTime"
};
static WCHAR status[200];
static BOOL okay = TRUE, previous_enabled, suspended;
static ULONGLONG next_check;

typedef struct { BOOL exists; WCHAR text[64]; uint64_t time; } DATE_VALUE;
static uint64_t ft_number(FILETIME f)
{
    return ((uint64_t)f.dwHighDateTime << 32) | f.dwLowDateTime;
}
static uint64_t utc_now(void)
{
    FILETIME f;
    GetSystemTimeAsFileTime(&f);
    return ft_number(f);
}
static BOOL parse_time(const WCHAR *s, uint64_t *out)
{
    SYSTEMTIME t = {0};
    FILETIME f;
    unsigned v[6] = {0}, i, j, pos = 0;
    const unsigned sizes[] = {4,2,2,2,2,2};
    const WCHAR separators[] = L"--T::Z";
    if (lstrlenW(s) != 20) return FALSE;
    for (i = 0; i < 6; ++i) {
        for (j = 0; j < sizes[i]; ++j) {
            if (s[pos] < L'0' || s[pos] > L'9') return FALSE;
            v[i] = v[i] * 10 + (unsigned)(s[pos++] - L'0');
        }
        if (s[pos++] != separators[i]) return FALSE;
    }
    t.wYear = (WORD)v[0]; t.wMonth = (WORD)v[1]; t.wDay = (WORD)v[2];
    t.wHour = (WORD)v[3]; t.wMinute = (WORD)v[4]; t.wSecond = (WORD)v[5];
    if (!SystemTimeToFileTime(&t, &f)) return FALSE;
    *out = ft_number(f);
    return TRUE;
}
static void format_time(uint64_t n, WCHAR *s)
{
    FILETIME f = {(DWORD)n, (DWORD)(n >> 32)};
    SYSTEMTIME t;
    FileTimeToSystemTime(&f, &t);
    wsprintfW(s, L"%04u-%02u-%02uT%02u:%02u:%02uZ", t.wYear, t.wMonth,
        t.wDay, t.wHour, t.wMinute, t.wSecond);
}
static BOOL read_date(HKEY key, const WCHAR *name, DATE_VALUE *d)
{
    DWORD type, size = sizeof(d->text);
    LONG r;
    ZeroMemory(d, sizeof(*d));
    r = RegQueryValueExW(key, name, NULL, &type, (BYTE *)d->text, &size);
    if (r == ERROR_FILE_NOT_FOUND) return TRUE;
    if (r != ERROR_SUCCESS || type != REG_SZ || size < sizeof(WCHAR) ||
        size > sizeof(d->text) || size % sizeof(WCHAR) ||
        d->text[size / sizeof(WCHAR) - 1] != 0) return FALSE;
    if (!d->text[0]) return TRUE;
    d->exists = TRUE;
    return parse_time(d->text, &d->time);
}
static BOOL read_qword(HKEY k, const WCHAR *name, uint64_t *v)
{
    DWORD type, n = sizeof(*v);
    LONG r;
    *v = 0;
    r = RegQueryValueExW(k, name, NULL, &type, (BYTE *)v, &n);
    return r == ERROR_FILE_NOT_FOUND ||
        (r == ERROR_SUCCESS && type == REG_QWORD && n == sizeof(*v));
}
static LONG put_qword(HKEY k, const WCHAR *name, uint64_t v)
{
    return RegSetValueExW(k, name, 0, REG_QWORD, (BYTE *)&v, sizeof(v));
}
static LONG put_date(HKEY k, const WCHAR *name, const WCHAR *s)
{
    return RegSetValueExW(k, name, 0, REG_SZ, (const BYTE *)s,
                         (lstrlenW(s) + 1) * sizeof(WCHAR));
}
static BOOL key_present(const WCHAR *path)
{
    HKEY k;
    LONG r = RegOpenKeyExW(HKEY_LOCAL_MACHINE, path, 0,
                           KEY_QUERY_VALUE | KEY_WOW64_64KEY, &k);
    if (r == ERROR_SUCCESS) RegCloseKey(k);
    return r != ERROR_FILE_NOT_FOUND && r != ERROR_PATH_NOT_FOUND;
}
/* Reads only the named policy; unrelated WU policies and agent presence
   are not evidence that the user's Pause updates action is prohibited. */
static int optional_dword(HKEY key, const WCHAR *name, DWORD *value)
{
    DWORD type, size = sizeof(*value);
    LONG r;
    *value = 0;
    r = RegQueryValueExW(key, name, NULL, &type, (BYTE *)value, &size);
    if (r == ERROR_FILE_NOT_FOUND) return 0;
    if (r != ERROR_SUCCESS || type != REG_DWORD || size != sizeof(*value)) return -1;
    return 1;
}
static BOOL pause_access_allowed(unsigned *max_days)
{
    static const WCHAR *paths[] = {
        L"SOFTWARE\\Policies\\Microsoft\\Windows\\WindowsUpdate",
        L"SOFTWARE\\Microsoft\\PolicyManager\\current\\device\\Update"
    };
    static const WCHAR *sources[] = {L"GPO", L"MDM"};
    unsigned i;
    *max_days = 35;
    for (i = 0; i < 2; ++i) {
        HKEY key;
        DWORD value;
        int found;
        LONG r = RegOpenKeyExW(HKEY_LOCAL_MACHINE, paths[i], 0,
                               KEY_QUERY_VALUE | KEY_WOW64_64KEY, &key);
        if (r == ERROR_FILE_NOT_FOUND || r == ERROR_PATH_NOT_FOUND) continue;
        if (r != ERROR_SUCCESS) {
            wsprintfW(status, AM_TEXT(L"%s 중지 정책 읽기 실패 (%lu)"), sources[i], (DWORD)r);
            return FALSE;
        }
        found = optional_dword(key, L"SetDisablePauseUXAccess", &value);
        if (found < 0 || (found && value > 1)) {
            RegCloseKey(key);
            wsprintfW(status, AM_TEXT(L"%s 중지 정책 값 확인 필요\nSetDisablePauseUXAccess"), sources[i]);
            return FALSE;
        }
        if (found && value == 1) {
            RegCloseKey(key);
            wsprintfW(status, AM_TEXT(L"%s 사용자 일시중지 금지\nSetDisablePauseUXAccess = 1"), sources[i]);
            return FALSE;
        }
        found = optional_dword(key, L"SetMaxPauseDays", &value);
        RegCloseKey(key);
        if (found < 0 || (found && (value < 1 || value > 35))) {
            wsprintfW(status, AM_TEXT(L"%s 최대 중지 기간 확인 필요\nSetMaxPauseDays"), sources[i]);
            return FALSE;
        }
        if (found && value < *max_days) *max_days = value;
    }
    return TRUE;
}
static const WCHAR *observed_pause_status(void)
{
    HKEY key;
    DWORD quality, feature;
    int q, f;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\WindowsUpdate\\UpdatePolicy\\Settings", 0,
        KEY_QUERY_VALUE | KEY_WOW64_64KEY, &key) != ERROR_SUCCESS)
        return AM_TEXT(L"OS 중지 상태 미확인 · 업데이트 설정 확인");
    q = optional_dword(key, L"PausedQualityStatus", &quality);
    f = optional_dword(key, L"PausedFeatureStatus", &feature);
    RegCloseKey(key);
    if (q == 1 && f == 1 && quality == 1 && feature == 1)
        return AM_TEXT(L"OS 상태값: 품질·기능 업데이트 중지");
    if (q == 1 && quality == 1) return AM_TEXT(L"OS 상태값: 품질 중지 · 기능 확인 필요");
    if (f == 1 && feature == 1) return AM_TEXT(L"OS 상태값: 기능 중지 · 품질 확인 필요");
    return AM_TEXT(L"OS 중지 상태 미확인 · 업데이트 설정 확인");
}
static BOOL admin(void)
{
    SID_IDENTIFIER_AUTHORITY nt = SECURITY_NT_AUTHORITY;
    PSID sid = NULL;
    BOOL member = FALSE;
    if (AllocateAndInitializeSid(&nt, 2, SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS, 0,0,0,0,0,0, &sid)) {
        CheckTokenMembership(NULL, sid, &member);
        FreeSid(sid);
    }
    return member;
}
static BOOL supported_os(void)
{
    HKEY k;
    WCHAR build[32];
    DWORD type, size = sizeof(build), n = 0, i;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0,
        KEY_QUERY_VALUE | KEY_WOW64_64KEY, &k) != ERROR_SUCCESS) return FALSE;
    ZeroMemory(build, sizeof(build));
    if (RegQueryValueExW(k, L"CurrentBuildNumber", NULL, &type,
        (BYTE *)build, &size) != ERROR_SUCCESS || type != REG_SZ) {
        RegCloseKey(k); return FALSE;
    }
    RegCloseKey(k);
    build[31] = 0;
    for (i = 0; build[i]; ++i) {
        if (build[i] < L'0' || build[i] > L'9' || n > 100000) return FALSE;
        n = n * 10 + build[i] - L'0';
    }
    return n >= 18362;
}
static void date_status(uint64_t n)
{
    FILETIME utc = {(DWORD)n, (DWORD)(n >> 32)}, local;
    SYSTEMTIME t;
    if (!FileTimeToLocalFileTime(&utc, &local)) local = utc;
    FileTimeToSystemTime(&local, &t);
    wsprintfW(status, AM_TEXT(L"기록 기한 %04u-%02u-%02u %02u:%02u\n%s"),
              t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, observed_pause_status());
}
const WCHAR *up_status(void) { return status; }
BOOL up_ok(void) { return okay; }
void up_open_settings(HWND owner)
{
    ShellExecuteW(owner, L"open", L"ms-settings:windowsupdate", NULL, NULL, SW_SHOWNORMAL);
}
void up_tick(BOOL enabled, BOOL force)
{
    HANDLE tx = INVALID_HANDLE_VALUE;
    HKEY ux = NULL, own = NULL;
    DATE_VALUE dates[6], expected;
    uint64_t now, anchor = 0, last = 0, end = 0, earliest = 0, current = 0;
    WCHAR stamp[64], start[64];
    LONG error = ERROR_SUCCESS;
    int plan;
    unsigned i, max_days;
    BOOL changed = FALSE;
    ULONGLONG tick = GetTickCount64();
    if (!enabled) {
        previous_enabled = FALSE; suspended = FALSE;
        okay = TRUE; next_check = 0;
        lstrcpyW(status, AM_TEXT(L"OFF · 남은 일시중지 기간은 유지"));
        return;
    }
    if (!previous_enabled) { force = TRUE; suspended = FALSE; }
    previous_enabled = TRUE;
    if (suspended || (!force && tick < next_check)) return;
    next_check = tick + 60000;
    okay = FALSE;
    if (!supported_os()) { lstrcpyW(status, AM_TEXT(L"Windows 10 1903 이상 / Windows 11 필요")); return; }
    if (!admin()) { lstrcpyW(status, AM_TEXT(L"미적용 · 종료 후 관리자 권한으로 실행")); return; }
    if (!pause_access_allowed(&max_days)) return;
    if (key_present(L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\WindowsUpdate\\Auto Update\\RebootRequired") ||
        key_present(L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Component Based Servicing\\RebootPending")) {
        lstrcpyW(status, AM_TEXT(L"재부팅 대기 · 설정 앱에서 중지 필요")); return;
    }
    /* One atomic local registry transaction: all six dates and the renewal
       anchor commit together, or none do. No service stop/start is needed. */
    tx = CreateTransaction(NULL, NULL, 0, 0, 0, 0, L"Awake Mini pause dates");
    if (tx == INVALID_HANDLE_VALUE) { error = GetLastError(); goto fail; }
    error = RegOpenKeyTransactedW(HKEY_LOCAL_MACHINE, UX, 0,
        KEY_QUERY_VALUE | KEY_SET_VALUE | KEY_WOW64_64KEY, &ux, tx, NULL);
    if (error != ERROR_SUCCESS) goto fail;
    error = RegCreateKeyTransactedW(HKEY_LOCAL_MACHINE, OWN, 0, NULL, 0,
        KEY_QUERY_VALUE | KEY_SET_VALUE | KEY_WOW64_64KEY, NULL, &own, NULL, tx, NULL);
    if (error != ERROR_SUCCESS) goto fail;
    now = utc_now();
    now -= now % UINT64_C(10000000); /* Stored dates have whole-second precision. */
    for (i = 0; i < 6; ++i) {
        if (!read_date(ux, date_names[i], &dates[i])) goto unknown;
        if (!(i % 2) && dates[i].exists) {
            if (dates[i].time > now) goto unknown;
            if (!earliest || dates[i].time < earliest) earliest = dates[i].time;
        }
        if ((i % 2) && dates[i].time > current) current = dates[i].time;
    }
    if (!read_qword(own, L"Anchor", &anchor) || !read_qword(own, L"LastRenewal", &last)) goto unknown;
    /* Stop on any external edit (including Resume updates). Only a new
       pause explicitly started in Windows Settings can start a new cycle. */
    for (i = 0; i < 6; ++i) {
        if (!read_date(own, date_names[i], &expected)) goto unknown;
        if (anchor && (dates[i].exists != expected.exists ||
            (dates[i].exists && lstrcmpW(dates[i].text, expected.text)))) changed = TRUE;
    }
    if (changed && !(earliest && earliest > anchor)) {
        suspended = TRUE;
        lstrcpyW(status, AM_TEXT(L"외부 날짜 변경 감지 · 자동 갱신 중단")); goto done;
    }
    /* A new pause explicitly started in Settings may start a new cycle.
       Simply restarting or toggling this app cannot reset the anchor. */
    if (earliest && (!anchor || earliest > anchor)) { anchor = earliest; last = 0; }
    if (!anchor) anchor = earliest ? earliest : now;
    if (earliest && earliest < anchor) anchor = earliest;
    plan = up_plan(now, anchor, current, last, max_days, &end);
    if (plan == -2) {
        wsprintfW(status, AM_TEXT(L"%u일 상한 · 업데이트 설정 확인 필요"), max_days); goto done;
    }
    if (plan < 0) goto unknown;
    /* All update classes must have coherent dates. Don't invent semantics
       for a partially configured existing pause or shorten a longer one. */
    if (current > now) {
        for (i = 0; i < 6; i += 2)
            if (!dates[i].exists || !dates[i+1].exists || dates[i+1].time <= now ||
                dates[i+1].time < dates[i].time || dates[i+1].time != current) goto unknown;
    }
    if (!plan) { date_status(current); okay = TRUE; goto done; }
    format_time(end, stamp);
    format_time(anchor, start);
    for (i = 0; i < 6; ++i) {
        const WCHAR *value = i % 2 ? stamp : (dates[i].exists ? dates[i].text : start);
        error = put_date(ux, date_names[i], value);
        if (error != ERROR_SUCCESS) goto fail;
        error = put_date(own, date_names[i], value);
        if (error != ERROR_SUCCESS) goto fail;
    }
    error = put_qword(own, L"Anchor", anchor);
    if (error != ERROR_SUCCESS) goto fail;
    error = put_qword(own, L"LastRenewal", now);
    if (error != ERROR_SUCCESS) goto fail;
    RegCloseKey(ux); ux = NULL; RegCloseKey(own); own = NULL;
    if (!CommitTransaction(tx)) { error = GetLastError(); goto fail; }
    date_status(end); okay = TRUE;
    goto done;
unknown:
    lstrcpyW(status, AM_TEXT(L"미적용 · 기존 날짜 확인 필요 (Windows 설정)"));
    goto done;
fail:
    wsprintfW(status, AM_TEXT(L"미적용 · 설정 기록 실패 (오류 %lu)"), (DWORD)error);
done:
    if (ux) RegCloseKey(ux);
    if (own) RegCloseKey(own);
    if (tx != INVALID_HANDLE_VALUE) { RollbackTransaction(tx); CloseHandle(tx); }
}
