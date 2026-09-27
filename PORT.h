#ifndef PORT_H
#define PORT_H

#include <stdint.h>
#include <stddef.h>

/* ==========================================================================
 *  PORT - Port Control and Interrupts (S32K144)
 * ========================================================================== */

#define PORT_PIN_COUNT          32u

/* PCR[MUX] - bit 10-8 */       
#define PORT_MUX_DISABLED       0u      /* ALT0: pin disabled / analog */
#define PORT_MUX_GPIO           1u      /* ALT1: GPIO                  */
#define PORT_MUX_ALT2           2u      /* ALT2..ALT7: chip-specific   */
#define PORT_MUX_ALT3           3u
#define PORT_MUX_ALT4           4u
#define PORT_MUX_ALT5           5u
#define PORT_MUX_ALT6           6u
#define PORT_MUX_ALT7           7u

#define PORT_STATIC_ASSERT(cond, name)  typedef char port_sa_##name[(cond) ? 1 : -1]

/* ========================================================================
 *  PORT - Register definition
 * ======================================================================== */

/* PCR - Pin Control Register n */
typedef union {
    uint32_t REG;
    struct {
        uint32_t PS    : 1;       /* bit 0     : pull select, 0 = down, 1 = up */
        uint32_t PE    : 1;       /* bit 1     : pull enable                   */
        uint32_t       : 2;       /* bit 2-3   */
        uint32_t PFE   : 1;       /* bit 4     : passive filter enable         */
        uint32_t       : 1;       /* bit 5     */
        uint32_t DSE   : 1;       /* bit 6     : drive strength enable         */
        uint32_t       : 1;       /* bit 7     */
        uint32_t MUX   : 3;       /* bit 8-10  : pin mux control               */
        uint32_t       : 4;       /* bit 11-14 */
        uint32_t LK    : 1;       /* bit 15    : lock PCR[15:0] until reset    */
        uint32_t IRQC  : 4;       /* bit 16-19 : interrupt / DMA config        */
        uint32_t       : 4;       /* bit 20-23 */
        uint32_t ISF   : 1;       /* bit 24    : interrupt status flag (w1c)   */
        uint32_t       : 7;       /* bit 25-31 */
    } BITS;
} PORT_PCR_t;

typedef struct {
    PORT_PCR_t  PCR[PORT_PIN_COUNT];    /* 0x000 - 0x07C */
    uint32_t    GPCLR;                  /* 0x080 (WO) global pin control low  */
    uint32_t    GPCHR;                  /* 0x084 (WO) global pin control high */
    uint32_t    GICLR;                  /* 0x088 (WO) global int control low  */
    uint32_t    GICHR;                  /* 0x08C (WO) global int control high */
    uint32_t    RESERVED0[4];           /* 0x090 - 0x09C */
    uint32_t    ISFR;                   /* 0x0A0 (w1c) interrupt status flags */
    uint32_t    RESERVED1[7];           /* 0x0A4 - 0x0BC */
    uint32_t    DFER;                   /* 0x0C0 digital filter enable  */
    uint32_t    DFCR;                   /* 0x0C4 digital filter clock   */
    uint32_t    DFWR;                   /* 0x0C8 digital filter width   */
} PORT_typedef;

/* Base address: 4 KB apart -> PCC index = 73..77 (consecutive) */
#define PORTA_BASE              0x40049000UL
#define PORTB_BASE              0x4004A000UL
#define PORTC_BASE              0x4004B000UL
#define PORTD_BASE              0x4004C000UL
#define PORTE_BASE              0x4004D000UL

#define PORTA                   ((volatile PORT_typedef *)PORTA_BASE)
#define PORTB                   ((volatile PORT_typedef *)PORTB_BASE)
#define PORTC                   ((volatile PORT_typedef *)PORTC_BASE)
#define PORTD                   ((volatile PORT_typedef *)PORTD_BASE)
#define PORTE                   ((volatile PORT_typedef *)PORTE_BASE)

PORT_STATIC_ASSERT(sizeof(PORT_PCR_t)              == 4u,     port_pcr_size);
PORT_STATIC_ASSERT(offsetof(PORT_typedef, GPCLR)   == 0x080u, port_gpclr);
PORT_STATIC_ASSERT(offsetof(PORT_typedef, ISFR)    == 0x0A0u, port_isfr);
PORT_STATIC_ASSERT(offsetof(PORT_typedef, DFER)    == 0x0C0u, port_dfer);

/* ========================================================================
 *  API
 * ======================================================================== */

void PORT_SetMux(volatile PORT_typedef *port, uint32_t pin, uint32_t mux);  /* mux = PORT_MUX_xxx        */

#endif /* PORT_H */
