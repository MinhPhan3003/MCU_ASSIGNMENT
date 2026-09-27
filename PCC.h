#ifndef PCC_H
#define PCC_H
#define PCC_index          116u

/*PERIPHERAL CLOCK  CONTROL (PCC) module provides clock control and configuration for
on-chip peripherals. Each peripheral has its own clock control and configuration register.*/

#include <stdint.h>
#include <stddef.h>

/* PCC.PCS */

#define PCS_PCC_OFF             0u
#define PCS_PCC_SOSCDIV2        1u
#define PCS_PCC_SIRCDIV2        2u
#define PCS_PCC_FIRCDIV2        3u
#define PCS_PCC_SPLLDIV2        6u

/*PCC Memory Map*/
#define PCC_FLEXCAN0      36u     /* 0x090 */
#define PCC_FLEXCAN1      37u     /* 0x094 */
#define PCC_FTM3          38u     /* 0x098 */
#define PCC_ADC1          39u     /* 0x09C */
#define PCC_FLEXCAN2      43u     /* 0x0AC */
#define PCC_LPSPI0        44u     /* 0x0B0 */
#define PCC_LPSPI1        45u     /* 0x0B4 */
#define PCC_LPSPI2        46u     /* 0x0B8 */
#define PCC_LPIT          55u     /* 0x0DC */
#define PCC_FTM0          56u     /* 0x0E0 */
#define PCC_FTM1          57u     /* 0x0E4 */
#define PCC_FTM2          58u     /* 0x0E8 */
#define PCC_ADC0          59u     /* 0x0EC */
#define PCC_LPTMR0        64u     /* 0x100 */
#define PCC_PORTA         73u     /* 0x124 */
#define PCC_PORTB         74u     /* 0x128 */
#define PCC_PORTC         75u     /* 0x12C */
#define PCC_PORTD         76u     /* 0x130 */
#define PCC_PORTE         77u     /* 0x134 */
#define PCC_LPI2C0        102u    /* 0x198 */
#define PCC_LPUART0       106u    /* 0x1A8 */
#define PCC_LPUART1       107u    /* 0x1AC */
#define PCC_LPUART2       108u    /* 0x1B0 */

typedef union {
    uint32_t REG;
    struct {
        uint32_t PCD   : 3;       /* bit 0-2   */
        uint32_t FRAC  : 1;       /* bit 3     */
        uint32_t       : 20;
        uint32_t PCS   : 3;       /* bit 24-26 */
        uint32_t       : 2;
        uint32_t INUSE : 1;       /* bit 29    */
        uint32_t CGC   : 1;       /* bit 30    */
        uint32_t PR    : 1;       /* bit 31 (RO) */                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                     
    } BITS;
} PCC_REG_t;

typedef struct {
    PCC_REG_t PCCn[PCC_index];
} PCC_typedef;

#define PCC                     ((volatile PCC_typedef *)0x40065000UL)


void CLOCK_EnablePeripheral(uint32_t pcc_index, uint32_t PCS);  
void CLOCK_DisablePeripheral(uint32_t pcc_index);

#endif