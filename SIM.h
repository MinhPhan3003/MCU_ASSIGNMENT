#ifndef SIM_H
#define SIM_H

#include <stdint.h>
#include <stddef.h>

/*SIM = System Integration Module*/
/*Choose source for CLK_OUT*/
#define SIM_CLKOUT_SCG_CLKOUT      ((uint32_t)0U)
#define SIM_CLKOUT_SOSC_DIV2       ((uint32_t)2U)
#define SIM_CLKOUT_SIRC_DIV2       ((uint32_t)4U)
#define SIM_CLKOUT_FIRC_DIV2       ((uint32_t)6U)
#define SIM_CLKOUT_HCLK            ((uint32_t)7U)
#define SIM_CLKOUT_SPLL_DIV2       ((uint32_t)8U)
#define SIM_CLKOUT_BUS_CLK         ((uint32_t)9U)
#define SIM_CLKOUT_LPO128K         ((uint32_t)10U)
#define SIM_CLKOUT_LPO_CLK         ((uint32_t)12U)
#define SIM_CLKOUT_RTC_CLK         ((uint32_t)14U)

typedef union
{
    uint32_t REG;

    struct
    {
        uint32_t      : 4;
        uint32_t CLKOUTSEL : 4;
        uint32_t CLKOUTDIV : 3;
        uint32_t CLKOUTEN  : 1;
        uint32_t      : 20;
    } BITS;

} SIM_CHIPCTL_t;


typedef struct
{
    uint32_t        RESERVED0;        /* 0x00 */
    SIM_CHIPCTL_t   CHIPCTL;          /* 0x04 */
    uint32_t        RESERVED1;        /* 0x08 */
    uint32_t        FTMOPT0;          /* 0x0C */
    uint32_t        LPOCLKS;          /* 0x10 */
    uint32_t        RESERVED2[1];     /* 0x14-0x18 */
    uint32_t        ADCOPT;           /* 0x18 */
    uint32_t        FTMOPT1;          /* 0x1C */
    uint32_t        MISCTRL0;         /* 0x20 */
    uint32_t        SDID;             /* 0x24 */
    uint32_t        RESERVED3[6];     /* 0x28-0x3C */
    uint32_t        PLATCGC;          /* 0x40 */
    uint32_t        RESERVED4[2];     /* 0x44-0x48 */
    uint32_t        FCFG1;            /* 0x4C */
    uint32_t        RESERVED5;        /* 0x50 */
    uint32_t        UIDH;             /* 0x54 */
    uint32_t        UIDMH;            /* 0x58 */
    uint32_t        UIDML;            /* 0x5C */
    uint32_t        UIDL;             /* 0x60 */
    uint32_t        RESERVED6;        /* 0x64 */
    uint32_t        CLKDIV4;          /* 0x68 */
    uint32_t        MISCTRL1;         /* 0x6C */

} SIM_typedef;

#define SIM                      ((volatile SIM_typedef*)0x40048000UL)

void CHIP_CLKOUT_Init(uint32_t);          
 
#endif 