#ifndef LPIT_H
#define LPIT_H

#include <stdint.h>
#include <stddef.h>
#include "PCC.h"         

/* ==========================================================================
 *  LPIT - Low Power Periodic Interrupt Timer
 *
 *  - 4 channels, each a 32-bit down counter: TVAL -> 0, then reload TVAL.
 *    Period = (TVAL + 1) / f_func
 *  - Two clocks:
 *      bus clock        : register access (from the system clock)
 *      functional clock : counter clock, selected by PCC_LPIT[PCS]
 *  - This project uses SIRCDIV2 = 8 MHz as functional clock:
 *      1 ms -> TVAL = 8 000 000 * 0.001 - 1 = 7999
 *  - Channel 0 is used as the 1 ms system tick (IRQ 48, LPIT0_Ch0_IRQHandler).
 * ========================================================================== */

/*LPIT Channels*/
#define LPIT_CH_0          0U
#define LPIT_CH_1          1U
#define LPIT_CH_2          2U
#define LPIT_CH_3          3U

/* Functional clock frequency: must match the PCS used in LPIT_Tick_Init()
 * (PCS_PCC_SIRCDIV2 = 8 MHz) */
#define LPIT_CLOCK_HZ           8000000u

/* Tick configuration */
#define LPIT_TICK_HZ            1000u                                   /* 1 ms */
#define LPIT_TICK_TVAL          ((LPIT_CLOCK_HZ / LPIT_TICK_HZ) - 1u)   /* 7999 */
#define LPIT_TICK_IRQ_PRIO      2u                                      /* 0 = highest */

/* TCTRL[MODE] */
#define LPIT_MODE_32BIT_PERIODIC        0u
#define LPIT_MODE_DUAL_16BIT_PERIODIC   1u
#define LPIT_MODE_32BIT_TRIG_ACC        2u
#define LPIT_MODE_32BIT_TRIG_INPUT      3u

#define LPIT_STATIC_ASSERT(cond, name)  typedef char lpit_sa_##name[(cond) ? 1 : -1]

/* ========================================================================
 *  LPIT - Register definition
 * ======================================================================== */

/* MCR - Module Control Register */
typedef union {
    uint32_t REG;
    struct {
        uint32_t M_CEN   : 1;     /* bit 0 : module clock enable              */
        uint32_t SW_RST  : 1;     /* bit 1 : software reset                   */
        uint32_t DOZE_EN : 1;     /* bit 2 : keep running in DOZE (low power) */
        uint32_t DBG_EN  : 1;     /* bit 3 : keep running when core halted    */
        uint32_t         : 28;
    } BITS;
} LPIT_MCR_t;

/* MSR - Module Status Register (TIFn are write-1-to-clear) */
typedef union {
    uint32_t REG;
    struct {
        uint32_t TIF0 : 1;        /* bit 0 */
        uint32_t TIF1 : 1;        /* bit 1 */
        uint32_t TIF2 : 1;        /* bit 2 */
        uint32_t TIF3 : 1;        /* bit 3 */
        uint32_t      : 28;
    } BITS;
} LPIT_MSR_t;

/* MIER - Module Interrupt Enable Register */
typedef union {
    uint32_t REG;
    struct {
        uint32_t TIE0 : 1;        /* bit 0 */
        uint32_t TIE1 : 1;        /* bit 1 */
        uint32_t TIE2 : 1;        /* bit 2 */
        uint32_t TIE3 : 1;        /* bit 3 */
        uint32_t      : 28;
    } BITS;
} LPIT_MIER_t;

/* TCTRLn - Timer Control Register */
typedef union {
    uint32_t REG;
    struct {
        uint32_t T_EN    : 1;     /* bit 0     : timer enable                 */
        uint32_t CHAIN   : 1;     /* bit 1     : chain with channel n-1       */
        uint32_t MODE    : 2;     /* bit 2-3   : timer operation mode         */
        uint32_t         : 12;    /* bit 4-15  */
        uint32_t TSOT    : 1;     /* bit 16    : start on trigger             */
        uint32_t TSOI    : 1;     /* bit 17    : stop on interrupt            */
        uint32_t TROT    : 1;     /* bit 18    : reload on trigger            */
        uint32_t         : 4;     /* bit 19-22 */
        uint32_t TRG_SRC : 1;     /* bit 23    : 0 external, 1 internal       */
        uint32_t TRG_SEL : 4;     /* bit 24-27 : trigger select               */
        uint32_t         : 4;     /* bit 28-31 */
    } BITS;
} LPIT_TCTRL_t;

/* Each timer channel:*/
typedef struct {
    uint32_t        TVAL;               /* 0x0 timer value (reload)       */
    const uint32_t  CVAL;               /* 0x4 current value (read only)  */
    LPIT_TCTRL_t    TCTRL;              /* 0x8 timer control              */
    uint32_t        RESERVED;           /* 0xC */
} LPIT_CH_t;

typedef struct {
    const uint32_t  VERID;              /* 0x00 */
    const uint32_t  PARAM;              /* 0x04 */
    LPIT_MCR_t      MCR;                /* 0x08 */
    LPIT_MSR_t      MSR;                /* 0x0C (w1c) */
    LPIT_MIER_t     MIER;               /* 0x10 */
    uint32_t        SETTEN;             /* 0x14 (W1S) set timer enable   */
    uint32_t        CLRTEN;             /* 0x18 (W1C) clear timer enable */
    uint32_t        RESERVED0;          /* 0x1C */
    LPIT_CH_t       CH[4];              /* 0x20, 0x30, 0x40, 0x50 : 4 Channels */
} LPIT_typedef;

#define LPIT0_BaseAddress       0x40037000UL
#define LPIT0                   ((volatile LPIT_typedef *)LPIT0_BaseAddress)

LPIT_STATIC_ASSERT(offsetof(LPIT_typedef, MIER)  == 0x10u, lpit_mier);
LPIT_STATIC_ASSERT(offsetof(LPIT_typedef, CH)    == 0x20u, lpit_ch0);
LPIT_STATIC_ASSERT(sizeof(LPIT_CH_t)             == 0x10u, lpit_ch_size);

/* ========================================================================
 *  API
 * ======================================================================== */

void     LPIT_Tick_Init(void);          /* channel 0 -> 1 ms interrupt, started   */
uint32_t LPIT_GetTickMs(void);          /* milliseconds since LPIT_Tick_Init()    */

void LPIT0_Ch0_IRQHandler(void);        /* name must match the S32DS vector table */

#endif /* LPIT_H */
