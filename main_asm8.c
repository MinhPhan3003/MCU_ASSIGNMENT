#include "SCG.h"
#include "SoftTimer.h"
#include "FlexCAN.h"

/* ==========================================================================
 *  Assignment 8 - CAN, 500 kbit/s
 *    - Tx 0x750 every 1 s: DLC 8, counter (0..255) in byte 4, other bytes 0
 *    - Rx 0x751 (interrupt): byte 4 = 1 -> start sending, 0 -> stop
 *    - Nothing is sent until the tool sends START
 * ========================================================================== */

#define TIMER_CAN_TX        0u

#define CAN_TX_ID           0x750u
#define CAN_CMD_ID          0x751u
#define CAN_BYTE_4          3u          /* "byte 4" = data[3] (byte 1 = data[0]) */
#define CAN_CMD_STOP        0u
#define CAN_CMD_START       1u
#define CAN_TX_PERIOD_MS    1000u

static uint8_t s_txCounter = 0u;        /* 0..255, keeps its value after STOP   */
static bool    s_txRunning = false;     /* START while running must be ignored   */

/* Frame 0x750, DLC 8, counter in byte 4, other bytes 0 */
static void Send_Counter(void)
{
    CAN_Frame_t frame = { CAN_TX_ID, 8u, { 0u } };

    frame.data[CAN_BYTE_4] = s_txCounter;

    /* Count up only when the frame was accepted -> the tool sees 0, 1, 2 ... */
    if (CAN_Send(&frame)) {
        s_txCounter++;                  /* 255 -> 0 */
    }
}

static void Handle_Command(uint8_t command)
{
    if ((command == CAN_CMD_START) && !s_txRunning) {
        Send_Counter();                 /* first frame immediately, then every 1 s */
        SoftTimer_Start(TIMER_CAN_TX, CAN_TX_PERIOD_MS, true, Send_Counter);
        s_txRunning = true;
    } else if (command == CAN_CMD_STOP) {
        SoftTimer_Stop(TIMER_CAN_TX);
        s_txRunning = false;
    } else {
        /* START while already sending, or another value: ignored */
    }
}

int main(void)
{
    CAN_Frame_t rxFrame;

    CLOCK_Init();                       /* also SOSCDIV2 = 8 MHz for FlexCAN */
    CLOCK_EnterHSRUN();

    SoftTimer_Init();
    CAN_Init(CAN_CMD_ID);

    for (;;) {
        SoftTimer_Process();

        /* Byte 4 exists only when DLC >= 4 */
        if (CAN_Receive(&rxFrame) && (rxFrame.dlc > CAN_BYTE_4)) {
            Handle_Command(rxFrame.data[CAN_BYTE_4]);
        }
    }
}
