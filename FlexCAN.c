#include "FlexCAN.h"
#include "PCC.h"
#include "PORT.h"
#include "NVIC.h"

/* Last received frame: written by the ISR, read by CAN_Receive() */
static volatile CAN_Frame_t s_rxFrame;
static volatile bool        s_rxNew = false;

/* ------------------------------------------------------------------------
 *  Init sequence (RM 55.6.1):
 *    1. PCC + PORT : bus clock gate, pins
 *    2. Disable    : select PE clock (CTRL1[CLKSRC])
 *    3. Freeze     : RM 55.1.9.1
 *    4. MCR        : individual Rx masks
 *    5. CTRL1      : bit timing 500 kbit/s
 *    6. MBs        : C/S of all MBs, then Tx MB and Rx MB
 *    7. RXIMR      : Rx MB accepts only rx_id
 *    8. IMASK/NVIC : Rx MB interrupt
 *    9. HALT = 0   : leave freeze, synchronize to the bus
 * ------------------------------------------------------------------------ */
void CAN_Init(uint32_t rx_id)
{
    uint32_t    i;
    CAN_CTRL1_t ctrl1;
    CAN_MB_CS_t cs;

    /* 1. PCC_FlexCAN0 has only CGC: the PE clock is chosen by
     *    CTRL1[CLKSRC], not by PCC[PCS] */
    CLOCK_EnablePeripheral(PCC_FLEXCAN0, PCS_PCC_OFF);
    CLOCK_EnablePeripheral(PCC_PORTE, PCS_PCC_OFF);
    PORT_SetMux(PORTE, CAN_RX_PIN, PORT_MUX_ALT5);
    PORT_SetMux(PORTE, CAN_TX_PIN, PORT_MUX_ALT5);

    /* 2. After reset the module is in Disable mode (MCR[MDIS] = 1).
     *    CLKSRC can be written only in this mode. 0 = SOSCDIV2 8 MHz */
    CAN0->CTRL1.BITS.CLKSRC = 0u;

    /* 3. Freeze mode entry (RM 55.1.9.1) */
    CAN0->MCR.BITS.FRZ  = 1u;
    CAN0->MCR.BITS.HALT = 1u;
    CAN0->MCR.BITS.MDIS = 0u;
    while (CAN0->MCR.BITS.FRZACK == 0u) {}

    /* 4. Each Rx MB uses its own RXIMR mask. MAXMB keeps its reset value 15
     *    -> MB0..MB15 are used, all served by IRQ 81 */
    CAN0->MCR.BITS.IRMQ = 1u;

    /* 5. Bit timing (RM 55.5.9.7 Protocol timing):
     *    Tq       = (PRESDIV + 1) / 8 MHz            = 125 ns
     *    Bit time = 1 + (PROPSEG + PSEG1 + 2) + (PSEG2 + 1)
     *             = 1 + 11 + 4                       = 16 Tq = 2 us -> 500 kbit/s
     *    Sample point = (1 + 11) / 16                = 75 % */
    ctrl1.REG          = 0u;       /* CLKSRC = 0, no loop-back, auto bus-off recovery */
    ctrl1.BITS.PRESDIV = 0u;
    ctrl1.BITS.PROPSEG = 6u;
    ctrl1.BITS.PSEG1   = 3u;
    ctrl1.BITS.PSEG2   = 3u;
    ctrl1.BITS.RJW     = 3u;
    CAN0->CTRL1.REG    = ctrl1.REG;

    /* 6-7. MB RAM and RXIMR are not reset by hardware (RM 55.6.1):
     *      write all of them. CS = 0 -> CODE = Rx INACTIVE */
    for (i = 0u; i < CAN_MB_COUNT; i++) {
        CAN0->MB[i].CS.REG  = 0u;
        CAN0->MB[i].ID      = 0u;
        CAN0->MB[i].DATA[0] = 0u;
        CAN0->MB[i].DATA[1] = 0u;
        CAN0->RXIMR[i]      = 0u;
    }

    /* Tx MB: inactive until CAN_Send() */
    CAN0->MB[CAN_TX_MB].CS.BITS.CODE = CAN_CODE_TX_INACTIVE;

    /* Rx MB (RM 55.5.3): ID first, then CODE = EMPTY activates the MB.
     * IDE = 0 -> only standard frames (CTRL2[EACEN] = 0: IDE is always compared) */
    CAN0->MB[CAN_RX_MB].ID = (rx_id & CAN_STD_ID_MASK) << CAN_STD_ID_SHIFT;
    CAN0->RXIMR[CAN_RX_MB] = CAN_STD_ID_MASK << CAN_STD_ID_SHIFT;   /* all 11 ID bits must match */
    cs.REG       = 0u;
    cs.BITS.CODE = CAN_CODE_RX_EMPTY;
    CAN0->MB[CAN_RX_MB].CS.REG = cs.REG;

    /* 8. Interrupt only for the Rx MB */
    CAN0->IFLAG1 = 0xFFFFFFFFu;                 /* w1c: clear all old flags */
    CAN0->IMASK1 = (1UL << CAN_RX_MB);

    NVIC_ClearPendingIRQ(CAN0_ORED_0_15_MB_IRQn);
    NVIC_SetPriority(CAN0_ORED_0_15_MB_IRQn, CAN_IRQ_PRIO);
    NVIC_EnableIRQ(CAN0_ORED_0_15_MB_IRQn);

    /* 9. Leave freeze mode */
    CAN0->MCR.BITS.HALT = 0u;
    while (CAN0->MCR.BITS.FRZACK == 1u) {}
}

