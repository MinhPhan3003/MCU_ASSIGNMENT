#include "SCG.h"
#include "GPIO.h"
#include "SoftTimer.h"

/* ==========================================================================
 *  Assignment 03 - LPIT 1 ms tick + software timers
 *    - LPIT0 CH0, SIRCDIV2 8 MHz (set by CLOCK_Init), TVAL = 7999, IRQ 48
 *    - Red LED   : one-shot 500 ms
 *    - Green LED : periodic 1000 ms
 * ========================================================================== */

#define TIMER_ONE_SHOT      0u
#define TIMER_PERIODIC      1u

static void Red_On_Once(void)      { LED_On(LED_RED_PIN); }          /* 500 ms, one-shot  */
static void Green_Toggle(void)     { LED_Toggle(LED_GREEN_PIN); }    /* every 1000 ms     */

int main(void)
{
    CLOCK_Init();                       /* also SIRCDIV2 = 8 MHz for LPIT */
    CLOCK_EnterHSRUN();

    LED_Init();
    SoftTimer_Init();

    /* Invalid input: both calls must be rejected */
    SoftTimer_Start(10u, 500u, false, Red_On_Once);      /* invalid id    */
    SoftTimer_Start(TIMER_ONE_SHOT, 500u, false, NULL);  /* NULL callback */

    SoftTimer_Start(TIMER_ONE_SHOT, 500u,  false, Red_On_Once);
    SoftTimer_Start(TIMER_PERIODIC, 1000u, true,  Green_Toggle);

    for (;;) {
        SoftTimer_Process();
    }
}
