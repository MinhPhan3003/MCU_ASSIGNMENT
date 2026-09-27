#ifndef GPIO_H
#define GPIO_H

#include <stdint.h>
#include <stddef.h>

/* ==========================================================================
 *  GPIO - General Purpose Input/Output (S32K144)
 *
 *  - Before using a pin as GPIO:
 *      1. CLOCK_EnablePeripheral(PCC_PORTx, PCS_PCC_OFF)
 *      2. PORT_SetMux(PORTx, pin, PORT_MUX_GPIO)
 *      3. GPIO_SetOutput(PTx, pin)
 *  - PSOR / PCOR / PTOR are write-1-to-act: writing 0 has no effect, so a
 *    single bit can be changed without read-modify-write.
 *
 *  S32K144EVB RGB LED (active low: output 0 = LED on)
 *      PTD0 = blue, PTD15 = red, PTD16 = green
 * ========================================================================== */

#define GPIO_STATIC_ASSERT(cond, name)  typedef char gpio_sa_##name[(cond) ? 1 : -1]

/* ========================================================================
 *  GPIO - Register definition
 * ======================================================================== */

typedef struct {
    uint32_t        PDOR;               /* 0x00 data output          */
    uint32_t        PSOR;               /* 0x04 set output     (W1S) */
    uint32_t        PCOR;               /* 0x08 clear output   (W1C) */
    uint32_t        PTOR;               /* 0x0C toggle output  (W1T) */
    const uint32_t  PDIR;               /* 0x10 data input     (RO)  */
    uint32_t        PDDR;               /* 0x14 direction: 1 = output */
    uint32_t        PIDR;               /* 0x18 input disable        */
} GPIO_typedef;

#define PTA                     ((volatile GPIO_typedef *)0x400FF000UL)
#define PTB                     ((volatile GPIO_typedef *)0x400FF040UL)
#define PTC                     ((volatile GPIO_typedef *)0x400FF080UL)
#define PTD                     ((volatile GPIO_typedef *)0x400FF0C0UL)
#define PTE                     ((volatile GPIO_typedef *)0x400FF100UL)

GPIO_STATIC_ASSERT(offsetof(GPIO_typedef, PDDR) == 0x14u, gpio_pddr);

/* EVB LEDs */
#define LED_PORT                PORTD
#define LED_GPIO                PTD
#define LED_BLUE_PIN            0u
#define LED_RED_PIN             15u
#define LED_GREEN_PIN           16u

/* ========================================================================
 *  API
 * ======================================================================== */

void GPIO_SetOutput(volatile GPIO_typedef *gpio, uint32_t pin);
void GPIO_WritePin(volatile GPIO_typedef *gpio, uint32_t pin, uint32_t level);
void GPIO_TogglePin(volatile GPIO_typedef *gpio, uint32_t pin);

void LED_Init(void);                    /* PORTD clock, mux, output, all LEDs off */
void LED_On(uint32_t pin);
void LED_Off(uint32_t pin);
void LED_Toggle(uint32_t pin);

#endif /* GPIO_H */
