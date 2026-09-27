#include "PCC.h"
#include "SCG.h"
#include "SIM.h"
#include "PORT.h"

/* ==========================================================================
 *  Assignment 02 - Clock
 *    - Core 112 MHz, bus 56 MHz, flash 28 MHz (HSRUN, SPLL from SOSC 8 MHz)
 *    - CLKOUT = SOSC_DIV2 8 MHz / 4 = 2 MHz on PTE10 (ALT2)
 * ========================================================================== */

#define CLKOUT_PIN          10u         /* PTE10 */

int main(void)
{
    CLOCK_Init();                       /* RUN: SPLL 112 MHz, core 56 MHz */
    CLOCK_EnterHSRUN();                 /* HSRUN: core 112 MHz            */

    CLOCK_EnablePeripheral(PCC_PORTE, PCS_PCC_OFF);
    PORT_SetMux(PORTE, CLKOUT_PIN, PORT_MUX_ALT2);
    CHIP_CLKOUT_Init(SIM_CLKOUT_SOSC_DIV2);

    for (;;) {
    }
}
