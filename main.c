#include "PCC.h"
#include "SIM.h"
#include "SCG.h"
#include "PORT.h"
#include "GPIO.h"
#include "SoftTimer.h"
#include "FlexCAN.h"

#define TIMER_ONE_SHOT      0u
#define TIMER_PERIODIC      1u
#define TIMER_CAN_TX        2u

/* Assignment 8 - CAN */
#define CAN_TX_ID           0x750u
#define CAN_CMD_ID          0x751u
#define CAN_BYTE_4          3u          /* "byte 4" = data[3] (byte 1 = data[0]) */
#define CAN_CMD_STOP        0u
#define CAN_CMD_START       1u
#define CAN_TX_PERIOD_MS    1000u

static uint8_t s_txCounter = 0u;        /* 0..255, keeps its value after STOP   */
static bool    s_txRunning = false;     /* START while running must be ignored   */

static void Red_On_Once(void)      { LED_On(LED_RED_PIN); }          /* 500 ms, one-shot  */
static void Green_Toggle(void)     { LED_Toggle(LED_GREEN_PIN); }    /* every 1000 ms     */

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

    CLOCK_Init();
    CLOCK_EnterHSRUN();

    CLOCK_EnablePeripheral(PCC_PORTE, PCS_PCC_OFF);
    PORT_SetMux(PORTE, 10u, PORT_MUX_ALT2);
    CHIP_CLKOUT_Init(SIM_CLKOUT_SOSC_DIV2);

    LED_Init();
    SoftTimer_Init();
    CAN_Init(CAN_CMD_ID);               /* nothing is sent until the tool sends START */

    /* Invalid input: both calls must be rejected */
    SoftTimer_Start(10u, 500u, false, Red_On_Once);      /* invalid id    */
    SoftTimer_Start(TIMER_ONE_SHOT, 500u, false, NULL);  /* NULL callback */

    SoftTimer_Start(TIMER_ONE_SHOT, 500u,  false, Red_On_Once);
    SoftTimer_Start(TIMER_PERIODIC, 1000u, true,  Green_Toggle);

    for (;;) {
        SoftTimer_Process();

        /* Byte 4 exists only when DLC >= 4 */
        if (CAN_Receive(&rxFrame) && (rxFrame.dlc > CAN_BYTE_4)) {
            Handle_Command(rxFrame.data[CAN_BYTE_4]);
        }
    }
}
