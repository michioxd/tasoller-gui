#include "tasoller.h"

#define HID_IO4_BUF ((uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP_HID_IO4_IN)))
#define HID_MISC_BUF ((uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP_HID_MISC_IN)))

static const uint8_t u8GroundThreshold = 20;
static const uint8_t u8GroundKeymap[32] = {
    KEY_I, KEY_COMMA,  //
    KEY_8, KEY_K,      //
    KEY_U, KEY_M,      //
    KEY_7, KEY_J,      //
    KEY_Y, KEY_N,      //
    KEY_6, KEY_H,      //
    KEY_T, KEY_B,      //
    KEY_5, KEY_G,      //
    KEY_R, KEY_V,      //
    KEY_4, KEY_F,      //
    KEY_E, KEY_C,      //
    KEY_3, KEY_D,      //
    KEY_W, KEY_X,      //
    KEY_2, KEY_S,      //
    KEY_Q, KEY_Z,      //
    KEY_1, KEY_A,      //
};
static const uint8_t u8AirKeymap[6] = {
    HID_KEYBOARD_SLASH_AND_QUESTION_MARK,              // VK_OEM_2
    HID_KEYBOARD_PERIOD_AND_GREATER_THAN,              // VK_OEM_PERIOD
    HID_KEYBOARD_QUOTE_AND_DOUBLEQUOTE,                // VK_OEM_7
    HID_KEYBOARD_SEMICOLON_AND_COLON,                  // VK_OEM_1
    HID_KEYBOARD_RIGHT_BRACKET_AND_RIGHT_CURLY_BRACE,  // VK_OEM_6
    HID_KEYBOARD_LEFT_BRACKET_AND_LEFT_CURLY_BRACE,    // VK_OEM_4
};
static inline void _HID_Keyboard_Tick(uint8_t bReal) {
    hid_report_t *buf = (hid_report_t *)HID_MISC_BUF;

    memset(buf, 0, sizeof *buf);
    buf->bReportId = HID_REPORT_ID_KEYBOARD;
    uint8_t kI = 0;

    if (bReal) {
        if (gu8DigitalButtons & 0x01) buf->bKeyboard[kI++] = HID_KEYBOARD_F1;
        if (gu8DigitalButtons & 0x02) buf->bKeyboard[kI++] = HID_KEYBOARD_F2;
        if (gu8DigitalButtons & 0x04) buf->bKeyboard[kI++] = u8AirKeymap[0];
        if (gu8DigitalButtons & 0x08) buf->bKeyboard[kI++] = u8AirKeymap[1];
        if (gu8DigitalButtons & 0x10) buf->bKeyboard[kI++] = u8AirKeymap[2];
        if (gu8DigitalButtons & 0x20) buf->bKeyboard[kI++] = u8AirKeymap[3];
        if (gu8DigitalButtons & 0x40) buf->bKeyboard[kI++] = u8AirKeymap[4];
        if (gu8DigitalButtons & 0x80) buf->bKeyboard[kI++] = u8AirKeymap[5];

        for (int i = 0; i < 32; i++) {
            if (gu8GroundData[i] > u8GroundThreshold) buf->bKeyboard[kI++] = u8GroundKeymap[i];
        }
    }

    USBD_SET_PAYLOAD_LEN(EP_HID_MISC_IN, sizeof *buf);
}

#define IO4_BUTTON_TEST (1 << 9)
#define IO4_BUTTON_SERVICE (1 << 6)

#define IO4_CMD_SET_COMM_TIMEOUT 0x01
#define IO4_CMD_SET_SAMPLING_COUNT 0x02
#define IO4_CMD_CLEAR_BOARD_STATUS 0x03
#define IO4_CMD_SET_GENERAL_OUTPUT 0x04
#define IO4_CMD_SET_PWM_OUTPUT 0x05
#define IO4_CMD_SET_UNIQUE_OUTPUT 0x41
#define IO4_CMD_UPDATE_FIRMWARE 0x85

volatile uint8_t u8IO4SystemStatus = 0;
volatile uint8_t u8IO4USBStatus = 0;
volatile uint16_t u16IO4CommTimeout = 0;
volatile uint8_t u8IO4SamplingCount = 0;

