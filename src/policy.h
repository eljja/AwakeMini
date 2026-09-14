#ifndef AWAKE_MINI_POLICY_H
#define AWAKE_MINI_POLICY_H
#include <stdint.h>

/* Unsigned subtraction deliberately handles the GetTickCount 49.7-day wrap. */
static int am_due(uint32_t now, uint32_t last_input, uint32_t last_attempt,
                  uint32_t interval, int enabled, int paused,
                  int available, int button_down, int blocked)
{
    uint32_t idle = (uint32_t)(now - last_input);
    return enabled && !paused && available && !button_down && !blocked &&
           idle < 0x80000000u && idle >= interval &&
           (uint32_t)(now - last_attempt) >= interval;
}
static unsigned am_power(int sleep, int display, int paused, int available)
{
    if (paused || !available || !sleep) return 0;
    return 1u | (display ? 2u : 0u);
}
#endif
