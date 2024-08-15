#include "tasoller.h"

#define HID_IO4_BUF ((uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP_HID_IO4_IN)))

volatile uint8_t u8IO4SystemStatus = 0;
volatile uint8_t u8IO4USBStatus = 0;
volatile uint16_t u16IO4CommTimeout = 0;
volatile uint8_t u8IO4SamplingCount = 0;

volatile uint16_t u16IO4Coins[2];

volatile uint8_t gu8IO4PWMScale;
volatile uint8_t gu8IO4PWMOutput[6];
volatile uint8_t gu8IO4DigitalOutput[3];  // 20 bits

uint16_t gu16IO4ForceButtons = 0;
void IO4_HID_Prepare(volatile uint8_t *pu8EpBuf) {
    io4_hid_in_t *buf = (io4_hid_in_t *)pu8EpBuf;

    memset(buf, 0, sizeof *buf);
    buf->bReportId = HID_REPORT_ID_IO4;

    // System buttons
    buf->wButtons[0] = gu16IO4ForceButtons;
    // if (gu8DigitalButtons & DIGITAL_FN2_Msk) buf->wButtons[0] |= IO4_BUTTON_TEST;
    // if (gu8DigitalButtons & DIGITAL_FN1_Msk) buf->wButtons[0] |= IO4_BUTTON_SERVICE;
    // Airs
    if (!(gu8DigitalButtons & 0x04)) buf->wButtons[0] |= 1 << 13;
    if (!(gu8DigitalButtons & 0x08)) buf->wButtons[1] |= 1 << 13;
    if (!(gu8DigitalButtons & 0x10)) buf->wButtons[0] |= 1 << 12;
    if (!(gu8DigitalButtons & 0x20)) buf->wButtons[1] |= 1 << 12;
    if (!(gu8DigitalButtons & 0x40)) buf->wButtons[0] |= 1 << 11;
    if (!(gu8DigitalButtons & 0x80)) buf->wButtons[1] |= 1 << 11;

    buf->bUsbStatus = u8IO4USBStatus;
    buf->bSystemStatus = u8IO4SystemStatus;

    // Pos-edge trigger
    static uint8_t su8LastDigitalButtons = 0;
    if ((gu8DigitalButtons & ~su8LastDigitalButtons) & DIGITAL_FN1_Msk) {
        if (!IO4_GetCoinBlocker()) {
            u16IO4Coins[0]++;
        }
    }
    su8LastDigitalButtons = gu8DigitalButtons;

    buf->wCoin[0] = BYTESWAP_U16(u16IO4Coins[0]);
    buf->wCoin[1] = BYTESWAP_U16(u16IO4Coins[1]);
}

void IO4_HID_Tick(void) {
    IO4_HID_Prepare(HID_IO4_BUF);

    // We must send data every 8ms! None of that "only sending changed keys" stuff
    // Trigger a write
    gu8HIDIO4Ready = 0;
    USBD_SET_PAYLOAD_LEN(EP_HID_IO4_IN, sizeof(io4_hid_in_t));
}

void IO4_Control(uint8_t u8Cmd, uint32_t u32Size, volatile uint8_t *pu8Buffer) {
    switch (u8Cmd) {
        case IO4_CMD_SET_COMM_TIMEOUT:
            if (u32Size >= 1) {
                u16IO4CommTimeout = (uint16_t)pu8Buffer[0] * 200;
                u8IO4SystemStatus |= 0x10;

                gu8HIDIO4Ready = 1;
                IO4_HID_Tick();
            }
            break;
        case IO4_CMD_SET_SAMPLING_COUNT:
            if (u32Size >= 1) {
                u8IO4SamplingCount = pu8Buffer[0];
                u8IO4SystemStatus |= 0x20;

                gu8HIDIO4Ready = 1;
                IO4_HID_Tick();
            }
            break;
        case IO4_CMD_CLEAR_BOARD_STATUS:
            u8IO4SystemStatus &= 0x0F;
            u8IO4USBStatus &= 0x04;

            gu8HIDIO4Ready = 1;
            IO4_HID_Tick();
            break;
        case IO4_CMD_SET_GENERAL_OUTPUT:
            // 20 bits of data for GPO (+4 bits of padding)
            if (u32Size >= 3) {
                gu8IO4DigitalOutput[0] = pu8Buffer[0];
                gu8IO4DigitalOutput[1] = pu8Buffer[1];
                gu8IO4DigitalOutput[2] = pu8Buffer[2];
            }
            break;
        case IO4_CMD_SET_PWM_OUTPUT:
            if (u32Size >= 0) {
                // 0 bytes of data for PWM duty cycles (IO4 has no PWM!)
            }
            break;
        case IO4_CMD_SET_UNIQUE_OUTPUT:
            // 62 bytes of unique output data
            if (u32Size >= 62) {
                // [0]: Enable mask (bit 7=PWM1, 2=PWM6, 0~1 unused)
                // [1]: Brightness scaler (1~256, with 0=256)
                // [2]: PWM1 brightness (pin CN3.55)
                // [3]: PWM2 brightness (pin CN3.56)
                // [4]: PWM3 brightness (pin CN9.5 )
                // [5]: PWM4 brightness (pin CN9.6 )
                // [6]: PWM5 brightness (pin CN9.9 )
                // [7]: PWM6 brightness (pin CN9.10)

                // For maimai DX:
                //   [0]: 11111100 (Enable PWM1~6)
                //   [1]: 00
                //   [2]: 1P Billboard Red
                //   [3]: 2P Billboard Red
                //   [4]: 1P Billboard Green
                //   [5]: 2P Billboard Green
                //   [6]: 1P Billboard Blue
                //   [7]: 2P Billboard Blue
                //
                // For chunithm:
                //   [0]: 00111000 (Enable PWM3~5)
                //   [1]: 00
                //   [2]: Unused
                //   [3]: Unused
                //   [4]: Light gate Blue
                //   [5]: Light gate Red
                //   [6]: Light gate Green
                //   [7]: Unused

                uint8_t u8Mask = pu8Buffer[0];
                if (pu8Buffer[1] == 0)
                    gu8IO4PWMScale = 255;
                else
                    gu8IO4PWMScale = pu8Buffer[1] - 1;

                for (uint8_t i = 0; i < 6; i++) {
                    gu8IO4PWMOutput[i] = (u8Mask & (1 << (7 - i))) ? pu8Buffer[2 + i] : 0;
                }
            }
            break;
        case IO4_CMD_UPDATE_FIRMWARE:
            break;
    }
}
