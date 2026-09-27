#include "PORT.h"

/* ------------------------------------------------------------------------
 *  NOTE: the PORT clock must be enabled in PCC before calling this driver:
 *        CLOCK_EnablePeripheral(PCC_PORTx, PCS_PCC_OFF);
 *        Accessing PORT registers with CGC = 0 causes a bus fault.
 * ------------------------------------------------------------------------ */

void PORT_SetMux(volatile PORT_typedef* port, uint32_t pin, uint32_t mux)
{
    PORT_PCR_t PCR_temp;

    if ((port == NULL) || (pin >= PORT_PIN_COUNT) || (mux > PORT_MUX_ALT7)) {
        return;
    }

    PCR_temp.REG = port->PCR[pin].REG;

    /* LK = 1: PCR[15:0] is locked until next reset, MUX cannot change */
    if (PCR_temp.BITS.LK == 1u) {
        return;
    }

    /* ISF is write-1-to-clear: writing back a 1 would clear a pending flag */
    PCR_temp.BITS.ISF = 0u;
    PCR_temp.BITS.MUX = mux;

    port->PCR[pin].REG = PCR_temp.REG;
}