#ifndef AM_BLACKOUT_KEYS_H
#define AM_BLACKOUT_KEYS_H
/* Pure key-state matcher. Fire once after Win+B has been released, allowing
   Explorer's normal Win+B handling to finish before showing the cover. */
typedef struct { unsigned char down[256]; int pending; } AM_BLACKOUT_KEYS;
static int am_blackout_key(AM_BLACKOUT_KEYS *s, unsigned key, int released)
{
    int repeat;
    if (key >= 256) return 0;
    repeat = s->down[key];
    s->down[key] = !released;
    if (!released && !repeat) {
        if (key == 'B' && (s->down[0x5b] || s->down[0x5c]) &&
            !s->down[0x10] && !s->down[0x11] && !s->down[0x12] &&
            !s->down[0xa0] && !s->down[0xa1] && !s->down[0xa2] &&
            !s->down[0xa3] && !s->down[0xa4] && !s->down[0xa5]) s->pending = 1;
        else if (key != 0x5b && key != 0x5c) s->pending = 0;
    }
    if (s->pending && !s->down['B'] && !s->down[0x5b] && !s->down[0x5c]) {
        s->pending = 0;
        return 1;
    }
    return 0;
}
#endif