/* ------------------------------------------------------------------------
 *  Transmit process (RM 55.5.1), MCR[AEN] = 0
 * ------------------------------------------------------------------------ */
bool CAN_Send(const CAN_Frame_t *frame)
{
    CAN_MB_CS_t cs;

    if ((frame == NULL) || (frame->dlc > 8u)) {
        return false;
    }

    /* CODE stays TX_DATA until the frame is sent (ACK received).
     * Do not overwrite a pending frame. */
    if (CAN0->MB[CAN_TX_MB].CS.BITS.CODE == CAN_CODE_TX_DATA) {
        return false;
    }

    /* 1. Clear the flag of the previous transmission (w1c: only this bit) */
    CAN0->IFLAG1 = (1UL << CAN_TX_MB);

    /* 6. ID */
    CAN0->MB[CAN_TX_MB].ID = (frame->id & CAN_STD_ID_MASK) << CAN_STD_ID_SHIFT;

    /* 7. Data */
    CAN0->MB[CAN_TX_MB].DATA[0] = ((uint32_t)frame->data[0] << 24) |
                                  ((uint32_t)frame->data[1] << 16) |
                                  ((uint32_t)frame->data[2] << 8)  |
                                   (uint32_t)frame->data[3];
    CAN0->MB[CAN_TX_MB].DATA[1] = ((uint32_t)frame->data[4] << 24) |
                                  ((uint32_t)frame->data[5] << 16) |
                                  ((uint32_t)frame->data[6] << 8)  |
                                   (uint32_t)frame->data[7];

    /* 8. C/S in ONE 32-bit write: IDE = 0, RTR = 0, DLC, CODE = DATA starts Tx */
    cs.REG       = 0u;
    cs.BITS.DLC  = frame->dlc;
    cs.BITS.CODE = CAN_CODE_TX_DATA;
    CAN0->MB[CAN_TX_MB].CS.REG = cs.REG;

    return true;
}

bool CAN_Receive(CAN_Frame_t *frame)
{
    bool received = false;

    if (frame == NULL) {
        return false;
    }

    /* Block the CAN IRQ while copying: otherwise the ISR could write a new
     * frame in the middle of the copy (half old, half new). A frame that
     * arrives now stays pending in NVIC and is handled after EnableIRQ. */
    NVIC_DisableIRQ(CAN0_ORED_0_15_MB_IRQn);
    if (s_rxNew) {
        *frame   = s_rxFrame;
        s_rxNew  = false;
        received = true;
    }
    NVIC_EnableIRQ(CAN0_ORED_0_15_MB_IRQn);

    return received;
}

/* ------------------------------------------------------------------------
 *  ISR - MB0..MB15. Only the Rx MB has IMASK = 1.
 *  Reading a received frame (RM 55.5.3):
 *    1-2. read C/S until BUSY = 0 (reading C/S locks the MB)
 *    3.   read ID and data
 *    4.   clear IFLAG (w1c)
 *    5.   read TIMER -> unlock the MB
 * ------------------------------------------------------------------------ */
void CAN0_ORed_0_15_MB_IRQHandler(void)
{
    CAN_MB_CS_t cs;

    if ((CAN0->IFLAG1 & (1UL << CAN_RX_MB)) == 0u) {
        return;
    }

    do {
        cs.REG = CAN0->MB[CAN_RX_MB].CS.REG;
    } while ((cs.BITS.CODE & CAN_CODE_BUSY) != 0u);

    s_rxFrame.id      = (CAN0->MB[CAN_RX_MB].ID >> CAN_STD_ID_SHIFT) & CAN_STD_ID_MASK;
    s_rxFrame.dlc     = (uint8_t)cs.BITS.DLC;
    s_rxFrame.data[0] = (uint8_t)(CAN0->MB[CAN_RX_MB].DATA[0] >> 24);
    s_rxFrame.data[1] = (uint8_t)(CAN0->MB[CAN_RX_MB].DATA[0] >> 16);
    s_rxFrame.data[2] = (uint8_t)(CAN0->MB[CAN_RX_MB].DATA[0] >> 8);
    s_rxFrame.data[3] = (uint8_t)(CAN0->MB[CAN_RX_MB].DATA[0]);
    s_rxFrame.data[4] = (uint8_t)(CAN0->MB[CAN_RX_MB].DATA[1] >> 24);
    s_rxFrame.data[5] = (uint8_t)(CAN0->MB[CAN_RX_MB].DATA[1] >> 16);
    s_rxFrame.data[6] = (uint8_t)(CAN0->MB[CAN_RX_MB].DATA[1] >> 8);
    s_rxFrame.data[7] = (uint8_t)(CAN0->MB[CAN_RX_MB].DATA[1]);

    CAN0->IFLAG1 = (1UL << CAN_RX_MB);
    (void)CAN0->TIMER;

    s_rxNew = true;
}
