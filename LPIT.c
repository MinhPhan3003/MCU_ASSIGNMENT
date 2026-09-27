#include "LPIT.h"
#include "NVIC.h"

static volatile uint32_t s_tickMs        = 0u;


void LPIT_Tick_Init(void)
{
 
    /*Initialization Guide in 48.5.2 */
    /* 1. Clock gate + functional clock (PCS is only writable when CGC = 0) */
    CLOCK_EnablePeripheral(PCC_LPIT, PCS_PCC_SIRCDIV2);

    /* 2. Enable the module clock. 
        (0.5 us at 8 MHz) before accessing the other LPIT registers.*/
    LPIT0->MCR.BITS.M_CEN  = 1u;
    LPIT0->MCR.BITS.DBG_EN = 0u;       
    /*RM: wait 4 functional clock cycles*/
    for (uint32_t i = 0u; i < 32u; i++) {
        (void)LPIT0->MCR.REG;
    }

    /* 3. Stop the channel and select the mode */
    LPIT0->CH[LPIT_CH_0].TCTRL.BITS.T_EN = 0u;
    LPIT0->CH[LPIT_CH_0].TCTRL.BITS.MODE = LPIT_MODE_32BIT_PERIODIC;

    /* 4. Load value: period = (TVAL + 1) / 8 MHz = 1 ms */
    LPIT0->CH[LPIT_CH_0].TVAL = LPIT_TICK_TVAL;

    /* 5. Clear a stale flag, enable the interrupt */
    LPIT0->MSR.REG = (1UL << LPIT_CH_0);
    LPIT0->MIER.BITS.TIE0 = 1u;

    /* 6. NVIC */
    NVIC_ClearPendingIRQ(LPIT0_CH0_IRQn);
    NVIC_SetPriority(LPIT0_CH0_IRQn, LPIT_TICK_IRQ_PRIO);
    NVIC_EnableIRQ(LPIT0_CH0_IRQn);

    /* 7. Start Timer Control Register */
    s_tickMs = 0u;
    LPIT0->CH[LPIT_CH_0].TCTRL.BITS.T_EN = 1u;
}

uint32_t LPIT_GetTickMs(void)
{
    return s_tickMs;
}

/* ------------------------------------------------------------------------
 *  ISR - keep it short: clear the flag, count the tick, return.
 *  All timer callbacks run later in SoftTimer_Process() (main context).
 * ------------------------------------------------------------------------ */
void LPIT0_Ch0_IRQHandler(void)
{
    /* Clear TIF0. MSR is W1C write ONLY bit 0 */
    LPIT0->MSR.REG = (1UL << LPIT_CH_0);

    while ((LPIT0->MSR.REG & (1UL << LPIT_CH_0)) != 0u) {}

    s_tickMs++;
}
