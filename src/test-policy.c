#include <assert.h>
#include "policy.h"
int main(void)
{
    assert(am_power(1, 1, 0, 1) == 3);
    assert(am_power(1, 0, 0, 1) == 1);
    assert(am_power(0, 1, 0, 1) == 0);
    assert(am_power(1, 1, 1, 1) == 0);
    assert(am_power(1, 1, 0, 0) == 0);
    assert(am_due(60000, 0, 0, 60000, 1, 0, 1, 0, 0));
    assert(!am_due(59999, 0, 0, 60000, 1, 0, 1, 0, 0));
    assert(!am_due(60000, 1000, 0, 60000, 1, 0, 1, 0, 0));
    assert(!am_due(60000, 0, 1000, 60000, 1, 0, 1, 0, 0));
    assert(!am_due(60000, 0, 0, 60000, 0, 0, 1, 0, 0));
    assert(!am_due(60000, 0, 0, 60000, 1, 1, 1, 0, 0));
    assert(!am_due(60000, 0, 0, 60000, 1, 0, 0, 0, 0));
    assert(!am_due(60000, 0, 0, 60000, 1, 0, 1, 1, 0));
    assert(!am_due(60000, 0, 0, 60000, 1, 0, 1, 0, 1));
    assert(am_due(30000, UINT32_MAX-30000, UINT32_MAX-30000, 60000, 1, 0, 1, 0, 0));
    assert(!am_due(100, UINT32_MAX-100, UINT32_MAX-100, 60000, 1, 0, 1, 0, 0));
    assert(!am_due(60000, 60001, 0, 60000, 1, 0, 1, 0, 0));
    return 0;
}
