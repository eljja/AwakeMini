"""Run the actual policy reader against a mock registry, without Windows writes."""
from pathlib import Path
import subprocess
import tempfile
source = Path(__file__).with_name('update-pause.c').read_text()
reader = source[source.index('static int optional_dword'):source.index('static const WCHAR *observed_pause_status')]
stub = r'''
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <stdint.h>
typedef uint32_t DWORD;
typedef long LONG;
typedef int BOOL;
typedef unsigned char BYTE;
typedef wchar_t WCHAR;
typedef int HKEY;
#define AM_TEXT(x) (x)
#define TRUE 1
#define FALSE 0
#define ERROR_SUCCESS 0
#define ERROR_FILE_NOT_FOUND 2
#define ERROR_PATH_NOT_FOUND 3
#define REG_DWORD 4
#define KEY_QUERY_VALUE 1
#define KEY_WOW64_64KEY 256
#define HKEY_LOCAL_MACHINE 0
#define wsprintfW(dst,fmt,...) swprintf(dst,200,fmt,__VA_ARGS__)
static WCHAR status[200];
struct Entry { int key; const WCHAR *name; DWORD value; DWORD type; } entries[12];
static int count, read_failure;
static LONG RegOpenKeyExW(HKEY root,const WCHAR *path,DWORD options,DWORD access,HKEY *key)
{ (void)root;(void)options;(void)access; *key=wcsstr(path,L"PolicyManager")?2:1;return read_failure?5:0; }
static LONG RegCloseKey(HKEY k) { (void)k;return 0; }
static LONG RegQueryValueExW(HKEY key,const WCHAR *name,void *unused,DWORD *type,BYTE *out,DWORD *size)
{
 (void)unused;
 for(int i=0;i<count;i++) if(entries[i].key==key && !wcscmp(entries[i].name,name)) {
   *type=entries[i].type;*size=sizeof(DWORD);memcpy(out,&entries[i].value,sizeof(DWORD));return 0;
 }
 return ERROR_FILE_NOT_FOUND;
}
static void add(int k,const WCHAR *n,DWORD v,DWORD t) {entries[count++]=(struct Entry){k,n,v,t};}
'''
cases = r'''
int main(void) {
 unsigned cap;
 assert(pause_access_allowed(&cap) && cap==35);
 add(1,L"SetActiveHoursEnd",18,REG_DWORD);
 add(1,L"WUServer",1,1);
 add(1,L"SetDisableUXWUAccess",1,REG_DWORD);
 add(2,L"ActiveHoursEnd_ProviderSet",1,REG_DWORD);
 assert(pause_access_allowed(&cap) && cap==35); /* Regression: unrelated policies allowed. */
 add(1,L"SetDisablePauseUXAccess",0,REG_DWORD);
 assert(pause_access_allowed(&cap));
 entries[count-1].value=1;
 assert(!pause_access_allowed(&cap));
 count=0; add(2,L"SetDisablePauseUXAccess",1,REG_DWORD);
 assert(!pause_access_allowed(&cap));
 count=0; add(1,L"SetMaxPauseDays",7,REG_DWORD);add(2,L"SetMaxPauseDays",3,REG_DWORD);
 assert(pause_access_allowed(&cap) && cap==3);
 entries[count-1].value=0;assert(!pause_access_allowed(&cap));
 entries[count-1].value=36;assert(!pause_access_allowed(&cap));
 count=0;add(1,L"SetDisablePauseUXAccess",0,1);assert(!pause_access_allowed(&cap));
 count=0;read_failure=1;assert(!pause_access_allowed(&cap));
 puts("10 policy gate regression scenarios passed (mock registry)");
}
'''
with tempfile.TemporaryDirectory() as d:
    p=Path(d)/'test.c';p.write_text(stub+reader+cases)
    executable=Path(d)/'test'
    subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror',str(p),'-o',str(executable)],check=True)
    subprocess.run([str(executable)],check=True)
