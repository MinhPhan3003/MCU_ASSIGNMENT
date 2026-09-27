#ifndef NVIC_H
#define NVIC_H

#include <stdint.h>
#include <stddef.h>

/* ==========================================================================
 *  NVIC - Nested Vectored Interrupt Controller (Cortex-M4F core peripheral)
 *
 *  - Base address 0xE000E100 (System Control Space).
 *  - S32K144 implements 4 priority bits -> 16 levels (0 = highest).
 *    The level is stored in bits [7:4] of the IP byte, bits [3:0] read as 0.
 *  - IRQ n is controlled by bit (n % 32) of word (n / 32) in ISER/ICER/...
 * ========================================================================== */

#define NVIC_PRIO_BITS          4u
#define NVIC_PRIO_MAX           ((1u << NVIC_PRIO_BITS) - 1u)   /* 15 = lowest */

/* S32K144 IRQ numbers (RM attachment S32K1xx_DMA_Interrupt_mapping.xlsx) */
#define LPIT0_CH0_IRQn          48u
#define LPIT0_CH1_IRQn          49u
#define LPIT0_CH2_IRQn          50u
#define LPIT0_CH3_IRQn          51u
#define CAN0_ORED_0_15_MB_IRQn  81u     /* FlexCAN0 MB0..MB15 */

#define NVIC_STATIC_ASSERT(cond, name)  typedef char nvic_sa_##name[(cond) ? 1 : -1]

/* ========================================================================
 *  NVIC - Register definition
 * ======================================================================== */

typedef struct {
    volatile uint32_t    ISER[8];                /* 0x000 set-enable      (W1S) */
    uint32_t             RESERVED0[24];
    volatile uint32_t    ICER[8];                /* 0x080 clear-enable    (W1C) */
    uint32_t             RESERVED1[24];
    volatile uint32_t    ISPR[8];                /* 0x100 set-pending     (W1S) */
    uint32_t             RESERVED2[24];
    volatile uint32_t    ICPR[8];                /* 0x180 clear-pending   (W1C) */
    uint32_t             RESERVED3[24];
    volatile uint32_t    IABR[8];                /* 0x200 active bit      (RO)  */
    uint32_t             RESERVED4[56];
    volatile uint8_t     IP[240];                /* 0x300 priority, 1 byte per IRQ */
} NVIC_typedef;

#define NVIC                    ((volatile NVIC_typedef *)0xE000E100UL)

NVIC_STATIC_ASSERT(offsetof(NVIC_typedef, ICER) == 0x080u, nvic_icer);
NVIC_STATIC_ASSERT(offsetof(NVIC_typedef, ICPR) == 0x180u, nvic_icpr);
NVIC_STATIC_ASSERT(offsetof(NVIC_typedef, IP)   == 0x300u, nvic_ip);

/* ========================================================================
 *  API
 * ======================================================================== */

void NVIC_EnableIRQ(uint32_t irq);
void NVIC_DisableIRQ(uint32_t irq);
void NVIC_ClearPendingIRQ(uint32_t irq);
void NVIC_SetPriority(uint32_t irq, uint32_t priority);    /* 0 (highest) .. 15 */

#endif /* NVIC_H */
