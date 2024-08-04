#pragma once
#include "tasoller.h"

// 16 cells (0-15)
#define CELL_0_Msk BIT15
#define CELL_1_Msk BIT14
#define CELL_2_Msk BIT13
#define CELL_3_Msk BIT12
#define CELL_4_Msk BIT11
#define CELL_5_Msk BIT10
#define CELL_6_Msk BIT9
#define CELL_7_Msk BIT8
#define CELL_8_Msk BIT7
#define CELL_9_Msk BIT6
#define CELL_10_Msk BIT5
#define CELL_11_Msk BIT4
#define CELL_12_Msk BIT3
#define CELL_13_Msk BIT2
#define CELL_14_Msk BIT1
#define CELL_15_Msk BIT0
// 32 pads (1-32)
#define PAD_1_Msk BIT30
#define PAD_2_Msk BIT31
#define PAD_3_Msk BIT28
#define PAD_4_Msk BIT29
#define PAD_5_Msk BIT26
#define PAD_6_Msk BIT27
#define PAD_7_Msk BIT24
#define PAD_8_Msk BIT25
#define PAD_9_Msk BIT22
#define PAD_10_Msk BIT23
#define PAD_11_Msk BIT20
#define PAD_12_Msk BIT21
#define PAD_13_Msk BIT18
#define PAD_14_Msk BIT19
#define PAD_15_Msk BIT16
#define PAD_16_Msk BIT17
#define PAD_17_Msk BIT14
#define PAD_18_Msk BIT15
#define PAD_19_Msk BIT12
#define PAD_20_Msk BIT13
#define PAD_21_Msk BIT10
#define PAD_22_Msk BIT11
#define PAD_23_Msk BIT8
#define PAD_24_Msk BIT9
#define PAD_25_Msk BIT6
#define PAD_26_Msk BIT7
#define PAD_27_Msk BIT4
#define PAD_28_Msk BIT5
#define PAD_29_Msk BIT2
#define PAD_30_Msk BIT3
#define PAD_31_Msk BIT0
#define PAD_32_Msk BIT1

typedef enum : uint8_t {
    // =========================
    // Actually used by Chunithm
    // =========================
    /* Set LED brightness and BRG values */
    SLIDER_CMD_Rx_LED = 0x02,

    /* Enable automatic transmission of standard reports */
    SLIDER_CMD_Rx_REPORT_ENABLE = 0x03,
    /* Disable automatic tranmission of all reports */
    SLIDER_CMD_Rx_REPORT_DISABLE = 0x04,

    /* Reset the slider state */
    SLIDER_CMD_Rx_RESET = 0x10,
    /* Retrieve hardware information (model number, etc.) */
    SLIDER_CMD_Rx_HW_INFO = 0xF0,

    // ==========================================
    // Custom additions, used for debugging, etc.
    // ==========================================
    SLIDER_CMD_Rx_DEBUG = 0xF1,

    // =====================================================================
    // Required for a complete slider implementation, but unused by the game
    // =====================================================================
    /* Request a single standard report */
    SLIDER_CMD_Rx_REPORT = 0x01,
    /* Set LED brightness and BRG values, with a report as the response */
    SLIDER_CMD_Rx_REPORT_PING_PONG = 0x05,

    /* Request a single raw report */
    SLIDER_CMD_Rx_RAW = 0x06,
    /* Enable automatic transmission of raw reports */
    SLIDER_CMD_Rx_RAW_ENABLE = 0x07,
    /* Set LED brightness and BRG values, with a raw report as the response */
    SLIDER_CMD_Rx_RAW_PING_PONG = 0x08,

    /* Set the offset used when producing byte-raw reports */
    SLIDER_CMD_Rx_BRAW_SET_OFFSET = 0x09,
    /* Set the shift used when producing byte-raw reports */
    SLIDER_CMD_Rx_BRAW_SET_SHIFT = 0x0A,
    /* Request a single byte-raw report */
    SLIDER_CMD_Rx_BRAW = 0x0B,
    /* Enable automatic transmission of byte-raw reports */
    SLIDER_CMD_Rx_BRAW_ENABLE = 0x0C,
    /* Set LED brightness and BRG values, with a byte-raw report as the response */
    SLIDER_CMD_Rx_BRAW_PING_PONG = 0x0D,

    /* Request the CPU status registers */
    SLIDER_CMD_Rx_CPU_STATUS = 0xE0,
} slider_cmd_Rx;
typedef enum : uint8_t {
    // Actual debugging stuff
    SLIDER_DEBUG_CMD_Rx_GET_FINGER_CAP = 0x00,
    SLIDER_DEBUG_CMD_Rx_TRACE_RESET = 0x01,
    SLIDER_DEBUG_CMD_Rx_GET_LAST_CS_START = 0x02,
    SLIDER_DEBUG_CMD_Rx_GET_LAST_CS_END = 0x03,
    SLIDER_DEBUG_CMD_Rx_PSoC_REQUEST_DEBUG = 0x04,

    // Flash access
    SLIDER_DEBUG_CMD_Rx_HOST_FMC_READ = 0x10,
    SLIDER_DEBUG_CMD_Rx_LED_FMC_READ = 0x12,

    // Chip reset
    SLIDER_DEBUG_CMD_Rx_HOST_ENTER_LDROM = 0x20,
    SLIDER_DEBUG_CMD_Rx_LED_ENTER_LDROM = 0x21,
    SLIDER_DEBUG_CMD_Rx_LED_CHECK = 0x22,

    // IO Access
    SLIDER_DEBUG_CMD_Rx_LED_GET_DIGITAL = 0x30,
} slider_debug_cmd_Rx;
typedef enum : uint8_t {
    SLIDER_CMD_Tx_REPORT = 0x01,
    SLIDER_CMD_Tx_REPORT_DISABLE = 0x04,

    SLIDER_CMD_Tx_RAW = 0x06,

    SLIDER_CMD_Tx_BRAW_SET_OFFSET = 0x09,
    SLIDER_CMD_Tx_BRAW_SET_SHIFT = 0x0A,
    SLIDER_CMD_Tx_BRAW = 0x0B,

    SLIDER_CMD_Tx_RESET = 0x10,
    SLIDER_CMD_Tx_CPU_STATUS = 0xE0,
    SLIDER_CMD_Tx_EXCEPTION = 0xEE,
    SLIDER_CMD_Tx_HW_INFO = 0xF0,

    SLIDER_CMD_Tx_DEBUG = 0xF1,
} slider_cmd_Tx;
typedef enum : uint8_t {
    SLIDER_EXCEPTION_CHECKSUM = 1,
    SLIDER_EXCEPTION_BUS_ERROR = 2,
} slider_exception;
/**
 * If an exception occurs for reasons other than processing a command,
 * this code is used.
 * That is, the exception occurs when performing an automatic report.
 * These will all be bus errors.
 */
