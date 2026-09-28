#include "SCG.h"
#include "GPIO.h"
#include "SoftTimer.h"

/* ==========================================================================
 *  Assignment 03 - LPIT 1 ms tick + software timers
 *    - LPIT0 CH0, SIRCDIV2 8 MHz (set by CLOCK_Init), TVAL = 7999, IRQ 48
 *    - LED1 (PTB10) : one-shot 500 ms
 *    - LED2 (PTB11) : periodic 1000 ms
 * ========================================================================== */

#define TIMER_ONE_SHOT      0u
#define TIMER_PERIODIC      1u

static void Led1_On_Once(void)     { LED_On(LED1_PIN); }             /* 500 ms, one-shot  */
static void Led2_Toggle(void)      { LED_Toggle(LED2_PIN); }         /* every 1000 ms     */

int main(void)
{
    CLOCK_Init();                       /* also SOSCDIV2 = 8 MHz for LPIT */

    LED_Init();
    SoftTimer_Init();

    SoftTimer_Start(TIMER_ONE_SHOT, 500u,  false, Led1_On_Once);
    SoftTimer_Start(TIMER_PERIODIC, 1000u, true,  Led2_Toggle);

    for (;;) {
        SoftTimer_Process();
    }
}
