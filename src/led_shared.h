/**
 * !!WARNING!!
 * This file is included into the LED APROM firmware too.
 * Don't place anything in here that would be inappropriate there.
 * !!WARNING!!
 */

#pragma once

#include <stdint.h>

#include "_compiler.h"

// Are we going to be doing global colour correction on the LED MCU (yes!)
#define LED_CORRECTION_ON_LED_MCU

// === LED Firmware Type Configuration ===
// (Only applicable on the host)
#define LED_FW_STOCK 0
#define LED_FW_MAINLAND 1
#define LED_FW_OURS 2

// #define LED_FIRMWARE_TYPE LED_FW_STOCK
// #define LED_FIRMWARE_TYPE LED_FW_MAINLAND
#define LED_FIRMWARE_TYPE LED_FW_OURS

// === LED Count Definitions ===
/**
 * There are 48 LEDs for ground, but we only send 31 values
 * They're mapped to the LEDs as follows:
 *   00a11b22c33d44f55g66h77i...
 *
 * That is, we can't split-colour a key :P
 */
#define LED_NUM_GROUND_LOGICAL 31
// Ground: [0] = Obscured, [1] = Right, [47] = Left
#define LED_NUM_GROUND_PHYSICAL 48
// Right tower: [0] = Bottom, [23] = Top
// Left tower: [23] = Bottom, [0] = Top
#define LED_NUM_TOWER 24

// === Colour Byte Order Definitions ===
// What we know and love
#define RGB_R 0
#define RGB_G 1
#define RGB_B 2
// What WS2813 uses, and what we receive from the host in stock mode
#define GRB_R 1
#define GRB_G 0
#define GRB_B 2
// Our internal reversed WS2813 format
#define BRG_R 1
#define BRG_G 2
#define BRG_B 0

#define HOST_FORMAT GRB
#define _CHANNEL(fmt, c) fmt##_##c
#define CHANNEL(fmt, c) _CHANNEL(fmt, c)

#define HOST_R CHANNEL(HOST_FORMAT, R)
#define HOST_G CHANNEL(HOST_FORMAT, G)
#define HOST_B CHANNEL(HOST_FORMAT, B)

typedef union {
    struct __packed {
        uint8_t r;
        uint8_t g;
        uint8_t b;
    } rgb;
    // Data received from the game is in BRG format
    struct __packed {
        uint8_t b;
        uint8_t r;
        uint8_t g;
    } game;
    // WS2813 uses GRB, and that's what we received from the host
    struct __packed {
        uint8_t g;
        uint8_t r;
        uint8_t b;
    } host;
    // Internally we use a reversed WS2813 format (BRG)
    struct __packed {
        uint8_t b;
        uint8_t r;
        uint8_t g;
    } internal;
    uint8_t aRaw[3];
} rgb_t;
typedef struct __packed {
    uint16_t h;
    uint8_t s;
    uint8_t v;
} hsv_t;

// === Host->LED MCU Commands ===
#define LED_I2C_REG_PACKET 0xFF

#define LED_CMD_BASIC 0xA5
#define LED_CMD_RGB_FULL 0x5A
#define LED_CMD_CUSTOM_RGB 0x5B
#define LED_CMD_CUSTOM_HSV 0x5C
#define LED_CMD_CUSTOM_MIXED 0x5D
#define LED_CMD_FMC_READ 0x5E
#define LED_CMD_FMC_ENTER_LDROM 0x5F

// === HOST->LED MCU Packets ===
typedef struct __packed {
    rgb_t aTowerL[LED_NUM_TOWER];
    rgb_t aTowerR[LED_NUM_TOWER];
} _led_towers_rgb;
typedef struct __packed {
    hsv_t aTowerL[LED_NUM_TOWER];
    hsv_t aTowerR[LED_NUM_TOWER];
} _led_towers_hsv;

typedef struct __packed {
    uint8_t u8Cmd;
    uint32_t u32Ground;

    uint8_t u8TowerFill;
#if LED_FIRMWARE_TYPE == LED_FW_MAINLAND
    uint8_t u8Rainbow;
    uint8_t u8Rsv07;
    uint8_t u8Rsv08;
    uint8_t u8Rsv09;
#else
    uint8_t u8HueLeft;
    uint8_t u8HueRight;
    uint8_t u8HueGround;
    uint8_t u8HueGroundActive;
#endif
    uint8_t u8Flags0;
    uint8_t u8Flags1;
} led_rx_basic, *Pled_rx_basic;

typedef struct __packed {
    uint8_t u8Cmd;
#if LED_FIRMWARE_TYPE == LED_FW_MAINLAND
    uint8_t u8Config;
#endif
    rgb_t aGround[LED_NUM_GROUND_LOGICAL];
    _led_towers_rgb Towers;
} led_rx_full, *Pled_rx_full;

typedef struct __packed {
    uint8_t u8Cmd;
    uint8_t u8GroundBrightness;
    uint8_t u8TowerBrightness;
    rgb_t aGround[LED_NUM_GROUND_LOGICAL];
    _led_towers_rgb Towers;
} led_rx_custom_rgb, *Pled_rx_custom_rgb;

typedef struct __packed {
    uint8_t u8Cmd;
    uint8_t u8GroundBrightness;
    uint8_t u8TowerBrightness;
    rgb_t aGround[LED_NUM_GROUND_LOGICAL];
    _led_towers_hsv Towers;
} led_rx_custom_mixed, *Pled_rx_custom_mixed;

typedef struct __packed {
    uint8_t u8Cmd;
    uint8_t u8GroundBrightness;
    uint8_t u8TowerBrightness;
    hsv_t aGround[LED_NUM_GROUND_LOGICAL];
    _led_towers_hsv Towers;
} led_rx_custom_hsv, *Pled_rx_custom_hsv;

typedef struct __packed {
    uint8_t u8Cmd;
    uint32_t u32Offset;
} led_rx_fmc_read, *Pled_rx_fmc_read;

#define LED_PACKET_MAX_SIZE                                                         \
    Maximum(Maximum(Maximum(sizeof(led_rx_basic), sizeof(led_rx_full)),             \
                    Maximum(sizeof(led_rx_custom_rgb), sizeof(led_rx_custom_hsv))), \
            sizeof(led_rx_custom_mixed))

// === LED index names ===
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

// === HSV Definitions ===
#define LED_HUE_MAX 360
