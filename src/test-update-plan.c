#include <assert.h>
#include <stdio.h>
#include "update-plan.h"
int main(void)
{
    uint64_t a = 100 * UP_DAY, e = 0;
    assert(up_plan(a, a, 0, 0, 35, &e) == 1 && e == a + 7 * UP_DAY);
    assert(up_plan(a + UP_DAY, a, e, a, 35, &e) == 1 && e == a + 8 * UP_DAY);
    assert(up_plan(a + UP_DAY - 1, a, e, a, 35, &e) == 0);
    assert(up_plan(a + 30 * UP_DAY, a, 0, 0, 35, &e) == 1 && e == a + 35 * UP_DAY);
    assert(up_plan(a + 35 * UP_DAY, a, 0, 0, 35, &e) == -2);
    assert(up_plan(a, a, a + 20 * UP_DAY, 0, 35, &e) == 0);
    assert(up_plan(a - 1, a, 0, 0, 35, &e) == -1);
    assert(up_plan(a, a, 0, a + UP_DAY, 35, &e) == 0);
    assert(up_plan(a, 0, 0, 0, 35, &e) == -1);
    assert(up_plan(UINT64_MAX, UINT64_MAX - UP_DAY, 0, 0, 35, &e) == -1);
    assert(up_plan(a, a, 0, 0, 3, &e) == 1 && e == a + 3 * UP_DAY);
    assert(up_plan(a + 3 * UP_DAY, a, 0, 0, 3, &e) == -2);
    assert(up_plan(a, a, 0, 0, 0, &e) == -1);
    assert(up_plan(a, a, 0, 0, 36, &e) == -1);
    puts("14 pause scheduling boundary checks passed");
    return 0;
}
