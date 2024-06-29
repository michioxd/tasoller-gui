#pragma once

#include <stdint.h>

#define LED_FIRMWARE_CFW
#ifndef LED_FIRMWARE_CFW
#define LED_DATA_OFFSET 1
#else
#define LED_DATA_OFFSET 2
#endif

// LED count definitions
#define LED_NUM_GROUND 31
#define LED_NUM_LEFT 24
#define LED_NUM_RIGHT 24

// HSV definitions
#define LED_HUE_SCALE 5  // For transmission to LED board in basic mode
#define LED_HUE_MAX 360

// Host->LED MCU commands
#define LED_CMD_BASIC 0xA5
#define LED_CMD_RGB_FULL 0x5A

// LED index names
#define LED_CELL_0 0
#define LED_CELL_1 2
#define LED_CELL_2 4
#define LED_CELL_3 6
#define LED_CELL_4 8
#define LED_CELL_5 10
#define LED_CELL_6 12
#define LED_CELL_7 14
#define LED_CELL_8 16
#define LED_CELL_9 18
#define LED_CELL_10 20
#define LED_CELL_11 22
#define LED_CELL_12 24
#define LED_CELL_13 26
#define LED_CELL_14 28
#define LED_CELL_15 30

#define LED_DIVIDER_0_1 1
#define LED_DIVIDER_1_2 3
#define LED_DIVIDER_2_3 5
#define LED_DIVIDER_3_4 7
#define LED_DIVIDER_4_5 9
#define LED_DIVIDER_5_6 11
#define LED_DIVIDER_6_7 13
#define LED_DIVIDER_7_8 15
#define LED_DIVIDER_8_9 17
#define LED_DIVIDER_9_10 19
#define LED_DIVIDER_10_11 21
#define LED_DIVIDER_11_12 23
#define LED_DIVIDER_12_13 25
#define LED_DIVIDER_13_14 27
#define LED_DIVIDER_14_15 29

typedef struct {
    uint16_t u16H;
    uint8_t u8S;
    uint8_t u8V;
} hsv_t;
// Internal LED control, from the settings UI (gets priority)
extern hsv_t gaControlledIntLedData[LED_NUM_GROUND];
extern uint8_t gbLedDataIsControlledInt;
// External LED control, from the game
extern uint8_t gu8aControlledExtLedData[32 * 3];
extern uint8_t gbLedDataIsControlledExt;

void LED_I2C1_Init(void);
void LED_WriteBasicGrounds(void);
void LED_WriteRGB(void);
