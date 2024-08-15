#include "tasoller.h"

uint16_t u16RequestedConsumerControl = 0;
uint32_t u32EnterPressStarted = 0xFFFFFFFF;

#define ENTER_HOLD_TIME 5000  // 5 seconds

#define HID_MISC_BUF ((uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP_HID_MISC_IN)))
#define HID_IO4_BUF ((uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP_HID_IO4_IN)))

#define HID_MISC_SEND(buf)                                   \
    do {                                                     \
        USBD_SET_PAYLOAD_LEN(EP_HID_MISC_IN, sizeof *(buf)); \
        gu8HIDMiscReady = 0;                                 \
    } while (0)

static const uint8_t u8GroundKeymap[32] = {
    KEY_I, KEY_9,  // Dao uses: KEY_I, KEY_COMMA
    KEY_8, KEY_K,  //
    KEY_U, KEY_M,  //
    KEY_7, KEY_J,  //
    KEY_Y, KEY_N,  //
    KEY_6, KEY_H,  //
    KEY_T, KEY_B,  //
    KEY_5, KEY_G,  //
    KEY_R, KEY_V,  //
    KEY_4, KEY_F,  //
    KEY_E, KEY_C,  //
    KEY_3, KEY_D,  //
    KEY_W, KEY_X,  //
    KEY_2, KEY_S,  //
    KEY_Q, KEY_Z,  //
    KEY_1, KEY_A,  //
};
static const uint8_t u8AirKeymap[6] = {
    // Dao mapping:
    // HID_KEYBOARD_SLASH_AND_QUESTION_MARK,              // VK_OEM_2
    // HID_KEYBOARD_PERIOD_AND_GREATER_THAN,              // VK_OEM_PERIOD
    // HID_KEYBOARD_QUOTE_AND_DOUBLEQUOTE,                // VK_OEM_7
    // HID_KEYBOARD_SEMICOLON_AND_COLON,                  // VK_OEM_1
    // HID_KEYBOARD_RIGHT_BRACKET_AND_RIGHT_CURLY_BRACE,  // VK_OEM_6
    // HID_KEYBOARD_LEFT_BRACKET_AND_LEFT_CURLY_BRACE,    // VK_OEM_4

    // UMIGURI mapping:
    KEY_0, KEY_O, KEY_L, KEY_P, KEY_COMMA, KEY_PERIOD,
};

static uint8_t _HID_Keyboard_Tick(uint8_t bReleaseAll) {
    hid_kbd_report_t *buf = (hid_kbd_report_t *)HID_MISC_BUF;

    // Send a report of zeroes to release all keys
    if (bReleaseAll) {
        memset(buf, 0, sizeof *buf);
        buf->bReportId = HID_REPORT_ID_KEYBOARD;

        HID_MISC_SEND(buf);
        return 1;
    }

    static uint8_t u8LastButtons = 0;
    static uint32_t u32LastSlider = 0;

    // If nothing changed, do nothing
    if (gu8DigitalButtons == u8LastButtons && gu32PSoCDigital == u32LastSlider) {
        return 0;
    }

    memset(buf, 0, sizeof *buf);
    buf->bReportId = HID_REPORT_ID_KEYBOARD;
    uint8_t kI = 0;

    if (gu8DigitalButtons & DIGITAL_FN2_Msk) buf->bKeyboard[kI++] = HID_KEYBOARD_F1;
    if (gu8DigitalButtons & DIGITAL_FN1_Msk) buf->bKeyboard[kI++] = HID_KEYBOARD_F2;
    if (gu8DigitalButtons & 0x04) buf->bKeyboard[kI++] = u8AirKeymap[0];
    if (gu8DigitalButtons & 0x08) buf->bKeyboard[kI++] = u8AirKeymap[1];
    if (gu8DigitalButtons & 0x10) buf->bKeyboard[kI++] = u8AirKeymap[2];
    if (gu8DigitalButtons & 0x20) buf->bKeyboard[kI++] = u8AirKeymap[3];
    if (gu8DigitalButtons & 0x40) buf->bKeyboard[kI++] = u8AirKeymap[4];
    if (gu8DigitalButtons & 0x80) buf->bKeyboard[kI++] = u8AirKeymap[5];

    for (int i = 0; i < 32; i++) {
        if (gu32PSoCDigital & (1 << i)) buf->bKeyboard[kI++] = u8GroundKeymap[i];
    }

    HID_MISC_SEND(buf);
    u8LastButtons = gu8DigitalButtons;
    u32LastSlider = gu32PSoCDigital;
    return 1;
}
static uint8_t _HID_Consumer_Tick(void) {
    static uint16_t u16Last = 0;
    if (u16Last == u16RequestedConsumerControl) return 0;
    u16Last = u16RequestedConsumerControl;

    hid_consumer_report_t *buf = (hid_consumer_report_t *)HID_MISC_BUF;

    memset(buf, 0, sizeof *buf);
    buf->bReportId = HID_REPORT_ID_CONSUMER_CONTROL;
    buf->u16Control[0] = u16RequestedConsumerControl;
    HID_MISC_SEND(buf);
    return 1;
}

