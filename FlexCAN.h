#ifndef FLEXCAN_H
#define FLEXCAN_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* ==========================================================================
 *  FlexCAN0 - Controller Area Network (RM chapter 55)
 *
 *  - Classical CAN, standard 11-bit ID, 500 kbit/s
 *  - Protocol Engine clock = oscillator clock = SOSCDIV2 8 MHz
 *    (CTRL1[CLKSRC] = 0, RM 55.1.7)
 *  - Pins: PTE4 = CAN0_RX, PTE5 = CAN0_TX (ALT5) -> external transceiver
 *  - MB0 = transmit, MB1 = receive one ID (interrupt, IRQ 81)
 * ========================================================================== */

/* Pins (other options: PTC2/PTC3 ALT3, PTB0/PTB1 ALT5) */
#define CAN_PCC_PORT            PCC_PORTE
#define CAN_PORT                PORTE
#define CAN_RX_PIN              4u
#define CAN_TX_PIN              5u
#define CAN_PIN_MUX             PORT_MUX_ALT5

/* Message buffers */
#define CAN_MB_COUNT            32u         /* MB RAM 0x80 - 0x27F, 16 bytes each */
#define CAN_TX_MB               0u
#define CAN_RX_MB               1u

#define CAN_IRQ_PRIO            3u          /* 0 = highest */

/* MB CS[CODE] - RM Table 55-11 (Rx) and Table 55-12 (Tx) */
#define CAN_CODE_RX_EMPTY       0x4u        /* active, waiting for a frame       */
#define CAN_CODE_TX_INACTIVE    0x8u        /* not transmitting                  */
#define CAN_CODE_TX_DATA        0xCu        /* transmit a data frame             */
#define CAN_CODE_BUSY           0x1u        /* CODE[0]: FlexCAN is updating the MB */

/* MB ID word: standard ID is in bits 28-18 */
#define CAN_STD_ID_SHIFT        18u
#define CAN_STD_ID_MASK         0x7FFu

#define CAN_STATIC_ASSERT(cond, name)   typedef char can_sa_##name[(cond) ? 1 : -1]

/* ========================================================================
 *  FlexCAN - Register definition
 * ======================================================================== */

/* MCR - Module Configuration Register */
typedef union {
    uint32_t REG;
    struct {
        uint32_t MAXMB   : 7;     /* bit 0-6   : number of the last MB          */
        uint32_t         : 1;     /* bit 7     */
        uint32_t IDAM    : 2;     /* bit 8-9   : Rx FIFO ID acceptance mode     */
        uint32_t         : 1;     /* bit 10    */
        uint32_t FDEN    : 1;     /* bit 11    : CAN FD enable                  */
        uint32_t AEN     : 1;     /* bit 12    : abort enable                   */
        uint32_t LPRIOEN : 1;     /* bit 13    : local priority enable          */
        uint32_t PNET_EN : 1;     /* bit 14    : pretended networking enable    */
        uint32_t DMA     : 1;     /* bit 15    : DMA enable (Rx FIFO)           */
        uint32_t IRMQ    : 1;     /* bit 16    : individual Rx masking          */
        uint32_t SRXDIS  : 1;     /* bit 17    : self reception disable         */
        uint32_t         : 2;     /* bit 18-19 */
        uint32_t LPMACK  : 1;     /* bit 20    : low power mode acknowledge     */
        uint32_t WRNEN   : 1;     /* bit 21    : warning interrupt enable       */
        uint32_t         : 1;     /* bit 22    */
        uint32_t SUPV    : 1;     /* bit 23    : supervisor mode                */
        uint32_t FRZACK  : 1;     /* bit 24    : freeze mode acknowledge        */
        uint32_t SOFTRST : 1;     /* bit 25    : soft reset                     */
        uint32_t         : 1;     /* bit 26    */
        uint32_t NOTRDY  : 1;     /* bit 27    : not ready (disable/stop/freeze) */
        uint32_t HALT    : 1;     /* bit 28    : halt (request freeze)          */
        uint32_t RFEN    : 1;     /* bit 29    : Rx FIFO enable                 */
        uint32_t FRZ     : 1;     /* bit 30    : freeze enable                  */
        uint32_t MDIS    : 1;     /* bit 31    : module disable                 */
    } BITS;
} CAN_MCR_t;

/* CTRL1 - Control 1 Register (bit timing) */
typedef union {
    uint32_t REG;
    struct {
        uint32_t PROPSEG : 3;     /* bit 0-2   : propagation segment - 1        */
        uint32_t LOM     : 1;     /* bit 3     : listen-only mode               */
        uint32_t LBUF    : 1;     /* bit 4     : lowest buffer transmitted first */
        uint32_t TSYN    : 1;     /* bit 5     : timer sync                     */
        uint32_t BOFFREC : 1;     /* bit 6     : 0 = automatic bus-off recovery */
        uint32_t SMP     : 1;     /* bit 7     : 3 samples per bit              */
        uint32_t         : 2;     /* bit 8-9   */
        uint32_t RWRNMSK : 1;     /* bit 10    : Rx warning interrupt mask      */
        uint32_t TWRNMSK : 1;     /* bit 11    : Tx warning interrupt mask      */
        uint32_t LPB     : 1;     /* bit 12    : loop-back mode                 */
        uint32_t CLKSRC  : 1;     /* bit 13    : 0 = oscillator, 1 = peripheral */
        uint32_t ERRMSK  : 1;     /* bit 14    : error interrupt mask           */
        uint32_t BOFFMSK : 1;     /* bit 15    : bus-off interrupt mask         */
        uint32_t PSEG2   : 3;     /* bit 16-18 : phase segment 2 - 1            */
        uint32_t PSEG1   : 3;     /* bit 19-21 : phase segment 1 - 1            */
        uint32_t RJW     : 2;     /* bit 22-23 : resync jump width - 1          */
        uint32_t PRESDIV : 8;     /* bit 24-31 : prescaler - 1                  */
    } BITS;
} CAN_CTRL1_t;

