#pragma once

#include <stdint.h>

#define IO4_BUTTON_TEST (1 << 9)
#define IO4_BUTTON_SERVICE (1 << 6)

#define IO4_CMD_SET_COMM_TIMEOUT 0x01
#define IO4_CMD_SET_SAMPLING_COUNT 0x02
#define IO4_CMD_CLEAR_BOARD_STATUS 0x03
#define IO4_CMD_SET_GENERAL_OUTPUT 0x04
#define IO4_CMD_SET_PWM_OUTPUT 0x05
#define IO4_CMD_SET_UNIQUE_OUTPUT 0x41
#define IO4_CMD_84 0x84
#define IO4_CMD_UPDATE_FIRMWARE 0x85
#define IO4_CMD_88 0x88  // data[0] = D9

extern volatile uint8_t gu8IO4PWMScale;
extern volatile uint8_t gu8IO4PWMOutput[6];
extern volatile uint8_t gu8IO4DigitalOutput[3];

extern uint16_t gu16IO4ForceButtons;

static inline uint8_t IO4_GetCoinBlocker(void) {
    // Chunithm's coin blocker is wired to OUT1
    return (gu8IO4DigitalOutput[0] & 0x80) ? 0 : 1;
}

void IO4_HID_Prepare(volatile uint8_t *pu8EpBuf);
void IO4_HID_Tick(void);
void IO4_Control(uint8_t u8Cmd, uint32_t u32Size, volatile uint8_t *pu8Buffer);
