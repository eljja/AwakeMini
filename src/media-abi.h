#ifndef AM_MEDIA_ABI_H
#define AM_MEDIA_ABI_H
/* Minimal C ABI prefixes for the documented Windows.Media interfaces missing
   from this MinGW SDK. Method order/IID checked against Microsoft's bindings:
   https://github.com/microsoft/windows-rs/blob/master/crates/libs/windows/src/Windows/Media/mod.rs
   Unused slots are never called. WinRT boolean is one byte, not Win32 BOOL. */
#include <inspectable.h>
#include <stddef.h>
typedef struct AmMedia AmMedia;
typedef struct AmUpdater AmUpdater;
typedef struct AmVideo AmVideo;
typedef void (STDMETHODCALLTYPE *AmUnused)(void);
typedef struct {
    IInspectableVtbl base;
    HRESULT (STDMETHODCALLTYPE *get_status)(AmMedia *, INT32 *);
    HRESULT (STDMETHODCALLTYPE *put_status)(AmMedia *, INT32);
    HRESULT (STDMETHODCALLTYPE *get_updater)(AmMedia *, AmUpdater **);
    AmUnused sound_level;
    HRESULT (STDMETHODCALLTYPE *get_enabled)(AmMedia *, BYTE *);
    HRESULT (STDMETHODCALLTYPE *put_enabled)(AmMedia *, BYTE);
} AmMediaVtbl;
struct AmMedia { const AmMediaVtbl *lpVtbl; };
typedef struct {
    IInspectableVtbl base;
    AmUnused get_type;
    HRESULT (STDMETHODCALLTYPE *put_type)(AmUpdater *, INT32);
    AmUnused get_id, put_id, get_thumbnail, put_thumbnail, get_music;
    HRESULT (STDMETHODCALLTYPE *get_video)(AmUpdater *, AmVideo **);
    AmUnused get_image, copy_file;
    HRESULT (STDMETHODCALLTYPE *clear)(AmUpdater *);
    HRESULT (STDMETHODCALLTYPE *update)(AmUpdater *);
} AmUpdaterVtbl;
struct AmUpdater { const AmUpdaterVtbl *lpVtbl; };
typedef struct {
    IInspectableVtbl base;
    AmUnused get_title;
    HRESULT (STDMETHODCALLTYPE *put_title)(AmVideo *, HSTRING);
} AmVideoVtbl;
struct AmVideo { const AmVideoVtbl *lpVtbl; };
static const IID AM_IID_SMTC = {0x99fa3ff4,0x1742,0x42a6,{0x90,0x2e,0x08,0x7d,0x41,0xf9,0x65,0xec}};
static const IID AM_IID_SMTC_INTEROP = {0xddb0472d,0xc911,0x4a1f,{0x86,0xd9,0xdc,0x3d,0x71,0xa9,0x5f,0x5a}};
_Static_assert(offsetof(AmMediaVtbl, put_status) == 7 * sizeof(AmUnused), "SMTC status ABI");
_Static_assert(offsetof(AmMediaVtbl, put_enabled) == 11 * sizeof(AmUnused), "SMTC enabled ABI");
_Static_assert(offsetof(AmUpdaterVtbl, get_video) == 13 * sizeof(AmUnused), "SMTC video ABI");
_Static_assert(offsetof(AmUpdaterVtbl, update) == 17 * sizeof(AmUnused), "SMTC update ABI");
_Static_assert(offsetof(AmVideoVtbl, put_title) == 7 * sizeof(AmUnused), "Video title ABI");
#endif
