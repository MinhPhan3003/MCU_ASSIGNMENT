#include "SoftTimer.h"
#include "LPIT.h"
#include <stddef.h>

typedef struct {
    bool            active;
    bool            periodic;
    uint32_t        timeout_ms;
    uint32_t        start_ms;       /* tick value when the current period started */
    TimerCallback   cb;
} SoftTimer_t;

static SoftTimer_t s_timers[SOFTTIMER_MAX];


void SoftTimer_Init(void)
{
    LPIT_Tick_Init();
}

/* Returns false (and changes nothing) for an invalid id, a NULL callback
 * or a zero timeout. Restarting an active timer is allowed. */
bool SoftTimer_Start(uint8_t id, uint32_t timeout_ms, bool periodic, TimerCallback cb)
{
    if ((id >= SOFTTIMER_MAX) || (cb == NULL) || (timeout_ms == 0u)) {
        return false;
    }

    s_timers[id].periodic   = periodic;
    s_timers[id].timeout_ms = timeout_ms;
    s_timers[id].cb         = cb;
    s_timers[id].start_ms   = LPIT_GetTickMs();
    s_timers[id].active     = true;

    return true;
}

void SoftTimer_Stop(uint8_t id)
{   /* invalid id: ignored */
    if (id >= SOFTTIMER_MAX) {
        return;                         
    }
    s_timers[id].active = false;
}

void SoftTimer_Process(void)
{
    uint8_t       i;
    uint32_t      now = LPIT_GetTickMs();     /* one snapshot for this pass */
    TimerCallback cb;

    for (i = 0u; i < SOFTTIMER_MAX; i++) {
        if (!s_timers[i].active) {
            continue;
        }

        if ((uint32_t)(now - s_timers[i].start_ms) < s_timers[i].timeout_ms) {
            continue;                           /* not expired yet */
        }

        cb = s_timers[i].cb;

        if (s_timers[i].periodic) {
            /* Advance by exactly one period: no drift even if Process runs late */
            s_timers[i].start_ms += s_timers[i].timeout_ms;
        } else {
            /* One-shot: deactivate BEFORE the callback, so the callback may
             * restart this timer without being cancelled afterwards. */
            s_timers[i].active = false;
        }
        cb();
    }
}
