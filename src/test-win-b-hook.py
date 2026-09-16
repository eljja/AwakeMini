"""Compile the actual hook module against deterministic Win32 mocks (no native input)."""
from pathlib import Path
import subprocess, tempfile, os
root=Path(__file__).resolve().parent
source=(root/'win-b-hook.c').read_text()
source='\n'.join(line for line in source.splitlines() if not line.startswith('#include'))
stub=r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
typedef int BOOL; typedef uint32_t DWORD,UINT; typedef int32_t LONG;
typedef uintptr_t WPARAM; typedef intptr_t LPARAM,LRESULT;
typedef void *HANDLE,*HINSTANCE,*HWND,*HHOOK,*LPVOID;
typedef wchar_t WCHAR;
#define TRUE 1
#define FALSE 0
#define CALLBACK
#define WINAPI
#define HC_ACTION 0
#define LLKHF_INJECTED 16
#define VK_LCONTROL 162
#define VK_LWIN 91
#define VK_RWIN 92
#define VK_CONTROL 17
#define VK_MENU 18
#define VK_SHIFT 16
#define INPUT_KEYBOARD 1
#define KEYEVENTF_KEYUP 2
#define WM_KEYDOWN 256
#define WM_KEYUP 257
#define WM_SYSKEYDOWN 260
#define WM_SYSKEYUP 261
#define WM_QUIT 18
#define AM_WIN_B_TOGGLE 32771
#define PM_NOREMOVE 0
#define PM_REMOVE 1
#define WH_KEYBOARD_LL 13
#define INFINITE 0xffffffffu
#define QS_ALLINPUT 1279
#define WAIT_OBJECT_0 0
#define WAIT_TIMEOUT 258
#define WAIT_FAILED 0xffffffffu
#define AM_TEXT(x) x
#define wsprintfW(buffer,fmt,...) swprintf(buffer,160,fmt,__VA_ARGS__)
#define lstrcpyW wcscpy
typedef struct {DWORD vkCode,flags;} KBDLLHOOKSTRUCT;
typedef struct {unsigned short wVk; DWORD dwFlags;} KEYBDINPUT;
typedef struct {DWORD type;KEYBDINPUT ki;} INPUT;
typedef struct {UINT message;} MSG;
static unsigned short keys[256];
static unsigned posted,passed,input_calls,unhooked,closed,events;
static int post_ok=1,fail_event,fail_thread,hook_ok=1;
static UINT sent_count=2;
static DWORD ready_wait,worker_wait,loop_wait=WAIT_OBJECT_0;
static HWND expected_target=(HWND)3;
static short GetAsyncKeyState(int key){return (short)keys[key];}
static LONG InterlockedExchange(volatile LONG *p,LONG v){LONG old=*p;*p=v;return old;}
static LONG InterlockedIncrement(volatile LONG *p){return ++*p;}
static LONG InterlockedCompareExchange(volatile LONG *p,LONG v,LONG cmp){LONG old=*p;if(old==cmp)*p=v;return old;}
static LRESULT CallNextHookEx(HHOOK h,int c,WPARAM w,LPARAM l){(void)h;(void)c;(void)w;(void)l;++passed;return 42;}
static BOOL PostMessageW(HWND h,UINT m,WPARAM w,LPARAM l){assert(h==expected_target&&m==AM_WIN_B_TOGGLE&&!w&&!l);if(post_ok)++posted;return post_ok;}
static UINT SendInput(UINT n,INPUT *p,int size){assert(size==sizeof(INPUT));assert(p[0].type==INPUT_KEYBOARD&&p[0].ki.wVk==VK_LCONTROL);++input_calls;if(n==2){assert(!p[0].ki.dwFlags&&p[1].ki.dwFlags==KEYEVENTF_KEYUP);return sent_count;}assert(n==1&&p[0].ki.dwFlags==KEYEVENTF_KEYUP);return 1;}
static BOOL PeekMessageW(MSG *m,HWND h,UINT a,UINT b,UINT flags){(void)m;(void)h;(void)a;(void)b;(void)flags;return 0;}
static DWORD GetLastError(void){return 5;}
static HHOOK SetWindowsHookExW(int id,LRESULT (*fn)(int,WPARAM,LPARAM),HINSTANCE inst,DWORD tid){assert(id==WH_KEYBOARD_LL&&fn&&inst==(HINSTANCE)2&&tid==0);return hook_ok?(HHOOK)9:NULL;}
static BOOL UnhookWindowsHookEx(HHOOK h){assert(h==(HHOOK)9);++unhooked;return 1;}
static BOOL SetEvent(HANDLE h){assert(h);return 1;}
static DWORD MsgWaitForMultipleObjects(DWORD n,HANDLE *h,BOOL all,DWORD ms,DWORD mask){assert(n==1&&*h&&!all&&ms==INFINITE&&mask==QS_ALLINPUT);return loop_wait;}
static void TranslateMessage(MSG *m){(void)m;}
static void DispatchMessageW(MSG *m){(void)m;}
static BOOL CloseHandle(HANDLE h){assert(h);++closed;return 1;}
static HANDLE CreateEventW(void *a,BOOL manual,BOOL initial,void *name){assert(!a&&manual&&!initial&&!name);++events;if((int)events==fail_event)return NULL;return (HANDLE)(uintptr_t)(events==1?11:12);}
static HANDLE CreateThread(void *a,UINT size,DWORD (*fn)(LPVOID),LPVOID arg,DWORD flags,DWORD *tid){assert(!a&&!size&&fn&&!arg&&!flags&&!tid);return fail_thread?NULL:(HANDLE)13;}
static DWORD WaitForSingleObject(HANDLE h,DWORD ms){assert(ms==2000);return h==(HANDLE)13?worker_wait:ready_wait;}
'''
tests=r'''
static LRESULT keyevent(int vk,int down,DWORD flags){KBDLLHOOKSTRUCT k={(DWORD)vk,flags};return keyboard_proc(HC_ACTION,down?WM_KEYDOWN:WM_KEYUP,(LPARAM)&k);}
static void reset(void){worker=stop_event=ready_event=NULL;module=(HINSTANCE)2;target=expected_target;installed=error_code=captures=mask_failures=stopping=reset_requested=0;physical_b_down=swallowed_b=0;memset(keys,0,sizeof(keys));posted=passed=input_calls=unhooked=closed=events=0;post_ok=hook_ok=1;fail_event=fail_thread=0;sent_count=2;ready_wait=worker_wait=0;loop_wait=0;}
int main(void){
 reset();assert(keyboard_proc(-1,0,0)==42);assert(keyevent('A',1,0)==42);
 assert(keyevent('B',1,0)==42);keys[VK_LWIN]=0x8000;
 assert(keyevent('B',1,0)==42); /* B before Win stays ordinary B, including repeats. */
 assert(keyevent('B',0,0)==42);
 assert(keyevent('B',1,0)==1);assert(posted==1&&input_calls==1);
 for(int i=0;i<100;++i)assert(keyevent('B',1,0)==1);
 assert(posted==1);keys[VK_LWIN]=0;assert(keyevent('B',0,0)==1); /* Win released first. */
 assert(keyevent('B',1,0)==42);assert(keyevent('B',0,0)==42);
 keys[VK_RWIN]=0x8000;assert(keyevent('B',1,0)==1);assert(keyevent('B',0,0)==1);
 assert(keyevent('B',1,0)==1);assert(keyevent('B',0,0)==1);assert(posted==3); /* Win stays down. */
 for(int modifier=VK_CONTROL;modifier<=VK_MENU;++modifier){keys[modifier]=0x8000;assert(keyevent('B',1,0)==42);assert(keyevent('B',0,0)==42);keys[modifier]=0;}
 keys[VK_SHIFT]=0x8000;assert(keyevent('B',1,0)==42);assert(keyevent('B',0,0)==42);keys[VK_SHIFT]=0;
 assert(keyevent('B',1,LLKHF_INJECTED)==42);assert(keyevent('B',0,LLKHF_INJECTED)==42);assert(posted==3);
 post_ok=0;assert(keyevent('B',1,0)==42);assert(keyevent('B',0,0)==42);assert(input_calls==3);post_ok=1;
 sent_count=1;assert(keyevent('B',1,0)==1);assert(input_calls==5&&mask_failures==1);assert(keyevent('B',0,0)==1);
 sent_count=0;assert(keyevent('B',1,0)==1);assert(input_calls==6&&mask_failures==2);assert(keyevent('B',0,0)==1);
 stopping=1;assert(keyevent('B',1,0)==42);assert(keyevent('B',0,0)==42);stopping=0;
 sent_count=2;assert(keyevent('B',1,0)==1);am_keys_reset(); /* session change with missed release */
 assert(keyevent('A',1,0)==42);assert(!swallowed_b&&!physical_b_down);
 assert(keyevent('B',1,0)==1);assert(keyevent('B',0,0)==1);
 KBDLLHOOKSTRUCT k={'B',0};assert(keyboard_proc(0,WM_SYSKEYDOWN,(LPARAM)&k)==1);assert(keyboard_proc(0,WM_SYSKEYUP,(LPARAM)&k)==1);
 reset();am_keys_start((HINSTANCE)2,expected_target);assert(worker&&stop_event&&ready_event);am_keys_cleanup();assert(!worker&&closed==3);am_keys_cleanup();assert(closed==3);
 for(int stage=1;stage<=3;++stage){reset();fail_event=stage;fail_thread=stage==3;am_keys_start((HINSTANCE)2,expected_target);assert(error_code==5&&!worker&&!stop_event&&!ready_event);assert(closed==(unsigned)(stage-1));}
 reset();ready_wait=WAIT_TIMEOUT;am_keys_start((HINSTANCE)2,expected_target);assert(error_code==WAIT_TIMEOUT&&!worker&&closed==3);
 reset();am_keys_start((HINSTANCE)2,expected_target);worker_wait=WAIT_TIMEOUT;am_keys_cleanup();assert(worker&&closed==0&&error_code==WAIT_TIMEOUT);am_keys_start((HINSTANCE)2,expected_target);assert(worker&&events==2);worker_wait=0;am_keys_cleanup();assert(!worker&&closed==3);
 reset();stop_event=(HANDLE)11;ready_event=(HANDLE)12;keyboard_thread(NULL);assert(unhooked==1&&!installed&&!error_code);
 reset();stop_event=(HANDLE)11;ready_event=(HANDLE)12;hook_ok=0;keyboard_thread(NULL);assert(!unhooked&&!installed&&error_code==5);
 reset();stop_event=(HANDLE)11;ready_event=(HANDLE)12;loop_wait=WAIT_FAILED;keyboard_thread(NULL);assert(unhooked==1&&!installed&&error_code==5);
 puts("Actual Win+B hook: suppression, repeat, modifier/release order, injection, queue/mask failures, reset, thread startup/cleanup failures passed (mock Win32).");
}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.c').write_text(stub+source+tests)
 subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror','-fsanitize=address,undefined',str(p/'test.c'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0'})
