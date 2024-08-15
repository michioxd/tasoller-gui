#pragma once

#include <stdint.h>

#define PSoC_DATA_SIZE 32
#define PSoC_PACKET_SIZE (2 + PSoC_DATA_SIZE)

// The difference between CAP and MIN must be divisible by 8
// The stupidly high gain kicks in around 0.7pF. It's not directly based on fingerCap though
// so 1.0pF gives us a bit of a margin to ensure we avoid it
#define PSoC_FINGER_CAP_MIN 100  // 1.0pF
#define PSoC_DEFAULT_CAP 500     // 5.0pF
// Maximum is CAP*2-MIN*2 ie 8.0pF

#define PSoC_FINGER_CAP_STEP ((PSoC_DEFAULT_CAP - PSoC_FINGER_CAP_MIN) / 8)

typedef enum : uint8_t {
    // Host -> PSoC
    PSoC_CMD_TX_GET_FINGER_CAP = 0xB5,
    PSoC_CMD_TX_GET_DEBUG = 0xB6,
    PSoC_CMD_TX_SET_FINGER_CAP = 0xBD,
    PSoC_CMD_TX_ENABLE_DEBUG = 0xDE,
} PSoC_CMD_TX;
typedef enum : uint8_t {
    // We'd use 0x00, but that's used, so 0x01 it is
    _PSoC_CMD_RX_NONE = 0x01,

    // PSoC -> Host
    PSoC_CMD_RX_MASTER_DIFF = 0xAD,
    PSoC_CMD_RX_SLAVE_DIFF = 0xAF,
    PSoC_CMD_RX_INITIALISATION_COMPLETE = 0xC1,

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
    PSoC_CMD_RX_ENABLE_DEBUG = PSoC_CMD_TX_ENABLE_DEBUG,
} PSoC_CMD_RX;
typedef enum : uint8_t {
    PSoC_CMD_DEBUG_MASTER_TOUCH_TH = 0,
    PSoC_CMD_DEBUG_SLAVE_TOUCH_TH,
    PSoC_CMD_DEBUG_MASTER_FINGER_TH,
    PSoC_CMD_DEBUG_SLAVE_FINGER_TH,  // Broken!
    PSoC_CMD_DEBUG_MASTER_HYSTERESIS,
    PSoC_CMD_DEBUG_SLAVE_HYSTERESIS,  // Broken!
} PSoC_CMD_DEBUG;

// Difference between the raw count value and the baseline, from SmartSense
extern uint16_t gu16PSoCDiff[32];

// Has the difference data changed in the interrupt handler?
extern volatile uint8_t bPSoCDirtyVolatile;
// Have we received indication that the PSoC is ready for packets?
extern volatile uint8_t bPSoCAliveVolatile;
// Has the post-processed difference data changed?
extern volatile uint8_t bForceSliderSend;

// Used for producing gu32PSoCDigital and gu16PSoCDigital
// (Same threshold as default in Chunithm)
#define PSoC_INTERNAL_DIGITAL_TH 20

// PSoC data, re-interpreted as digital inputs
extern uint32_t gu32PSoCDigital;
extern uint32_t gu32PSoCDigitalPos;
extern uint32_t gu32PSoCDigitalNeg;
extern uint32_t gu32PSoCDigitalTrig;
extern uint16_t gu16PSoCDigital;
extern uint16_t gu16PSoCDigitalPos;
extern uint16_t gu16PSoCDigitalNeg;
extern uint16_t gu16PSoCDigitalTrig;

//
extern uint32_t gu32LastCapSenseStart;
extern uint32_t gu32LastCapSenseEnd;

void PSoC_PostProcessing(void);
void PSoC_DigitalCalc(void);

// General usage PSoC commands
void PSoC_SetFingerCapacitance(uint16_t u16FingerCap, uint8_t u8Blocking);
void PSoC_SetFingerCapacitanceFromConfig(uint8_t u8Blocking);
/**
 * @brief Get the finger capacitance value currently configured on the Master PSoC
 *
 * Warning! This function is BLOCKING. Calls are likely to take at least 15ms
 */
uint16_t PSoC_GetFingerCapacitance(void);
/**
 * @brief Request a debug array from the PSoC pair
 *
 * @param u8Cmd The specific debug array to request
 * @param pu8Data Pointer to array that will receive the data. Must be 32 bytes.
 */
void PSoC_GetDebug(PSoC_CMD_DEBUG u8Cmd, uint8_t* pu8Data);

typedef enum : uint8_t {
    // Flag 0 doesn't appear to be used anywhere, nor does it appear to have any effect
    PSoC_DebugFlag_0 = 0,
    PSoC_DebugFlag_TraceReset = 1,
} PSoC_DebugFlag;
/**
 * @brief Enable or disable a debug tracing flag
 *
 * The reception of debug traces will be recorded in:
 * - gu32LastCapSenseStart
 * - gu32LastCapSenseEnd
 *
 * @param eFlag Which flag to set
 * @param bEnable Should the flag be enabled or disabled
 */
void PSoC_SetDebug(PSoC_DebugFlag eFlag, uint8_t bEnable);