#define SLIDER_EXCEPTION_CTX_GENERIC 0xED

typedef struct __packed {
    uint8_t u8Brightness;  // Range: 0~63
    struct __packed {
        uint8_t u8B;
        uint8_t u8R;
        uint8_t u8G;
    } aBRG[32];
} slider_cmd_Rx_led;
typedef struct __packed {
    uint16_t u16Raw[32];
} slider_cmd_Tx_raw;
typedef struct __packed {
    uint16_t u16Offset;
} slider_cmd_Rx_braw_set_offset;
typedef struct __packed {
    uint8_t u8Shift;
} slider_cmd_Rx_braw_set_shift;
typedef struct __packed {
    uint8_t u8Context;
    uint8_t u8Error;
} slider_cmd_TxRx_exception;
typedef struct __packed {
    char sModel[8];
    uint8_t u8DeviceClass;
    char sChipPart[5];
    uint8_t u8Unk0E;
    uint8_t u8FwVer;
    uint8_t u8Unk10;
    uint8_t u8Unk11;
} slider_cmd_Tx_hw_info;
typedef struct __packed {
    union {
        uint8_t u8Scr0;
        struct __packed {
            uint8_t bGlobalInterrupt : 1;
            uint8_t bRes16 : 1;
            uint8_t bWatchdogReset : 1;
            uint8_t bPowerOnReset : 1;
            uint8_t bSleep : 1;
            uint8_t bRes12 : 1;
            uint8_t bRes11 : 1;
            uint8_t bStop : 1;
        };
    };
    union {
        uint8_t u8Scr1;
        struct __packed {
            uint8_t bBootMultiple : 1;
            uint8_t bRes06 : 1;
            uint8_t bRes05 : 1;
            uint8_t bSlowImo : 1;
            uint8_t bEcoExistsWritten : 1;
            uint8_t bEcoExists : 1;
            uint8_t bRes01 : 1;
            uint8_t bSramWatchdog : 1;
        };
    };
} slider_cmd_Tx_cpu_status;

extern uint8_t gu8GameBrightness;  // Range: 0~63
void Slider_TickSerial(void);
void Slider_Tick1ms(void);