static void _HID_IO4_Prepare(volatile uint8_t *pu8EpBuf) {
    io4_hid_in_t *buf = (io4_hid_in_t *)pu8EpBuf;

    memset(buf, 0, sizeof *buf);
    buf->bReportId = HID_REPORT_ID_IO4;

    // System buttons
    if (gu8DigitalButtons & 0x01) buf->wButtons[0] |= IO4_BUTTON_TEST;
    if (gu8DigitalButtons & 0x02) buf->wButtons[0] |= IO4_BUTTON_SERVICE;
    // Airs
    if (!(gu8DigitalButtons & 0x04)) buf->wButtons[0] |= 1 << 13;
    if (!(gu8DigitalButtons & 0x08)) buf->wButtons[1] |= 1 << 13;
    if (!(gu8DigitalButtons & 0x10)) buf->wButtons[0] |= 1 << 12;
    if (!(gu8DigitalButtons & 0x20)) buf->wButtons[1] |= 1 << 12;
    if (!(gu8DigitalButtons & 0x40)) buf->wButtons[0] |= 1 << 11;
    if (!(gu8DigitalButtons & 0x80)) buf->wButtons[1] |= 1 << 11;

    buf->bUsbStatus = u8IO4USBStatus;
    buf->bSystemStatus = u8IO4SystemStatus;
}
static void _HID_IO4_Tick() {
    _HID_IO4_Prepare(HID_IO4_BUF);

    // We must send data every 8ms! None of that "only sending changed keys" stuff
    // Trigger a write
    gu8HIDIO4Ready = 0;
    USBD_SET_PAYLOAD_LEN(EP_HID_IO4_IN, sizeof(io4_hid_in_t));
}
static void _HID_Debug_Tick() {
    debug_hid_report_t *buf = (debug_hid_report_t *)HID_MISC_BUF;
    memset(buf, 0, sizeof *buf);

    static uint8_t bWhich = 0;
    if ((bWhich++) & 1) {
        buf->bReportId = HID_REPORT_ID_DEBUG_A;
        for (uint8_t i = 0; i < 32; i += 2) buf->wData[i / 2] = gu8GroundData[i];
    } else {
        buf->bReportId = HID_REPORT_ID_DEBUG_B;
        for (uint8_t i = 1; i < 32; i += 2) buf->wData[i / 2] = gu8GroundData[i];
    }
    USBD_SET_PAYLOAD_LEN(EP_HID_MISC_IN, sizeof *buf);
}

static void _HID_Misc_Tick() {
    static uint8_t sbLastEnableKeyboard = 0;
    if (gConfig.bEnableKeyboard) {
        sbLastEnableKeyboard = 1;
        _HID_Keyboard_Tick(1);
    } else if (sbLastEnableKeyboard) {
        // If we've just disabled the keyboard, make sure to send a packet with all keys released!
        sbLastEnableKeyboard = 0;
        _HID_Keyboard_Tick(0);
    }
    // TODO: gbEnableDebug (we'll need to use a toggle to alternate)

    gu8HIDMiscReady = 0;
}

void USBD_HID_PrepareReport() {
    if (gu8HIDIO4Ready) _HID_IO4_Tick();
    if (gu8HIDMiscReady) _HID_Misc_Tick();
}
static uint8_t sIO4InBuffer[sizeof(io4_hid_in_t)] = { 0 };
uint8_t *USBD_HID_GetReport(uint8_t u8ReportId, uint32_t *pu32Size) {
    switch (u8ReportId) {
        case HID_REPORT_ID_IO4:
            if (!gConfig.bEnableIO4) return NULL;
            _HID_IO4_Prepare(sIO4InBuffer);
            *pu32Size = sizeof sIO4InBuffer;
            return sIO4InBuffer;
        default:
            return NULL;
    }
}
void USBD_HID_SetReport(volatile uint8_t *pu8EpBuf, uint32_t u32Size) {
    // TODO: is pu8EpBuf[0] the report ID?
    // We need to switch on that report ID so we know what we're doing!
    if (!gConfig.bEnableIO4) return;

    if (u32Size < 2) return;
    switch (pu8EpBuf[1]) {
        case IO4_CMD_SET_COMM_TIMEOUT:
            if (u32Size >= 2 + 1) {
                u16IO4CommTimeout = (uint16_t)pu8EpBuf[2] * 200;
                u8IO4SystemStatus |= 0x10;

                gu8HIDIO4Ready = 1;
                _HID_IO4_Tick();
            }
            break;
        case IO4_CMD_SET_SAMPLING_COUNT:
            if (u32Size >= 2 + 1) {
                u8IO4SamplingCount = pu8EpBuf[2];
                u8IO4SystemStatus |= 0x20;

                gu8HIDIO4Ready = 1;
                _HID_IO4_Tick();
            }
            break;
        case IO4_CMD_CLEAR_BOARD_STATUS:
            u8IO4SystemStatus &= 0x0F;
            u8IO4USBStatus &= 0x04;

            gu8HIDIO4Ready = 1;
            _HID_IO4_Tick();
            break;
        case IO4_CMD_SET_GENERAL_OUTPUT:
            if (u32Size >= 2 + 3) {
                // 20 bits of data for GPO (+4 of padding)
            }
            break;
        case IO4_CMD_SET_PWM_OUTPUT:
            if (u32Size >= 2 + 0) {
                // 0 bytes of data for PWM duty cycles (IO4 has no PWM!)
            }
            break;
        case IO4_CMD_SET_UNIQUE_OUTPUT:
            if (u32Size >= 2 + 62) {
                // 62 bytes of unique output data
            }
            break;
        case IO4_CMD_UPDATE_FIRMWARE:
            break;
    }
}
