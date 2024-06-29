#pragma once

#include <stdint.h>

#define PSoC_DATA_SIZE 32
#define PSoC_PACKET_SIZE (2 + PSoC_DATA_SIZE)

// Used for calculation of PSoC_FINGER_CAP_MIN
// The difference between CAP and MIN must be divisible by 8
#define PSoC_FINGER_CAP_MIN 20  // 0.2pF
#define PSoC_DEFAULT_CAP 340    // 3.2pF
// Maximum is CAP*2-MIN*2 ie 6.0pF

#define PSoC_FINGER_CAP_STEP ((PSoC_DEFAULT_CAP - PSoC_FINGER_CAP_MIN) / 8)

typedef enum {
    // Host -> PSoC
    PSoC_CMD_TX_GET_FINGER_CAP = 0xB5,
    PSoC_CMD_TX_GET_DEBUG = 0xB6,
    PSoC_CMD_TX_SET_FINGER_CAP = 0xBD,
    PSoC_CMD_TX_ENABLE_DEBUG = 0xDE,
} PSoC_CMD_TX;
typedef enum {
    // PSoC -> Host
    PSoC_CMD_RX_MASTER_DIFF = 0xAD,
    PSoC_CMD_RX_SLAVE_DIFF = 0xAF,
    PSoC_CMD_RX_REQUEST_FINGER_CAP = 0xC1,

    // PSoC -> Host (Debug)
    PSoC_CMD_RX_MASTER_TOUCH_TH = 0xAA,
    PSoC_CMD_RX_SLAVE_TOUCH_TH = 0xAB,
    PSoC_CMD_RX_MASTER_FINGER_TH = 0xA0,
    PSoC_CMD_RX_MASTER_HYSTERESIS = 0xA1,
    PSoC_CMD_RX_SLAVE_FINGER_TH = 0xA2,
    PSoC_CMD_RX_SLAVE_HYSTERESIS = 0xA3,
    PSoC_CMD_RX_CS_START = 0x00,
    PSoC_CMD_RX_CS_END = 0xFF,

    // Packets that get a direct response
    PSoC_CMD_RX_GET_FINGER_CAP = PSoC_CMD_TX_GET_FINGER_CAP,
    PSoC_CMD_RX_SET_FINGER_CAP = PSoC_CMD_TX_SET_FINGER_CAP,
} PSoC_CMD_RX;
typedef enum {
    PSoC_CMD_DEBUG_MASTER_TOUCH_TH = 0,
    PSoC_CMD_DEBUG_SLAVE_TOUCH_TH,
    PSoC_CMD_DEBUG_MASTER_FINGER_TH,
    PSoC_CMD_DEBUG_SLAVE_FINGER_TH,
    PSoC_CMD_DEBUG_MASTER_HYSTERESIS,
    PSoC_CMD_DEBUG_SLAVE_HYSTERESIS,
} PSoC_CMD_DEBUG;

// Difference between the raw count value and the baseline, from SmartSense
extern uint16_t gu16PSoCDiff[32];

// Has the difference data changed?
extern volatile uint8_t bPSoCDirty;

// Used for producing gu32PSoCDigital and gu16PSoCDigital
#define PSoC_INTERNAL_DIGITAL_TH 0

// PSoC data, re-interpreted as digital inputs
extern uint32_t gu32PSoCDigital;
extern uint32_t gu32PSoCDigitalPos;
extern uint32_t gu32PSoCDigitalNeg;
extern uint32_t gu32PSoCDigitalTrig;
extern uint16_t gu16PSoCDigital;
extern uint16_t gu16PSoCDigitalPos;
extern uint16_t gu16PSoCDigitalNeg;
extern uint16_t gu16PSoCDigitalTrig;

extern uint8_t gu8PSoCSeenData;

#define PSoC_RAW_T0 34
#define PSoC_RAW_T1 91
#define PSoC_RAW_T2 121
#define PSoC_RAW_MIN 199
#define PSoC_SCALE 5  // ie to scale from [0~4ff] to [0~ff]

#define PSoC_OUT_MIN 20
#define PSoC_OUT_MAX 255

#define PSoC_SCALE_MIN (0 + 128)
#define PSoC_SCALE_MAX (0x4ff - 128 - 256)
#define PSoC_SCALE_RANGE (3)  // ie to scale from [min~max] to [0~ff]

void PSoC_PostProcessing(void);
void PSoC_DigitalCalc(void);

// General usage PSoC commands; return instantly
void PSoC_SetFingerCapacitance(uint16_t u16FingerCap);
void PSoC_SetFingerCapacitanceFromConfig(void);
/**
 * @brief Get the finger capacitance value currently configured on the Master PSoC
 *
 * Warning! This function is BLOCKING. Calls are likely to take at least 15ms
 */
uint16_t PSoC_GetFingerCapacitance(void);
/**
 * @brief Request a debug array from the PSoC pair
 *
 * This call is asynchronous. Use pu8Ready to ensure pu8Data is ready.
 * Only one call can be in-flight at once.
 * **DO NOT** call PSoCGetFingerCapacitance while waiting on this function.
 *
 * @param u8Cmd The specific debug array to request
 * @param pu8Data Pointer to array that will receive the data. Must be 32 bytes.
 * @param pu8Ready Optional uint8_t to signal the data is ready
 */
void PSoC_GetDebug(PSoC_CMD_DEBUG u8Cmd, uint8_t* pu8Data, volatile uint8_t* pu8Ready);
void PSoC_EnableDebug(uint8_t u8D1, uint8_t u8D2);
