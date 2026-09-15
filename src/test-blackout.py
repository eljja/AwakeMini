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
#include "blackout-keys.h"
#include <string.h>
typedef wchar_t WCHAR; typedef long LONG; typedef long long LONGLONG; typedef unsigned UINT;
typedef intptr_t LPARAM; typedef uintptr_t HRAWINPUT;
typedef int BOOL; typedef uintptr_t HWND; typedef uint32_t DWORD;
#define TRUE 1
#define FALSE 0
#undef NULL
#define NULL 0
#define SW_HIDE 0
#define IDC_ARROW 1
#define AM_BLACKOUT_CHANGED 2
#define AM_BLACKOUT_TOGGLE 3
#define ZeroMemory(p,n) memset(p,0,n)
#define RID_INPUT 1
#define RI_KEY_BREAK 1
#define RIM_TYPEKEYBOARD 1
#define RIDEV_REMOVE 1
#define GWL_EXSTYLE 1
#define WS_EX_TOPMOST 8
#define HWND_NOTOPMOST 101
#define SWP_NOMOVE 4
#define SWP_NOSIZE 8
typedef struct {unsigned short usUsagePage,usUsage;DWORD dwFlags;HWND hwndTarget;} RAWINPUTDEVICE;
typedef struct {DWORD dwType,dwSize;uintptr_t hDevice,wParam;} RAWINPUTHEADER;
typedef struct {unsigned short MakeCode,Flags,Reserved,VKey;UINT Message;DWORD ExtraInformation;} RAWKEYBOARD;
typedef struct {RAWINPUTHEADER header;struct {RAWKEYBOARD keyboard;} data;} RAWINPUT;
static RAWINPUT test_raw;
static BOOL raw_fail;
static UINT raw_size=sizeof(RAWINPUT);
static UINT GetRawInputData(HRAWINPUT h,UINT cmd,void *out,UINT *bytes,UINT header){
 (void)h;assert(cmd==RID_INPUT && header==sizeof(RAWINPUTHEADER));
 if(raw_fail)return (UINT)-1;
 memcpy(out,&test_raw,sizeof(test_raw));*bytes=raw_size;return raw_size;
}
static short GetAsyncKeyState(int key){(void)key;return 0;}
static LONG panel_style;
static LONG GetWindowLongPtrW(HWND h,int index){assert(h==20 && index==GWL_EXSTYLE);return panel_style;}

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
static int visible,fail_place,fail_focus,notified,unregistered,destroyed,key_toggles;
static HWND focused=90;
static DWORD power_flags;
static BOOL sleep_on,display_on,paused,available=TRUE,power_ok;
static DWORD applied=0xffffffffu;
static int ShowWindow(HWND h,int n){(void)h;visible=n;return 1;}
static int SetCursor(int c){return c;}
static int LoadCursorW(int h,int n){(void)h;return n;}
static int IsWindow(HWND h){return h!=0;}
static int SetForegroundWindow(HWND h){if(fail_focus)return 0;focused=h;return 1;}
static int PostMessageW(HWND h,int m,int a,int b){(void)h;(void)a;(void)b;++notified;if(m==AM_BLACKOUT_TOGGLE)++key_toggles;return 1;}
static int GetSystemMetrics(int i){return bounds[i];}
static int SetWindowPos(HWND h,HWND z,int x,int y,int w,int t,int flags){
 if(h==20){assert((flags&(SWP_NOMOVE|SWP_NOSIZE))==(SWP_NOMOVE|SWP_NOSIZE));
 panel_style=z==HWND_TOPMOST?WS_EX_TOPMOST:0;return 1;}
 assert(z==HWND_TOPMOST && flags==(SWP_NOACTIVATE|SWP_SHOWWINDOW));
 placed[0]=x;placed[1]=y;placed[2]=w;placed[3]=t;if(fail_place)return 0;visible=1;return 1;
}
static HWND GetForegroundWindow(void){return focused;}
static int UpdateWindow(HWND h){(void)h;return 1;}
static int RegisterRawInputDevices(RAWINPUTDEVICE *r,UINT n,UINT sz){assert(n==1 && sz==sizeof(*r) && r->dwFlags==RIDEV_REMOVE && !r->hwndTarget);++unregistered;return 1;}
static int DestroyWindow(HWND h){(void)h;++destroyed;return 1;}
static DWORD SetThreadExecutionState(DWORD flags){power_flags=flags;return 1;}
'''
cases=r'''
int main(void){
 cover=10; owner_window=11;
 test_raw.header.dwType=RIM_TYPEKEYBOARD;
 am_blackout_input_reset();
 test_raw.data.keyboard.VKey=0x5b;am_blackout_raw_input(1);
 test_raw.data.keyboard.VKey='B';am_blackout_raw_input(1);am_blackout_raw_input(1);
 test_raw.data.keyboard.Flags=RI_KEY_BREAK;am_blackout_raw_input(1);assert(!key_toggles);
 test_raw.data.keyboard.VKey=0x5b;am_blackout_raw_input(1);assert(key_toggles==1);
 am_blackout_raw_input(1);assert(key_toggles==1);
 raw_fail=1;am_blackout_raw_input(1);raw_fail=0;
 raw_size=0;am_blackout_raw_input(1);raw_size=sizeof(test_raw);
 assert(key_toggles==1);

 assert(!am_blackout_toggle(NULL) && !active && !visible); /* missing hotkey */
 assert(am_blackout_toggle(20) && active && visible && focused==20 && am_blackout_testing());
 assert(panel_style==WS_EX_TOPMOST);
 assert(am_blackout_toggle(20) && !active && !panel_style && focused==90);
 panel_style=WS_EX_TOPMOST;
 assert(am_blackout_toggle(20));am_blackout_hide(TRUE);assert(panel_style==WS_EX_TOPMOST);
 panel_style=0;
 registered=1; paused=1;
 assert(am_blackout_toggle(NULL) && active && visible && focused==10);
 assert(placed[0]==-1920 && placed[1]==-1080 && placed[2]==5760 && placed[3]==2160);
 try_mouse();assert(input_calls==1 && active && visible && !mouse_on && paused);
 clock_now+=119999;try_mouse();assert(input_calls==1);
 ++clock_now;held=1;try_mouse();assert(input_calls==1);held=0;
 saver_blocked=1;try_mouse();assert(input_calls==1);saver_blocked=0;
 available=0;try_mouse();assert(input_calls==1);available=1;
 try_mouse();assert(input_calls==2 && active && last_attempt==clock_now);
 update_power();assert(power_ok && power_flags==(ES_CONTINUOUS|3));
 assert(paused==1 && sleep_on==0 && display_on==0); /* saved settings untouched */
 assert(am_blackout_toggle(NULL) && !active && !visible && focused==90);
 update_power();assert(power_flags==ES_CONTINUOUS);
 clock_now+=120000;try_mouse();assert(input_calls==2);
 fail_place=1;assert(!am_blackout_toggle(NULL) && !active && !visible);fail_place=0;
 fail_focus=1;assert(am_blackout_toggle(NULL) && active && visible);am_blackout_hide(TRUE);fail_focus=0;
 bounds[0]=0;assert(!am_blackout_toggle(NULL) && !active);bounds[0]=1920;
 assert(am_blackout_toggle(NULL));bounds[2]=0;am_blackout_refresh();assert(placed[0]==0);
 available=0;update_power();assert(power_flags==ES_CONTINUOUS);
 focused=99;am_blackout_hide(FALSE);assert(!active && focused==99); /* lock/UAC */
 available=1;assert(am_blackout_toggle(NULL));bounds[1]=0;am_blackout_refresh();assert(!active);
 bounds[1]=1080;assert(am_blackout_toggle(NULL));am_blackout_cleanup();
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
 puts("Blackout button, focus-failure handling, raw key routing, lifecycle, monitor bounds, power and mouse gates passed (mock Windows)");
}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d)/'test.c';p.write_text(stub+core+cases)
 exe=Path(d)/'test'
 subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror','-I',str(Path(__file__).parent),str(p),'-o',str(exe)],check=True)
 subprocess.run([str(exe)],check=True)
