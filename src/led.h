#pragma once

#include "led_shared.h"

// Defined as volatile due to the I2C interrupt reading at random times!
extern volatile uint8_t gu8LEDTx[LED_PACKET_MAX_SIZE];

// Internal LED control, from the settings UI (gets priority)
extern hsv_t gaControlledIntLedData[LED_NUM_GROUND_LOGICAL];
extern uint8_t gbLedDataIsControlledInt;
// External LED control, from the game
extern rgb_t gaControlledExtLedData[32];
extern uint8_t gbLedDataIsControlledExt;

// For reception of data from I2C
extern volatile uint8_t* gpu8I2CRx;
extern volatile uint16_t u16I2CRxIndex;

extern uint8_t gbLedIsCustom;

// === Assigners (led_impl.c) ===
void LED_Wings_Reactive_HSV(_led_wings_hsv* pWings);
void LED_Ground_Rainbow_HSV(hsv_t* aGround);
void LED_Ground_Static_HSV(hsv_t* aGround);
void LED_Ground_Internal_HSV(hsv_t* aGround);
/** All "controlled" data is in RGB format, so these aren't a thing
 * void LED_Wings_Controlled_HSV(_led_wings_hsv* pWings);
 * void LED_Ground_Controlled_HSV(hsv_t* aGround);
 */

// Unimplemented nonsense stuff
void LED_Wings_Reactive_RGB(_led_wings_rgb* pWings);
void LED_Ground_Rainbow_RGB(rgb_t* pWings);
void LED_Ground_Internal_RGB(rgb_t* aGround);
void LED_Ground_Static_RGB(rgb_t* aGround);
// Actual RGBs that're used
void LED_Wings_Controlled_RGB(_led_wings_rgb* pWings);
void LED_Ground_Controlled_RGB(rgb_t* aGround);

// === Exported functions ===
void HsvToHost(rgb_t* pRGB, uint16_t u16H, uint8_t u8S, uint8_t u8V);
void LED_I2C1_Init(void);

/**
 * Write data out to the LEDs
 */
void LED_Write(void);

uint8_t LED_FMC_Read(uint32_t u32Offset, uint32_t* pu32Data);
