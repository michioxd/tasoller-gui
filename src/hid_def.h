#pragma once

#include <stdint.h>

#include "_compiler.h"

// Triggers to start sending data
extern uint8_t volatile gu8HIDIO4Ready;
extern uint8_t volatile gu8HIDMiscReady;

extern uint16_t u16RequestedConsumerControl;
extern uint32_t u32EnterPressStarted;

enum {
    HID_REPORT_ID_IO4 = 1,
    HID_REPORT_ID_KEYBOARD,
    HID_REPORT_ID_CONSUMER_CONTROL,
    HID_REPORT_ID_ENTER,
    HID_REPORT_ID_TOUCH,
    HID_REPORT_ID_IO4_CMD = 16,
};

#define NUM_FN 2
#define NUM_AIR 6
#define NUM_GROUND 32

typedef struct __packed {
    uint8_t bReportId;
    uint8_t bKeyboard[NUM_FN + NUM_AIR + NUM_GROUND];
} hid_kbd_report_t;

typedef struct __packed {
    uint8_t bReportId;
    uint16_t u16Control[2];
} hid_consumer_report_t;

typedef struct __packed {
    uint8_t bReportId;
    uint8_t u8Keyboard[1];
} hid_enter_report_t;
typedef struct __packed {
    uint8_t bReportId;

    /*
    uint8_t bTipSwitch : 1;
    uint8_t _1_3 : 3;
    uint8_t bInRange : 1;
    // uint8_t bConfidence : 1;
    uint16_t _5_11 : 11;
    uint16_t wX;
    uint16_t wY;
    uint16_t wWidth;
    uint16_t wHeight;
    uint16_t _80_16 : 16;
    */

    // uint8_t bTipSwitch : 1;
    // uint8_t bInRange : 1;
    // uint8_t _2_6 : 6;
    // uint8_t _8;
    // uint16_t wX;
    // uint16_t wY;

    struct {
        uint8_t bTipSwitch : 1;
        uint8_t _1_7 : 1;
        uint8_t bIdentifier;
        uint8_t bX;
        uint8_t bY;
        uint8_t bW;
        uint8_t bH;
    } sFinger[8];
    uint8_t bContactCount;
} hid_touch_report_t;

typedef struct __packed {
    uint8_t bReportId;
    uint16_t wADC[8];
    uint16_t wRotary[4];
    uint16_t wCoin[2];
    uint16_t wButtons[2];
    uint8_t bSystemStatus;
    uint8_t bUsbStatus;
    uint8_t bUnique[29];
} io4_hid_in_t;

typedef struct __packed {
    uint8_t bReportId;
    uint8_t bCmd;
    uint8_t bData[62];
} io4_hid_out_t;

// void HID_Tick();
void USBD_HID_PrepareReport();
uint8_t *USBD_HID_GetReport(uint8_t u8ReportId, uint32_t *pu32Size);
void USBD_HID_SetReport(volatile uint8_t *pu8EpBuf, uint32_t u32Size);