static uint8_t _HID_Enter_Tick(void) {
    // TODO: This isn't working, so we're using su8LastState for now instead
    if (u32EnterPressStarted == 0xFFFFFFFF) return 0;

    static uint8_t su8LastState = 0;
    uint8_t u8State = 0;

    // There's an _incredibly_ small chance the user tapped the cell at exactly the ms (49 days in!)
    // when the timer wrapped round to 0. This is too stupid to account for.
    if (u32EnterPressStarted && (MS_SINCE(u32EnterPressStarted) < ENTER_HOLD_TIME)) {
        u8State = 1;
    } else {
        u8State = 0;
        u32EnterPressStarted = 0xFFFFFFFF;
    }

    if (u8State == su8LastState) return 0;
    su8LastState = u8State;

    hid_enter_report_t *buf = (hid_enter_report_t *)HID_MISC_BUF;
    memset(buf, 0, sizeof *buf);
    buf->bReportId = HID_REPORT_ID_ENTER;
    if (u8State) buf->u8Keyboard[0] = KEY_ENTER;

    HID_MISC_SEND(buf);
    return 1;
}

#ifdef ENABLE_TOUCH_INPUT
enum {
    u8MaxFingers = (sizeof((hid_touch_report_t *)(0))->sFinger) /
                   (sizeof((hid_touch_report_t *)(0))->sFinger[0])
};
#define OVERLAP(start1, end1, start2, end2) (Minimum((end2), (end1)) >= Maximum((start1), (start2)))
typedef struct {
    uint8_t bActive;
    uint8_t u8Start;
    uint8_t u8End;

    uint8_t u8FakeCentreX;
    uint8_t u8FakeCentreY;
} tracked_finger_t;
tracked_finger_t trackedFingers[u8MaxFingers] = { 0 };

#define SCREEN_HEIGHT 127
#define SCREEN_WIDTH 128
// Definition for Sonolus on my Pixel 6
// #define LEFT_EDGE 28
// #define RIGHT_EDGE 20
// Definition for PJSK on my Pixel 6
// #define LEFT_EDGE 22
// #define RIGHT_EDGE 20
#define LEFT_EDGE 0
#define RIGHT_EDGE 0

#define VERTICAL_MOVEMENT 10

#define SCREEN_Y (SCREEN_HEIGHT / 5)
#define TOUCH_WIDTH ((SCREEN_WIDTH - LEFT_EDGE - RIGHT_EDGE) / 16)

static void _TrackTouches(void) {
    static tracked_finger_t newFingers[u8MaxFingers];
    memset(newFingers, 0, sizeof newFingers);

    uint8_t nFingers = 0;
    int8_t iStart = -1;

    static struct {
        uint16_t u16Bias;
        uint8_t u8Pos;
    } sBiasCalc[16] = { 0 };
    uint16_t u16BiasTop = 0;
    uint16_t u16BiasBottom = 0;

    // Identify our currently active fingers
    for (uint8_t x = 0; x <= 16; x++) {
        if ((x < 16) && gu16PSoCDigital & (1 << x)) {
            if (iStart == -1) {
                u16BiasTop = 0;
                u16BiasBottom = 0;
                iStart = x;
            }

            // Position = [LEFT_EDGE, {scale based on X} , RIGHT_EDGE] = SCREEN_WIDTH
            // Virtual width = SCREEN_WIDTH - LEFT_EDGE - RIGHT_EDGE

            // L + ((W-L-R)  * (16 - x)) / 16)

            // TODO: Might need -1 here not +1
            sBiasCalc[x - iStart].u8Pos = LEFT_EDGE + ((SCREEN_WIDTH - LEFT_EDGE - RIGHT_EDGE) * (32 - (x * 2 + 1))) / 32;
            // sBiasCalc[x - iStart].u8Pos -=
            //     (LEFT_EDGE + ((16 - x) * TOUCH_WIDTH) - (TOUCH_WIDTH / 2));
            sBiasCalc[x - iStart].u16Bias = gu8GroundData[x * 2] + gu8GroundData[x * 2 + 1];

            u16BiasTop += gu8GroundData[x * 2];
            u16BiasBottom += gu8GroundData[x * 2 + 1];
        } else {
            if (iStart != -1) {
                newFingers[nFingers].bActive = 1;
                newFingers[nFingers].u8Start = iStart;
                newFingers[nFingers].u8End = x - 1;

                uint32_t u32TotalWeight = 0;
                uint64_t u64Biased = 0;
                for (uint8_t i = 0; i < x - iStart; i++) {
                    u32TotalWeight += (uint32_t)sBiasCalc[i].u16Bias;
                    u64Biased += (uint64_t)sBiasCalc[i].u8Pos * (uint64_t)sBiasCalc[i].u16Bias;
                }
                newFingers[nFingers].u8FakeCentreX = u64Biased / u32TotalWeight;
                newFingers[nFingers].u8FakeCentreY = (u16BiasTop * VERTICAL_MOVEMENT) / (u16BiasTop + u16BiasBottom);

                nFingers++;
                iStart = -1;
            }
        }
        if (nFingers >= u8MaxFingers) break;
    }

    memcpy(trackedFingers, newFingers, sizeof newFingers);
}
static uint8_t _HID_Touch_Tick(void) {
    hid_touch_report_t *buf = (hid_touch_report_t *)HID_MISC_BUF;

    memset(buf, 0, sizeof *buf);
    buf->bReportId = HID_REPORT_ID_TOUCH;

    _TrackTouches();

    // When in portrait, X and Y make sense
    // In landscape, X and Y are inverse (ie screen rotation is ignored)

    uint8_t nFingers = 0;
    for (uint8_t i = 0; i < u8MaxFingers; i++) {
        buf->sFinger[nFingers].bTipSwitch = trackedFingers[i].bActive;
        buf->sFinger[nFingers].bIdentifier = i;

        if (trackedFingers[i].bActive) {
            uint8_t width = (trackedFingers[i].u8End + 1) - trackedFingers[i].u8Start;
            buf->sFinger[nFingers].bX = SCREEN_Y + trackedFingers[i].u8FakeCentreY;
            buf->sFinger[nFingers].bY = trackedFingers[i].u8FakeCentreX;

            buf->sFinger[nFingers].bW = TOUCH_WIDTH / 2;
            buf->sFinger[nFingers].bH = width * TOUCH_WIDTH;
        }

        nFingers++;
    }

    buf->bContactCount = nFingers;

    HID_MISC_SEND(buf);
    return 0;
}
#endif