/* MB word 0 - Control and Status (RM Table 55-10) */
typedef union {
    uint32_t REG;
    struct {
        uint32_t TIMESTAMP : 16;  /* bit 0-15  : free running timer at Tx/Rx   */
        uint32_t DLC       : 4;   /* bit 16-19 : data length code              */
        uint32_t RTR       : 1;   /* bit 20    : remote frame                  */
        uint32_t IDE       : 1;   /* bit 21    : 0 = standard ID               */
        uint32_t SRR       : 1;   /* bit 22    */
        uint32_t           : 1;   /* bit 23    */
        uint32_t CODE      : 4;   /* bit 24-27 : MB state, CAN_CODE_xxx        */
        uint32_t           : 1;   /* bit 28    */
        uint32_t ESI       : 1;   /* bit 29    : CAN FD only                   */
        uint32_t BRS       : 1;   /* bit 30    : CAN FD only                   */
        uint32_t EDL       : 1;   /* bit 31    : CAN FD only                   */
    } BITS;
} CAN_MB_CS_t;

/* Message buffer, 8 data bytes. Data is big-endian inside each word:
 *   DATA[0] = byte0[31:24] byte1[23:16] byte2[15:8] byte3[7:0]
 *   DATA[1] = byte4[31:24] byte5[23:16] byte6[15:8] byte7[7:0] */
typedef struct {
    CAN_MB_CS_t CS;                     /* 0x0 control and status        */
    uint32_t    ID;                     /* 0x4 identifier                */
    uint32_t    DATA[2];                /* 0x8 data bytes 0-3, 4-7       */
} CAN_MB_t;

typedef struct {
    CAN_MCR_t   MCR;                    /* 0x000 */
    CAN_CTRL1_t CTRL1;                  /* 0x004 */
    uint32_t    TIMER;                  /* 0x008 free running timer (read = unlock MB) */
    uint32_t    RESERVED0;              /* 0x00C */
    uint32_t    RXMGMASK;               /* 0x010 */
    uint32_t    RX14MASK;               /* 0x014 */
    uint32_t    RX15MASK;               /* 0x018 */
    uint32_t    ECR;                    /* 0x01C error counters          */
    uint32_t    ESR1;                   /* 0x020 error and status (w1c)  */
    uint32_t    RESERVED1;              /* 0x024 */
    uint32_t    IMASK1;                 /* 0x028 MB0-31 interrupt enable */
    uint32_t    RESERVED2;              /* 0x02C */
    uint32_t    IFLAG1;                 /* 0x030 MB0-31 flags (w1c)      */
    uint32_t    CTRL2;                  /* 0x034 */
    uint32_t    ESR2;                   /* 0x038 */
    uint32_t    RESERVED3[2];           /* 0x03C - 0x040 */
    uint32_t    CRCR;                   /* 0x044 */
    uint32_t    RXFGMASK;               /* 0x048 */
    uint32_t    RXFIR;                  /* 0x04C */
    uint32_t    CBT;                    /* 0x050 */
    uint32_t    RESERVED4[11];          /* 0x054 - 0x07C */
    CAN_MB_t    MB[CAN_MB_COUNT];       /* 0x080 - 0x27F */
    uint32_t    RESERVED5[384];         /* 0x280 - 0x87F */
    uint32_t    RXIMR[CAN_MB_COUNT];    /* 0x880 - 0x8FF individual Rx masks */
} FLEXCAN_typedef;

#define CAN0_BaseAddress        0x40024000UL
#define CAN0                    ((volatile FLEXCAN_typedef *)CAN0_BaseAddress)

CAN_STATIC_ASSERT(sizeof(CAN_MB_t)                    == 0x10u,  can_mb_size);
CAN_STATIC_ASSERT(offsetof(FLEXCAN_typedef, IFLAG1)   == 0x030u, can_iflag1);
CAN_STATIC_ASSERT(offsetof(FLEXCAN_typedef, CBT)      == 0x050u, can_cbt);
CAN_STATIC_ASSERT(offsetof(FLEXCAN_typedef, MB)       == 0x080u, can_mb);
CAN_STATIC_ASSERT(offsetof(FLEXCAN_typedef, RXIMR)    == 0x880u, can_rximr);

/* ========================================================================
 *  API
 * ======================================================================== */

typedef struct {
    uint32_t id;                        /* 11-bit standard ID             */
    uint8_t  dlc;                       /* number of data bytes, 0..8     */
    uint8_t  data[8];                   /* data[0] = first byte on the bus */
} CAN_Frame_t;

void CAN_Init(uint32_t rx_id);                  /* 500 kbit/s, MB1 receives only rx_id  */
bool CAN_Send(const CAN_Frame_t *frame);        /* false: previous frame still pending  */
bool CAN_Receive(CAN_Frame_t *frame);           /* true: a new frame was copied         */

void CAN0_ORed_0_15_MB_IRQHandler(void);        /* name must match the S32DS vector table */

#endif /* FLEXCAN_H */
