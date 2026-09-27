#include "SCG.h"

/* Restart back RUN + FIRC 48/48/24 MHz.*/
static void Clock_Run_On_Firc(void)
{   
    if (SMC->PMSTAT.BITS.PMSTAT == SMC_PMSTAT_HSRUN) {
        CLOCK_EnterRUN();
    }

    SCG->RCCR.BITS.SCS = SCG_SCS_FIRC; // Use FIRC
    while (SCG->CSR.BITS.SCS != SCG_SCS_FIRC) {}

    SCG->RCCR.BITS.DIVCORE = 0u;            /* 48 MHz */
    SCG->RCCR.BITS.DIVBUS  = 0u;            /* 48 MHz */
    SCG->RCCR.BITS.DIVSLOW = 1u;            /* 24 MHz */
}

/* SIRCDIV2 = 8 MHz, FIRCDIV2 = 48 MHz (PCS sources for peripherals) */
static void Clock_Sirc_Firc_Div_Init(void)
{
    SCG->SIRCDIV.BITS.DIV1 = SCG_DIV_1;
    SCG->SIRCDIV.BITS.DIV2 = SCG_DIV_1;
    SCG->FIRCDIV.BITS.DIV1 = SCG_DIV_1;
    SCG->FIRCDIV.BITS.DIV2 = SCG_DIV_1;
}

/* SOSC 8 MHz crystal */
static void Clock_SOSC_Init(void)
{
    SCG->SPLLCSR.REG = 0u;                  /* turn off SPLL */
    SCG->SOSCCSR.REG = 0u;                  /* SOSCCFG only write when SOSCEN = 0  */

    // Two out put from SOSC_CLK
    SCG->SOSCDIV.BITS.DIV1 = SCG_DIV_1;     /* SOSCDIV1 = 8 MHz */
    SCG->SOSCDIV.BITS.DIV2 = SCG_DIV_1;     /* SOSCDIV2 = 8 MHz */

    SCG->SOSCCFG.BITS.EREFS = 1u;           /* crystal         */
    SCG->SOSCCFG.BITS.HGO   = 0u;           /* Low-Gain        */
    SCG->SOSCCFG.BITS.RANGE = 2u;           /* medium, 4-8 MHz */

    SCG->SOSCCSR.BITS.SOSCEN = 1u;          /*Start SOSC */
    while (SCG->SOSCCSR.BITS.SOSCVLD == 0u) {}
}

/* SPLL: VCO 224 MHz, SPLL_CLK 112 MHz - 28.3.17 .. 28.3.19 */
static void Clock_SPLL_Init(void)
{
    SCG->SPLLCSR.REG = 0u;                  /* SPLLCFG writable only when SPLLEN = 0 */

    SCG->SPLLDIV.BITS.DIV1 = SCG_DIV_2;     /* SPLLDIV1 = 56 MHz */
    SCG->SPLLDIV.BITS.DIV2 = SCG_DIV_4;     /* SPLLDIV2 = 28 MHz */

    SCG->SPLLCFG.BITS.SOURCE = 0u;          /* SOSC */
    SCG->SPLLCFG.BITS.PREDIV = SPLL_PREDIV(1);
    SCG->SPLLCFG.BITS.MULT   = SPLL_MULT(28);

    SCG->SPLLCSR.BITS.SPLLEN = 1u;
    while (!(SCG->SPLLCSR.BITS.SPLLVLD == 1u)) {}
}

/* HCCR (HSRUN) and RCCR (RUN) - both take SPLL as clock source */
static void Clock_CCR_Init(void)
{
    /* HSRUN: 112 / 56 / 28 MHz
       This is only config for HCCR
     * HCCR not active when mode is RUN */
    SCG->HCCR.BITS.SCS     = SCG_SCS_SPLL;
    SCG->HCCR.BITS.DIVCORE = 0u;            /* /1 */
    SCG->HCCR.BITS.DIVBUS  = 1u;            /* /2 */
    SCG->HCCR.BITS.DIVSLOW = 3u;            /* /4 */

    /* RUN: 56 / 28 / 18.67 MHz (RCCR is active now)
     * Dividers are written BEFORE SCS so that the core never runs
     * 112 MHz in RUN mode (RUN limit: core 80, bus 40, flash 26.67 MHz). */
    SCG->RCCR.BITS.DIVSLOW = 2u;            /* /3 */
    SCG->RCCR.BITS.DIVBUS  = 1u;            /* /2 */
    SCG->RCCR.BITS.DIVCORE = 1u;            /* /2 */
    SCG->RCCR.BITS.SCS     = SCG_SCS_SPLL;
}

/* ------------------------------------------------------------------------
 *  Public
 * ------------------------------------------------------------------------ */

void CLOCK_Init(void)
{
    Clock_Run_On_Firc();
    Clock_Sirc_Firc_Div_Init();
    Clock_SOSC_Init();
    Clock_SPLL_Init();
    Clock_CCR_Init();

    /* Check RUN is using SPLL */
    while (SCG->CSR.BITS.SCS != SCG_SCS_SPLL) {}

    
}

void CLOCK_EnterHSRUN(void)
{   
    SMC->PMPROT.BITS.AHSRUN = 1u;
    /*PMSTAT is read only*/
    /* Start HSRUN only when PMSTAT is RUN mode */
    if (SMC->PMSTAT.BITS.PMSTAT != SMC_PMSTAT_RUN) {
        return;
    }

    SMC->PMCTRL.BITS.RUNM = SMC_RUNM_HSRUN;
    while (SMC->PMSTAT.BITS.PMSTAT != SMC_PMSTAT_HSRUN) {}
    while (SCG->CSR.REG != SCG->HCCR.REG) {}   
}

void CLOCK_EnterRUN(void)
{
    SMC->PMCTRL.BITS.RUNM = SMC_RUNM_RUN; // Switch to RUN mode
    while (SMC->PMSTAT.BITS.PMSTAT != SMC_PMSTAT_RUN) {}
    while (SCG->CSR.REG != SCG->RCCR.REG) {}
}


