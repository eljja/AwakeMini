"""Exercise the actual startup registration code with mocked Windows calls."""
from pathlib import Path
import re, subprocess, tempfile
source=Path(__file__).with_name('startup.c').read_text()
source=re.sub(r'^#include.*\n','',source,flags=re.M)
stub=r'''
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>
#include <stdio.h>
typedef int BOOL;typedef int HKEY;typedef wchar_t WCHAR;typedef uint32_t DWORD;
typedef long LONG;typedef unsigned char BYTE;
#define TRUE 1
#define FALSE 0
#define HKEY_CURRENT_USER 1
#define KEY_QUERY_VALUE 1
#define KEY_SET_VALUE 2
#define KEY_WOW64_64KEY 256
#define REG_SZ 1
#define REG_EXPAND_SZ 2
#define ERROR_SUCCESS 0
#define ERROR_FILE_NOT_FOUND 2
#define ERROR_PATH_NOT_FOUND 3
#define ERROR_FILENAME_EXCED_RANGE 206
#define CopyMemory memcpy
static WCHAR path[300]=L"C:\\My Tools\\Awake Mini.exe", stored[263];
static BOOL exists;static int fail_write;
static DWORD GetModuleFileNameW(void *module,WCHAR *out,DWORD size)
{(void)module;size_t n=wcslen(path);wcsncpy(out,path,size);return n>=size?size:(DWORD)n;}
static DWORD GetLastError(void){return 5;}
static LONG RegOpenKeyExW(HKEY root,const WCHAR *name,DWORD opts,DWORD access,HKEY *out)
{(void)root;(void)name;(void)opts;assert(access&KEY_WOW64_64KEY);*out=1;return 0;}
static LONG RegCreateKeyExW(HKEY root,const WCHAR *name,DWORD a,void*b,DWORD c,DWORD access,void*d,HKEY*out,void*e)
{(void)root;(void)name;(void)a;(void)b;(void)c;(void)d;(void)e;assert(access&KEY_WOW64_64KEY);*out=1;return fail_write?5:0;}
static LONG RegCloseKey(HKEY key){(void)key;return 0;}
static LONG RegQueryValueExW(HKEY k,const WCHAR*n,void*u,DWORD*t,BYTE*out,DWORD*size)
{(void)k;(void)u;(void)out;assert(!wcscmp(n,L"AwakeMini"));if(!exists)return 2;*t=REG_SZ;*size=(wcslen(stored)+1)*sizeof(WCHAR);return 0;}
static LONG RegSetValueExW(HKEY k,const WCHAR*n,DWORD u,DWORD t,const BYTE*data,DWORD size)
{(void)k;(void)u;assert(!wcscmp(n,L"AwakeMini")&&t==REG_SZ&&size<=sizeof(stored));memcpy(stored,data,size);exists=1;return 0;}
static LONG RegDeleteValueW(HKEY k,const WCHAR*n)
{(void)k;assert(!wcscmp(n,L"AwakeMini"));if(!exists)return 2;exists=0;return 0;}
'''
cases=r'''
int main(void){
 assert(!am_startup_registered());
 assert(am_startup_set(TRUE)==0 && am_startup_registered());
 assert(!wcscmp(stored,L"\"C:\\My Tools\\Awake Mini.exe\""));
 wcscpy(path,L"C:\\New Folder\\AwakeMini-en.exe");
 assert(am_startup_set(TRUE)==0 && !wcscmp(stored,L"\"C:\\New Folder\\AwakeMini-en.exe\""));
 fail_write=1;assert(am_startup_set(TRUE)==5 && exists);fail_write=0;
 for(int i=0;i<259;i++)path[i]=L'a';
 path[259]=0;assert(am_startup_set(TRUE)==206 && exists);
 assert(am_startup_set(FALSE)==0 && !am_startup_registered());
 assert(am_startup_set(FALSE)==0);
 puts("Startup registration: quoting, replacement, limits, failure, removal passed (mock Windows)");
}
'''
with tempfile.TemporaryDirectory() as d:
 p=Path(d)/'test.c';p.write_text(stub+source+cases)
 exe=Path(d)/'test'
 subprocess.run(['cc','-std=c11','-Wall','-Wextra','-Werror',str(p),'-o',str(exe)],check=True)
 subprocess.run([str(exe)],check=True)
