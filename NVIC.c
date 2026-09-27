#include "NVIC.h"

/* ------------------------------------------------------------------------
 *  ISER / ICER / ISPR / ICPR are "write 1 to act" registers: writing 0 has
 *  no effect, so a plain write of one bit is used (no read-modify-write).
 * ------------------------------------------------------------------------ */

void NVIC_EnableIRQ(uint32_t irq)
{
    NVIC->ISER[irq >> 5] = (1UL << (irq & 0x1Fu));
}

void NVIC_DisableIRQ(uint32_t irq)
{
    NVIC->ICER[irq >> 5] = (1UL << (irq & 0x1Fu));

    /* Same as CMSIS __NVIC_DisableIRQ(): DSB + ISB make sure the IRQ is
     * really disabled before the next instruction runs. */
#if defined(__GNUC__)
    __asm volatile ("dsb 0xF" ::: "memory");
    __asm volatile ("isb 0xF" ::: "memory");
#endif
}

void NVIC_ClearPendingIRQ(uint32_t irq)
{
    NVIC->ICPR[irq >> 5] = (1UL << (irq & 0x1Fu));
}

void NVIC_SetPriority(uint32_t irq, uint32_t priority)
{
    if (priority > NVIC_PRIO_MAX) {
        priority = NVIC_PRIO_MAX;
    }

    /* Only the upper NVIC_PRIO_BITS bits of the byte are implemented */
    NVIC->IP[irq] = (uint8_t)(priority << (8u - NVIC_PRIO_BITS));
}
