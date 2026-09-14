"""Run blackout lifecycle code with mocked Win32 calls (not a native UI test)."""
from pathlib import Path
import re, subprocess, tempfile
src=Path(__file__).with_name('blackout.c').read_text()
# Test the actual lifecycle functions; painting and registration require Windows.
core=src[src.index('static HWND'):src.index('static LRESULT CALLBACK')]
core+=src[src.index('void am_blackout_cleanup'):]
# Also execute the actual app power-selection function against the same state.
app=Path(__file__).with_name('awake-mini.c').read_text()
core+=app[app.index('static void update_power'):app.index('static void update_tray')]
core+=app[app.index('static LONG normalize'):app.index('static void show_menu')]
stub=r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <wchar.h>
#include "policy.h"
typedef wchar_t WCHAR; typedef long LONG; typedef long long LONGLONG; typedef unsigned UINT;
typedef int BOOL; typedef uintptr_t HWND; typedef uint32_t DWORD;
#define TRUE 1
#define FALSE 0
#undef NULL
#define NULL 0
#define SW_HIDE 0
#define IDC_ARROW 1
#define AM_BLACKOUT_CHANGED 2
#define AM_BLACKOUT_HOTKEY 110
#define SM_CXVIRTUALSCREEN 0
#define SM_CYVIRTUALSCREEN 1
#define SM_XVIRTUALSCREEN 2
#define SM_YVIRTUALSCREEN 3
#define HWND_TOPMOST 100
#define SWP_NOACTIVATE 1
#define SWP_SHOWWINDOW 2
#define MOUSEEVENTF_MOVE 1
#define MOUSEEVENTF_ABSOLUTE 2
#define MOUSEEVENTF_VIRTUALDESK 4
#define INPUT_MOUSE 0
#define AM_TEXT(x) (x)
typedef struct {UINT cbSize;DWORD dwTime;} LASTINPUTINFO;
typedef struct {LONG x,y;} POINT;
typedef struct {LONG left,top,right,bottom;} RECT;
typedef struct {LONG dx,dy;DWORD dwFlags;} MOUSEINPUT;
typedef struct {DWORD type;MOUSEINPUT mi;} INPUT;
static BOOL mouse_on,saver_blocked,held;
static DWORD interval_s=120,last_attempt,clock_now=120000,input_time;
static const WCHAR *mouse_status;
static UINT input_calls;
static DWORD GetTickCount(void){return clock_now;}
static BOOL GetLastInputInfo(LASTINPUTINFO *last){last->dwTime=input_time;return 1;}
static BOOL any_key_down(void){return held;}
static BOOL GetCursorPos(POINT *p){p->x=10;p->y=10;return 1;}
static BOOL GetClipCursor(RECT *r){r->left=-1920;r->top=-1080;r->right=3840;r->bottom=1080;return 1;}
static UINT SendInput(UINT n,INPUT *p,int size){
 assert(n==2 && size==sizeof(INPUT));assert(p[0].mi.dwFlags==7 && p[1].mi.dwFlags==7);
 assert(p[0].mi.dx!=p[1].mi.dx && p[0].mi.dy==p[1].mi.dy);
 ++input_calls;input_time=clock_now;return n;
}
#define ES_CONTINUOUS 0x80000000u
static int bounds[]={5760,2160,-1920,-1080},placed[4];
static int visible,fail_place,fail_focus,notified,unregistered,destroyed;
static HWND focused=90;
static DWORD power_flags;
static BOOL sleep_on,display_on,paused,available=TRUE,power_ok;
static DWORD applied=0xffffffffu;
static int ShowWindow(HWND h,int n){(void)h;visible=n;return 1;}
static int SetCursor(int c){return c;}
static int LoadCursorW(int h,int n){(void)h;return n;}
static int IsWindow(HWND h){return h!=0;}
static int SetForegroundWindow(HWND h){if(fail_focus)return 0;focused=h;return 1;}
static int PostMessageW(HWND h,int m,int a,int b){(void)h;(void)m;(void)a;(void)b;++notified;return 1;}
static int GetSystemMetrics(int i){return bounds[i];}
static int SetWindowPos(HWND h,HWND z,int x,int y,int w,int t,int flags){
 (void)h;assert(z==HWND_TOPMOST && flags==(SWP_NOACTIVATE|SWP_SHOWWINDOW));
 placed[0]=x;placed[1]=y;placed[2]=w;placed[3]=t;if(fail_place)return 0;visible=1;return 1;
}
static HWND GetForegroundWindow(void){return focused;}
static int UpdateWindow(HWND h){(void)h;return 1;}
static int UnregisterHotKey(HWND h,int i){(void)h;assert(i==110);++unregistered;return 1;}
static int DestroyWindow(HWND h){(void)h;++destroyed;return 1;}
static DWORD SetThreadExecutionState(DWORD flags){power_flags=flags;return 1;}
'''
cases=r'''
int main(void){
 cover=10; owner_window=11;
 assert(!am_blackout_toggle() && !active && !visible); /* missing hotkey */
 registered=1; paused=1;
 assert(am_blackout_toggle() && active && visible && focused==10);
 assert(placed[0]==-1920 && placed[1]==-1080 && placed[2]==5760 && placed[3]==2160);
 try_mouse();assert(input_calls==1 && active && visible && !mouse_on && paused);
 clock_now+=119999;try_mouse();assert(input_calls==1);
 ++clock_now;held=1;try_mouse();assert(input_calls==1);held=0;
 saver_blocked=1;try_mouse();assert(input_calls==1);saver_blocked=0;
 available=0;try_mouse();assert(input_calls==1);available=1;
 try_mouse();assert(input_calls==2 && active && last_attempt==clock_now);
 update_power();assert(power_ok && power_flags==(ES_CONTINUOUS|3));
 assert(paused==1 && sleep_on==0 && display_on==0); /* saved settings untouched */
 assert(am_blackout_toggle() && !active && !visible && focused==90);
 update_power();assert(power_flags==ES_CONTINUOUS);
 clock_now+=120000;try_mouse();assert(input_calls==2);
 fail_place=1;assert(!am_blackout_toggle() && !active && !visible);fail_place=0;
 fail_focus=1;assert(!am_blackout_toggle() && !active && !visible);fail_focus=0;
 bounds[0]=0;assert(!am_blackout_toggle() && !active);bounds[0]=1920;
 assert(am_blackout_toggle());bounds[2]=0;am_blackout_refresh();assert(placed[0]==0);
 available=0;update_power();assert(power_flags==ES_CONTINUOUS);
 focused=99;am_blackout_hide(FALSE);assert(!active && focused==99); /* lock/UAC */
 available=1;assert(am_blackout_toggle());bounds[1]=0;am_blackout_refresh();assert(!active);
 bounds[1]=1080;assert(am_blackout_toggle());am_blackout_cleanup();
 assert(!active && !registered && !cover && unregistered==1 && destroyed==1);
 am_blackout_cleanup();assert(unregistered==1 && destroyed==1);
 /* The same scheduling function retains idle delay, recent input, held-button,
    session and synthetic-input restrictions during blackout. */
 assert(am_due(120000,0,0,120000,1,0,1,0,0));
 assert(!am_due(119999,0,0,120000,1,0,1,0,0));
 assert(!am_due(120000,1,0,120000,1,0,1,0,0));
 assert(!am_due(120000,0,0,120000,1,0,1,1,0));
 assert(!am_due(120000,0,0,120000,1,0,0,0,0));
 assert(!am_due(120000,0,0,120000,1,0,1,0,1));
 puts("Blackout lifecycle, failure rollback, monitor bounds, power restoration and idle gates passed (mock Windows)");
}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d)/'test.c';p.write_text(stub+core+cases)
 exe=Path(d)/'test'
 subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror','-I',str(Path(__file__).parent),str(p),'-o',str(exe)],check=True)
 subprocess.run([str(exe)],check=True)
