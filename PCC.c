#include "PCC.h"

void CLOCK_EnablePeripheral(uint32_t pcc_index, uint32_t PCS)
{
    if (pcc_index >= PCC_index) {
        return;
    }

    PCC->PCCn[pcc_index].BITS.CGC = 0u;
    PCC->PCCn[pcc_index].BITS.PCS = PCS;
    PCC->PCCn[pcc_index].BITS.CGC = 1u;
}

void CLOCK_DisablePeripheral(uint32_t pcc_index)
{
    if (pcc_index >= PCC_index) {
        return;
    }

    PCC->PCCn[pcc_index].BITS.CGC = 0u;
}
