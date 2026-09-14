#define UNICODE
#define _UNICODE
#include <windows.h>
#include "startup.h"

/* Per-user, ordinary-privilege logon startup. Only explicit Apply changes
   this entry. Never alter StartupApproved or bypass startup restrictions. */
#define RUN_KEY L"Software\\Microsoft\\Windows\\CurrentVersion\\Run"
#define RUN_VALUE L"AwakeMini"
BOOL am_startup_registered(void)
{
    HKEY key;
    DWORD type, size = 0;
    LONG r = RegOpenKeyExW(HKEY_CURRENT_USER, RUN_KEY, 0,
                           KEY_QUERY_VALUE | KEY_WOW64_64KEY, &key);
    if (r != ERROR_SUCCESS) return FALSE;
    r = RegQueryValueExW(key, RUN_VALUE, NULL, &type, NULL, &size);
    RegCloseKey(key);
    return r == ERROR_SUCCESS && (type == REG_SZ || type == REG_EXPAND_SZ) && size > sizeof(WCHAR);
}
LONG am_startup_set(BOOL enabled)
{
    HKEY key;
    LONG r;
    WCHAR path[261], command[263];
    DWORD length;
    if (!enabled) {
        r = RegOpenKeyExW(HKEY_CURRENT_USER, RUN_KEY, 0,
                          KEY_SET_VALUE | KEY_WOW64_64KEY, &key);
        if (r == ERROR_FILE_NOT_FOUND || r == ERROR_PATH_NOT_FOUND) return ERROR_SUCCESS;
        if (r != ERROR_SUCCESS) return r;
        r = RegDeleteValueW(key, RUN_VALUE);
        RegCloseKey(key);
        return r == ERROR_FILE_NOT_FOUND ? ERROR_SUCCESS : r;
    }
    length = GetModuleFileNameW(NULL, path, 261);
    if (!length) return (LONG)GetLastError();
    /* Run command strings have a documented 260-character maximum. */
    if (length > 258) return ERROR_FILENAME_EXCED_RANGE;
    command[0] = L'"';
    CopyMemory(command + 1, path, length * sizeof(WCHAR));
    command[length + 1] = L'"';
    command[length + 2] = 0;
    r = RegCreateKeyExW(HKEY_CURRENT_USER, RUN_KEY, 0, NULL, 0,
        KEY_SET_VALUE | KEY_WOW64_64KEY, NULL, &key, NULL);
    if (r != ERROR_SUCCESS) return r;
    r = RegSetValueExW(key, RUN_VALUE, 0, REG_SZ, (const BYTE *)command,
                       (length + 3) * sizeof(WCHAR));
    RegCloseKey(key);
    return r;
}
