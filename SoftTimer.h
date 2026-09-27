#ifndef SOFTTIMER_H
#define SOFTTIMER_H

#include <stdint.h>
#include <stdbool.h>

/* ==========================================================================
 *  SoftTimer - software timers driven by the LPIT 1 ms tick
 * ========================================================================== */

#define SOFTTIMER_MAX           4u

typedef void (*TimerCallback)(void);

void SoftTimer_Init(void);                                  /* also starts the LPIT tick */
bool SoftTimer_Start(uint8_t id, uint32_t timeout_ms, bool periodic, TimerCallback cb);
void SoftTimer_Stop(uint8_t id);
void SoftTimer_Process(void);                               /* call from the main loop   */

#endif /* SOFTTIMER_H */
