"""Run the actual state-test lifecycle against mock Win32/WinRT interfaces.
The SMTC ABI prefix comes from media-abi.h; no Windows runtime is emulated.
"""
from pathlib import Path
import os, subprocess, tempfile
root=Path(__file__).resolve().parent
src=(root/'state-test.c').read_text()
abi=(root/'media-abi.h').read_text().replace('#include <inspectable.h>','')
media_code=src[src.index('static HRESULT make_string'):src.index('static HRESULT taskbar_connect')]
full_code=src[src.index('static HRESULT taskbar_connect'):src.index('static const WCHAR *query_name')]
full_code+=src[src.index('void am_test_bind'):src.index('void am_test_tick')]
tick_code=src[src.index('void am_test_tick'):src.index('void am_test_init')]
stub=r'''
#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
typedef int32_t HRESULT,INT32;typedef uint32_t UINT32,DWORD;typedef unsigned char BYTE;
typedef int BOOL;typedef wchar_t WCHAR;typedef const WCHAR *HSTRING;typedef uintptr_t HWND;
typedef struct {uint32_t a;uint16_t b,c;uint8_t d[8];} IID;
typedef struct {void (*slots[6])(void);} IInspectableVtbl;
typedef void IInspectable;
typedef struct {int n;} ISystemMediaTransportControlsInterop,ITaskbarList2;
#define STDMETHODCALLTYPE
#undef NULL
#define NULL 0
#define TRUE 1
#define FALSE 0
#define S_OK 0
#define E_FAIL ((HRESULT)0x80004005u)
#define E_HANDLE ((HRESULT)0x80070006u)
#define E_UNEXPECTED ((HRESULT)0x8000ffffu)
#define ERROR_CANCELLED 1223
#define HRESULT_FROM_WIN32(x) ((HRESULT)(0x80070000u|(x)))
#define FAILED(x) ((x)<0)
#define SUCCEEDED(x) ((x)>=0)
#define CLSCTX_INPROC_SERVER 1
#define SW_HIDE 0
#define SW_SHOWNORMAL 1
#define AM_TEXT(x) (x)
static const IID CLSID_TaskbarList={1,0,0,{0}},IID_ITaskbarList2={2,0,0,{0}};
static HWND app_owner=1,settings_dialog=5;
static BOOL signal_on,media_on,runtime_ready=1,can_run=1;
static HRESULT signal_result=S_OK,runtime_result=S_OK,media_result=S_OK;
static ITaskbarList2 *taskbar;
static INT32 media_status;
static BYTE media_enabled;
static unsigned step,fail_step,refs,strings,updates,closes,disables,clears,marks,unmarks;
static int fail_mark,fail_unmark,fail_cocreate,fail_hrinit,bad_readback;
static int dialog_visible=1,refresh_calls;
static unsigned dialog_destroyed, signal_marks, signal_unmarks;
static HRESULT call(void){++step;return step==fail_step?E_FAIL:S_OK;}
static void refresh(void){++refresh_calls;}
static int lstrlenW(const WCHAR *s){return (int)wcslen(s);}
static HRESULT WindowsCreateString(const WCHAR *s,UINT32 n,HSTRING *p){HRESULT h=call();assert(n==wcslen(s));if(SUCCEEDED(h)){*p=s;++strings;}return h;}
static HRESULT WindowsDeleteString(HSTRING p){if(p){assert(strings);--strings;}return S_OK;}
'''
api=r'''
static AmMedia *media;
static AmUpdater *updater;
static AmMedia media_obj;
static AmUpdater updater_obj;
static AmVideo video_obj;
static ISystemMediaTransportControlsInterop factory_obj;
static ITaskbarList2 taskbar_obj;
static HRESULT RoGetActivationFactory(HSTRING n,const IID *id,void **p){HRESULT h=call();assert(n&&id==&AM_IID_SMTC_INTEROP);if(SUCCEEDED(h)){*p=&factory_obj;++refs;}return h;}
static HRESULT ISystemMediaTransportControlsInterop_GetForWindow(ISystemMediaTransportControlsInterop *f,HWND w,const IID *id,void **p){HRESULT h=call();assert(f==&factory_obj&&w==app_owner&&id==&AM_IID_SMTC);if(SUCCEEDED(h)){*p=&media_obj;++refs;}return h;}
static unsigned IInspectable_Release(IInspectable *p){assert(p&&refs);--refs;return 0;}
#define ISystemMediaTransportControlsInterop_Release(p) IInspectable_Release(p)
static HRESULT get_updater(AmMedia *p,AmUpdater **v){HRESULT h=call();assert(p==&media_obj);if(SUCCEEDED(h)){*v=&updater_obj;++refs;}return h;}
static HRESULT put_type(AmUpdater *p,INT32 t){assert(p==&updater_obj&&t==2);return call();}
static HRESULT get_video(AmUpdater *p,AmVideo **v){HRESULT h=call();assert(p==&updater_obj);if(SUCCEEDED(h)){*v=&video_obj;++refs;}return h;}
static HRESULT put_title(AmVideo *p,HSTRING s){assert(p==&video_obj&&s);return call();}
static HRESULT update(AmUpdater *p){assert(p==&updater_obj);++updates;return call();}
static HRESULT put_enabled(AmMedia *p,BYTE v){assert(p==&media_obj);if(!v)++disables;return call();}
static HRESULT put_status(AmMedia *p,INT32 v){assert(p==&media_obj&&(v==0||v==3));if(!v)++closes;return call();}
static HRESULT get_status(AmMedia *p,INT32 *v){assert(p==&media_obj);*v=bad_readback?4:3;return call();}
static HRESULT get_enabled(AmMedia *p,BYTE *v){assert(p==&media_obj);*v=1;return call();}
static HRESULT clear(AmUpdater *p){assert(p==&updater_obj);++clears;return call();}
static const AmMediaVtbl mv={.get_updater=get_updater,.put_enabled=put_enabled,.put_status=put_status,.get_status=get_status,.get_enabled=get_enabled};
static const AmUpdaterVtbl uv={.put_type=put_type,.get_video=get_video,.update=update,.clear=clear};
static const AmVideoVtbl vv={.put_title=put_title};
static BOOL IsWindow(HWND w){return w!=0;}
static BOOL IsWindowVisible(HWND w){assert(w==settings_dialog);return dialog_visible;}
static BOOL DestroyWindow(HWND w){assert(w==settings_dialog);++dialog_destroyed;settings_dialog=0;signal_on=0;dialog_visible=0;return 1;}
static HRESULT CoCreateInstance(const IID *c,void *outer,int ctx,const IID *id,void **p){assert(c==&CLSID_TaskbarList&&!outer&&ctx==1&&id==&IID_ITaskbarList2);if(fail_cocreate)return E_FAIL;*p=&taskbar_obj;++refs;return S_OK;}
static HRESULT ITaskbarList2_HrInit(ITaskbarList2 *p){assert(p==&taskbar_obj);return fail_hrinit?E_FAIL:S_OK;}
#define ITaskbarList2_Release(p) IInspectable_Release(p)
static HRESULT ITaskbarList2_MarkFullscreenWindow(ITaskbarList2 *p,HWND w,BOOL on){assert(p==&taskbar_obj&&w==settings_dialog);if(w==settings_dialog){if(on)++signal_marks;else ++signal_unmarks;}if(on){++marks;return fail_mark?E_FAIL:S_OK;}++unmarks;return fail_unmark?E_FAIL:S_OK;}
'''
tests=r'''
static void reset(void){assert(!refs&&!strings);media_obj.lpVtbl=&mv;updater_obj.lpVtbl=&uv;video_obj.lpVtbl=&vv;step=fail_step=updates=closes=disables=clears=marks=unmarks=0;fail_mark=fail_unmark=fail_cocreate=fail_hrinit=bad_readback=0;runtime_ready=can_run=1;runtime_result=media_result=S_OK;dialog_visible=1;refresh_calls=0;media_on=0;media=NULL;updater=NULL;taskbar=NULL;settings_dialog=5;signal_on=0;signal_result=S_OK;dialog_destroyed=signal_marks=signal_unmarks=0;}
static void free_taskbar(void){if(taskbar){ITaskbarList2_Release(taskbar);taskbar=NULL;}}
int main(void){
 reset();am_test_media_toggle();assert(media_on&&media_enabled&&media_status==3&&refs==2&&!strings&&updates==1);unsigned steps=step;assert(steps==13);am_test_media_toggle();assert(!media_on&&!refs&&!strings&&closes==1&&disables==1&&clears==1);
 for(unsigned n=1;n<=steps;++n){reset();fail_step=n;am_test_media_toggle();assert(!media_on&&FAILED(media_result)&&!refs&&!strings&&!media&&!updater);}
 reset();bad_readback=1;am_test_media_toggle();assert(!media_on&&media_result==E_FAIL&&!refs&&!strings);
 reset();runtime_ready=0;runtime_result=E_UNEXPECTED;am_test_media_toggle();assert(media_result==E_UNEXPECTED&&!step);
 reset();can_run=0;am_test_media_toggle();assert(FAILED(media_result)&&!step);
 reset();am_test_media_toggle();fail_step=step+1;am_test_media_toggle();assert(!media_on&&!refs&&FAILED(media_result)&&disables==1&&clears==1); /* stop failure still releases */
 reset();am_test_signal_toggle();assert(signal_on&&signal_marks==1);am_test_signal_toggle();assert(!signal_on&&signal_unmarks==1&&!dialog_destroyed);free_taskbar();
 reset();am_test_media_toggle();am_test_signal_toggle();assert(media_on&&signal_on);am_test_signal_toggle();assert(media_on&&!signal_on);am_test_media_toggle();free_taskbar();
 reset();am_test_signal_toggle();dialog_visible=0;am_test_tick(TRUE);assert(signal_on&&!dialog_visible);am_test_bind(settings_dialog);assert(signal_on);am_test_tick(FALSE);assert(!signal_on&&!media_on);am_test_tick(TRUE);assert(!signal_on);free_taskbar();
 reset();am_test_signal_toggle();am_test_bind(6);assert(!signal_on&&signal_unmarks==1&&settings_dialog==6);free_taskbar();
 reset();am_test_signal_toggle();am_test_bind(0);assert(!signal_on&&signal_unmarks==1&&!settings_dialog);free_taskbar();
 reset();can_run=0;am_test_signal_toggle();assert(!signal_on&&!signal_marks&&FAILED(signal_result));
 reset();fail_mark=1;am_test_signal_toggle();assert(!signal_on&&FAILED(signal_result));free_taskbar();
 reset();fail_cocreate=1;am_test_signal_toggle();assert(!signal_on&&FAILED(signal_result)&&!refs);
 reset();fail_hrinit=1;am_test_signal_toggle();assert(!signal_on&&FAILED(signal_result)&&!refs);
 reset();settings_dialog=0;am_test_signal_toggle();assert(!signal_on&&signal_result==E_HANDLE&&!signal_marks);free_taskbar();
 reset();am_test_signal_toggle();fail_unmark=1;am_test_signal_toggle();assert(!signal_on&&FAILED(signal_result)&&dialog_destroyed==1&&!settings_dialog);free_taskbar();
 reset();am_test_signal_toggle();am_test_shell_changed();assert(signal_on&&signal_marks==2&&refs==1);am_test_signal_toggle();free_taskbar();
 reset();am_test_signal_toggle();fail_mark=1;am_test_shell_changed();assert(!signal_on&&FAILED(signal_result));free_taskbar();
 reset();am_test_media_toggle();am_test_signal_toggle();am_test_tick(FALSE);assert(!media_on&&!signal_on&&!can_run);am_test_tick(TRUE);assert(!media_on&&!signal_on);free_taskbar();
 reset();am_test_media_toggle();bad_readback=1;am_test_tick(TRUE);assert(!media_on&&!refs&&FAILED(media_result));
 puts("Main-dialog signal binding, hidden retention, independent media, shell reconnect, failure cleanup, pause/session cleanup and no auto-resume passed (mock Win32/WinRT).");
 puts("Actual media setup: 13 failure stages, ABI prefixes and readback passed.");
}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d);(p/'test.c').write_text(stub+abi+api+media_code+full_code+tick_code+tests)
 subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror','-fsanitize=address,undefined',str(p/'test.c'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=0'})
