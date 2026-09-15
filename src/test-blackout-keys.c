#include <assert.h>
#include <string.h>
#include "blackout-keys.h"
int main(void)
{
    AM_BLACKOUT_KEYS s = {{0}, 0};
    unsigned win, release_win_first, modifier;
    for (win=0x5b;win<=0x5c;++win) for(release_win_first=0;release_win_first<2;++release_win_first) {
        memset(&s,0,sizeof(s));
        assert(!am_blackout_key(&s,win,0));
        assert(!am_blackout_key(&s,'B',0));
        assert(!am_blackout_key(&s,'B',0)); /* autorepeat */
        assert(!am_blackout_key(&s,release_win_first?win:'B',1));
        assert(am_blackout_key(&s,release_win_first?'B':win,1));
        assert(!am_blackout_key(&s,win,1));
    }
    for(modifier=0xa0;modifier<=0xa5;++modifier) {
        memset(&s,0,sizeof(s));
        am_blackout_key(&s,modifier,0);am_blackout_key(&s,0x5b,0);
        am_blackout_key(&s,'B',0);am_blackout_key(&s,'B',1);
        assert(!am_blackout_key(&s,0x5b,1));
    }
    memset(&s,0,sizeof(s));
    am_blackout_key(&s,'B',0);am_blackout_key(&s,0x5b,0);
    am_blackout_key(&s,'B',0);am_blackout_key(&s,'B',1);
    assert(!am_blackout_key(&s,0x5b,1)); /* held B before Win */
    memset(&s,0,sizeof(s));
    am_blackout_key(&s,0x5b,0);am_blackout_key(&s,'B',0);
    am_blackout_key(&s,'C',0);am_blackout_key(&s,'B',1);
    assert(!am_blackout_key(&s,0x5b,1)); /* chord cancelled */
    assert(!am_blackout_key(&s,256,0));
    memset(&s,0,sizeof(s));
    am_blackout_key(&s,0x5b,0);am_blackout_key(&s,0x5c,0);
    am_blackout_key(&s,'B',0);am_blackout_key(&s,'B',1);
    assert(!am_blackout_key(&s,0x5b,1));
    assert(am_blackout_key(&s,0x5c,1));
    return 0;
}
