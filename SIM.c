#include "SIM.h"

void CHIP_CLKOUT_Init(uint32_t src){
     switch(src)
    {
    case SIM_CLKOUT_SCG_CLKOUT:
    case SIM_CLKOUT_SOSC_DIV2:
    case SIM_CLKOUT_SIRC_DIV2:
    case SIM_CLKOUT_FIRC_DIV2:
    case SIM_CLKOUT_HCLK:
    case SIM_CLKOUT_SPLL_DIV2:
    case SIM_CLKOUT_BUS_CLK:
    case SIM_CLKOUT_LPO128K:
    case SIM_CLKOUT_LPO_CLK:
    case SIM_CLKOUT_RTC_CLK:
        break;

    default:
        return;
    }
    SIM->CHIPCTL.BITS.CLKOUTEN = 0U;
    SIM->CHIPCTL.BITS.CLKOUTSEL = src;  // Using SOSC = 8Mhz
    SIM->CHIPCTL.BITS.CLKOUTDIV = 3U;   // Div by 4 to get 2Mhz  
    SIM->CHIPCTL.BITS.CLKOUTEN  = 1U;
    
}
