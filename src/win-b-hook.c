#define UNICODE
#define _UNICODE
#define _WIN32_WINNT 0x0A00
#include <windows.h>
#include "win-b-hook.h"
#include "language.h"

static HANDLE worker, stop_event, ready_event;
static HINSTANCE module;
static HWND target;
static volatile LONG installed, error_code, captures, mask_failures, stopping, reset_requested;
/* Only accessed by the hook thread. No key history or text is retained. */
static BOOL swallowed_b, physical_b_down;

static void mask_start_menu(void)
{
    INPUT input[2] = {{0}};
    UINT sent;
    /* Win down was passed through. A short Ctrl pulse prevents its eventual
       release from looking like Win alone. Called only with Ctrl/Alt/Shift up.
       Our injected events are passed through by the hook, never interpreted. */
    input[0].type = input[1].type = INPUT_KEYBOARD;
    input[0].ki.wVk = input[1].ki.wVk = VK_LCONTROL;
    input[1].ki.dwFlags = KEYEVENTF_KEYUP;
    sent = SendInput(2, input, sizeof(INPUT));
    if (sent != 2) {
        InterlockedIncrement(&mask_failures);
        if (sent == 1) SendInput(1, &input[1], sizeof(INPUT));
    }
}
static LRESULT CALLBACK keyboard_proc(int code, WPARAM wp, LPARAM lp)
{
    const KBDLLHOOKSTRUCT *key;
    BOOL down, up;
    if (code != HC_ACTION) return CallNextHookEx(NULL, code, wp, lp);
    if (InterlockedExchange(&reset_requested, 0)) {
        swallowed_b = FALSE;
        physical_b_down = (GetAsyncKeyState('B') & 0x8000) != 0;
    }
    key = (const KBDLLHOOKSTRUCT *)lp;
    if ((key->flags & LLKHF_INJECTED) || key->vkCode != 'B')
        return CallNextHookEx(NULL, code, wp, lp);
    down = wp == WM_KEYDOWN || wp == WM_SYSKEYDOWN;
    up = wp == WM_KEYUP || wp == WM_SYSKEYUP;
    if (up) {
        BOOL consume = swallowed_b;
        swallowed_b = physical_b_down = FALSE;
        if (consume) return 1;
    } else if (down) {
        /* Track B locally: a swallowed down does not update async key state.
           Only query modifiers, whose preceding events we passed through. */
        if (!physical_b_down) {
            physical_b_down = TRUE;
            swallowed_b = FALSE;
            if (!InterlockedCompareExchange(&stopping, 0, 0) &&
                ((GetAsyncKeyState(VK_LWIN) | GetAsyncKeyState(VK_RWIN)) & 0x8000) &&
                !((GetAsyncKeyState(VK_CONTROL) | GetAsyncKeyState(VK_MENU) |
                   GetAsyncKeyState(VK_SHIFT)) & 0x8000) &&
                PostMessageW(target, AM_WIN_B_TOGGLE, 0, 0)) {
                swallowed_b = TRUE;
                InterlockedIncrement(&captures);
                mask_start_menu();
            }
        }
        /* Swallow both edges and repeats even if Win is released before B. */
        if (swallowed_b) return 1;
    }
    return CallNextHookEx(NULL, code, wp, lp);
}
static DWORD WINAPI keyboard_thread(LPVOID unused)
{
    HHOOK hook;
    MSG msg;
    DWORD wait;
    (void)unused;
    PeekMessageW(&msg, NULL, 0, 0, PM_NOREMOVE);
    physical_b_down = (GetAsyncKeyState('B') & 0x8000) != 0;
    hook = SetWindowsHookExW(WH_KEYBOARD_LL, keyboard_proc, module, 0);
    if (!hook) InterlockedExchange(&error_code, (LONG)GetLastError());
    else InterlockedExchange(&installed, 1);
    SetEvent(ready_event);
    if (!hook) return 0;
    for (;;) {
        wait = MsgWaitForMultipleObjects(1, &stop_event, FALSE, INFINITE, QS_ALLINPUT);
        if (wait == WAIT_OBJECT_0) break;
        if (wait == WAIT_FAILED) {
            InterlockedExchange(&error_code, (LONG)GetLastError()); break;
        }
        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) goto done;
            TranslateMessage(&msg); DispatchMessageW(&msg);
        }
    }
done:
    UnhookWindowsHookEx(hook);
    InterlockedExchange(&installed, 0);
    return 0;
}
void am_keys_reset(void)
{
    InterlockedExchange(&reset_requested, 1);
}
void am_keys_cleanup(void)
{
    InterlockedExchange(&stopping, 1);
    if (stop_event) SetEvent(stop_event);
    if (worker && WaitForSingleObject(worker, 2000) != WAIT_OBJECT_0) {
        /* Never terminate a callback thread or close handles it still uses. */
        InterlockedExchange(&error_code, WAIT_TIMEOUT); return;
    }
    if (worker) CloseHandle(worker);
    if (stop_event) CloseHandle(stop_event);
    if (ready_event) CloseHandle(ready_event);
    worker = stop_event = ready_event = NULL;
    InterlockedExchange(&installed, 0);
    swallowed_b = physical_b_down = FALSE;
}
void am_keys_start(HINSTANCE instance, HWND owner)
{
    am_keys_cleanup();
    if (worker) return;
    module = instance; target = owner;
    InterlockedExchange(&error_code, 0);
    InterlockedExchange(&stopping, 0);
    stop_event = CreateEventW(NULL, TRUE, FALSE, NULL);
    if (!stop_event) goto fail;
    ready_event = CreateEventW(NULL, TRUE, FALSE, NULL);
    if (!ready_event) goto fail;
    worker = CreateThread(NULL, 0, keyboard_thread, NULL, 0, NULL);
    if (!worker) goto fail;
    if (WaitForSingleObject(ready_event, 2000) != WAIT_OBJECT_0) {
        InterlockedExchange(&error_code, WAIT_TIMEOUT);
        am_keys_cleanup();
    }
    return;
fail:
    InterlockedExchange(&error_code, (LONG)GetLastError());
    am_keys_cleanup();
}
void am_keys_status(WCHAR *text)
{
    LONG error = InterlockedCompareExchange(&error_code, 0, 0);
    if (error) wsprintfW(text, AM_TEXT(L"Win+B: 훅 오류 %lu · 버튼 사용 가능"), (DWORD)error);
    else if (InterlockedCompareExchange(&installed, 0, 0))
        wsprintfW(text, AM_TEXT(L"Win+B: 훅 설치됨 · 감지 %lu · 보조 입력 실패 %lu"),
            (DWORD)InterlockedCompareExchange(&captures, 0, 0),
            (DWORD)InterlockedCompareExchange(&mask_failures, 0, 0));
    else lstrcpyW(text, AM_TEXT(L"Win+B: 훅 미설치 · 버튼 사용 가능"));
}