typedef enum {
    TIMESLOT_KEYBOARD = 0,
    TIMESLOT_CONSUMER,
    TIMESLOT_ENTER,
#ifdef ENABLE_TOUCH_INPUT
    TIMESLOT_TOUCH,
#endif
    _TIMESLOT_COUNT,
} eTimeslot_t;
static void _HID_Misc_Tick(void) {
    static uint8_t sbLastEnableKeyboard = 0;

    // We have multiple things we're going to be sending over this HID endpoint, so we timeshare
    // which reports are sent. If a particular report has nothing to report in its slot, the next
    // report gets a chance instead.
    static eTimeslot_t eTimeslot = 0;

    uint8_t u8Tries = _TIMESLOT_COUNT;
    while (u8Tries--) {
        switch (eTimeslot++) {
            case TIMESLOT_KEYBOARD:
                if (gConfig.bEnableKeyboard) {
                    sbLastEnableKeyboard = 1;
                    if (_HID_Keyboard_Tick(0)) goto timeslot_used;
                } else if (sbLastEnableKeyboard) {
                    // If we've just disabled the keyboard, make sure to send a packet with all keys
                    // released!
                    sbLastEnableKeyboard = 0;
                    if (_HID_Keyboard_Tick(1)) goto timeslot_used;
                }
                break;

            case TIMESLOT_CONSUMER:
                if (_HID_Consumer_Tick()) goto timeslot_used;
                break;

            case TIMESLOT_ENTER:
                if (_HID_Enter_Tick()) goto timeslot_used;
                break;

#ifdef ENABLE_TOUCH_INPUT
            case TIMESLOT_TOUCH:
                if (_HID_Touch_Tick()) goto timeslot_used;
                break;
#endif

            default:
                break;
        }
        if (eTimeslot > _TIMESLOT_COUNT) eTimeslot = 0;
    }
timeslot_used:;
    ;
}

void USBD_HID_PrepareReport(void) {
    if (gu8HIDIO4Ready) IO4_HID_Tick();
    if (gu8HIDMiscReady) _HID_Misc_Tick();
}
static uint8_t sIO4InBuffer[sizeof(io4_hid_in_t)] = { 0 };
uint8_t *USBD_HID_GetReport(uint8_t u8ReportId, uint32_t *pu32Size) {
    switch (u8ReportId) {
        case HID_REPORT_ID_IO4:
            IO4_HID_Prepare(sIO4InBuffer);
            *pu32Size = sizeof sIO4InBuffer;
            return sIO4InBuffer;
        default:
            return NULL;
    }
}
void USBD_HID_SetReport(volatile uint8_t *pu8EpBuf, uint32_t u32Size) {
    // TODO: is pu8EpBuf[0] the report ID?
    // We need to switch on that report ID so we know what we're doing!

    if (u32Size < 2) return;
    IO4_Control(pu8EpBuf[1], u32Size - 2, &pu8EpBuf[2]);
}
