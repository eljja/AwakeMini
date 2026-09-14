#ifndef AM_UPDATE_PLAN_H
#define AM_UPDATE_PLAN_H
#include <stdint.h>
#define UP_DAY UINT64_C(864000000000)
/* FILETIME units; never shorten a pre-existing user pause. The 35-day
   ceiling is deliberately conservative even on newer Windows builds. */
static int up_plan(uint64_t now, uint64_t anchor, uint64_t current,
                   uint64_t last, unsigned max_days, uint64_t *end)
{
    uint64_t limit, target;
    if (!max_days || max_days > 35 || !anchor || now < anchor || anchor > UINT64_MAX - 35 * UP_DAY)
        return -1;
    limit = anchor + max_days * UP_DAY;
    if (now >= limit) return -2;
    if (last && (now < last || now - last < UP_DAY)) return 0;
    target = now + 7 * UP_DAY;
    if (target > limit) target = limit;
    if (current >= target) return 0;
    *end = target;
    return 1;
}
#endif
