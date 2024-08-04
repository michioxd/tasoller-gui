#include "tasoller.h"

#define FN1_HOLD_TIME 250  // ms of FN1 to enter config
#define FN2_TAP_TIME 250   // ms of FN2 to enter service/test
#define FN2_HOLD_TIME 500  // ms of FN2 to enter service/test

// All three of these are scaled based on gConfig.u8Sens
#define CALIB_HANDS_MIN 100   // If we read less than this, ignore it
#define CALIB_HANDS_REQ 700   // All values are required to be greater than this
#define CALIB_HANDS_MAX 1300  // Scale from orange to green between [req] and [max]

#define RED 0
#define GREEN 120
#define BLUE 240

uint8_t gbUIOpen = 0;
static uint8_t u8TestIsActive = 0;

static void UI_TickSensitivity(void) {
    for (uint8_t i = 0; i < LED_NUM_GROUND_LOGICAL; i++) {
        gaControlledIntLedData[i].h = 0;
        gaControlledIntLedData[i].s = (gConfig.u8Sens - 1) * 16;
        gaControlledIntLedData[i].v = 0;
    }

    for (uint8_t i = 0; i < gConfig.u8Sens; i++) gaControlledIntLedData[i * 2].v = 255;

    // Preferentially allow decreasing of sensitivity, in case it's been taken up too high
    if (gu32PSoCDigitalPos & PAD_26_Msk && gConfig.u8Sens > 1) {
        gConfig.u8Sens--;
    } else if (gu32PSoCDigitalPos & PAD_25_Msk && gConfig.u8Sens < 16) {
        gConfig.u8Sens++;
    }
}

static uint8_t su8SensTimeout = 0;
static void UI_TickSettings(void) {
    // If either of the sensitivity settings are being changed, just render that
    if (gu16PSoCDigital & CELL_12_Msk) {
        su8SensTimeout = 150;
        UI_TickSensitivity();
        return;
    } else if (su8SensTimeout) {
        su8SensTimeout--;
        UI_TickSensitivity();

        // Save the sensitivity on exit
        if (su8SensTimeout == 0) PSoC_SetFingerCapacitanceFromConfig(1);
        return;
    }

    static uint8_t u8Pulser = 0;
    static uint8_t u8PulserDir = 0;
    if (u8PulserDir) {
        if (--u8Pulser == 0) u8PulserDir = 0;
    } else {
        if (++u8Pulser == 255) u8PulserDir = 1;
    }

    for (uint8_t i = 0; i < LED_NUM_GROUND_LOGICAL; i++) {
        gaControlledIntLedData[i].h = 0;
        gaControlledIntLedData[i].s = 255;
        gaControlledIntLedData[i].v = 0;
    }

    {  // LED colour control
        gaControlledIntLedData[LED_CELL_0].h = gConfig.u16HueWingLeft;
        gaControlledIntLedData[LED_CELL_0].v = 255;
        gaControlledIntLedData[LED_CELL_1].h = gConfig.u16HueGround;
        gaControlledIntLedData[LED_CELL_1].v = 255;
        gaControlledIntLedData[LED_DIVIDER_1_2].h = gConfig.u16HueGroundActive;
        gaControlledIntLedData[LED_DIVIDER_1_2].v = 255;
        gaControlledIntLedData[LED_CELL_2].h = gConfig.u16HueGround;
        gaControlledIntLedData[LED_CELL_2].v = 255;
        gaControlledIntLedData[LED_CELL_3].h = gConfig.u16HueWingRight;
        gaControlledIntLedData[LED_CELL_3].v = 255;

        if (gu32PSoCDigitalTrig & PAD_1_Msk) MOD_INCR(gConfig.u16HueWingLeft, LED_HUE_MAX);
        if (gu32PSoCDigitalTrig & PAD_2_Msk) MOD_DECR(gConfig.u16HueWingLeft, LED_HUE_MAX);
        if (gu32PSoCDigitalTrig & PAD_3_Msk) MOD_INCR(gConfig.u16HueGround, LED_HUE_MAX);
        if (gu32PSoCDigitalTrig & PAD_4_Msk) MOD_DECR(gConfig.u16HueGround, LED_HUE_MAX);
        if (gu32PSoCDigitalTrig & PAD_5_Msk) MOD_INCR(gConfig.u16HueGroundActive, LED_HUE_MAX);
        if (gu32PSoCDigitalTrig & PAD_6_Msk) MOD_DECR(gConfig.u16HueGroundActive, LED_HUE_MAX);
        if (gu32PSoCDigitalTrig & PAD_7_Msk) MOD_INCR(gConfig.u16HueWingRight, LED_HUE_MAX);
        if (gu32PSoCDigitalTrig & PAD_8_Msk) MOD_DECR(gConfig.u16HueWingRight, LED_HUE_MAX);
    }
    // [Cell 4 no function]
    {  // Lighting toggles
        static uint16_t su16Hue = 0;
        MOD_INCR(su16Hue, LED_HUE_MAX * 5);
        if (gConfig.bEnableRainbow) {
            gaControlledIntLedData[LED_CELL_5].h = su16Hue / 5;
            gaControlledIntLedData[LED_CELL_5].v = 255;
        } else {
            gaControlledIntLedData[LED_CELL_5].s = 0;
            gaControlledIntLedData[LED_CELL_5].v = 255;
        }

        if (gu16PSoCDigitalPos & CELL_5_Msk) INV(gConfig.bEnableRainbow);
    }
    {  // Brightness
        if (gConfig.u8LedGroundBrightness) {
            gaControlledIntLedData[LED_CELL_6].s = 0;
            gaControlledIntLedData[LED_CELL_6].v = gConfig.u8LedGroundBrightness;
        } else {
            gaControlledIntLedData[LED_CELL_6].v = 255;
        }
        if (gConfig.u8LedWingBrightness) {
            gaControlledIntLedData[LED_CELL_7].s = 0;
            gaControlledIntLedData[LED_CELL_7].v = gConfig.u8LedWingBrightness;
        } else {
            gaControlledIntLedData[LED_CELL_7].v = 255;
        }

        if (gu32PSoCDigitalTrig & PAD_13_Msk) INCR(gConfig.u8LedGroundBrightness, 255);
        if (gu32PSoCDigitalTrig & PAD_14_Msk) DECR(gConfig.u8LedGroundBrightness, 0);
        if (gu32PSoCDigitalTrig & PAD_15_Msk) INCR(gConfig.u8LedWingBrightness, 255);
        if (gu32PSoCDigitalTrig & PAD_16_Msk) DECR(gConfig.u8LedWingBrightness, 0);
    }
    // [Cell 8 no function]

    {  // Consumer control
        gaControlledIntLedData[LED_CELL_9].s = 0;
        gaControlledIntLedData[LED_CELL_9].v = 255;
        if (u32EnterPressStarted != 0xFFFFFFFF) {
            gaControlledIntLedData[LED_CELL_10].s = 0;
            gaControlledIntLedData[LED_CELL_10].v = u8Pulser;
        } else {
            gaControlledIntLedData[LED_CELL_10].s = 0;
            gaControlledIntLedData[LED_CELL_10].v = 255;
        }

        if (gu32PSoCDigital & PAD_20_Msk) {
            u16RequestedConsumerControl = MEDIA_VOLUME_DOWN;
        } else if (gu32PSoCDigital & PAD_19_Msk) {
            u16RequestedConsumerControl = MEDIA_VOLUME_UP;
        } else {
            u16RequestedConsumerControl = 0;
        }

        if (gu16PSoCDigitalPos & CELL_10_Msk) {
            if (u32EnterPressStarted == 0xFFFFFFFF) u32EnterPressStarted = gu32NowMs;
        }
    }

    // [Cell 11 no function]

    {  // Sensitivity control (handled in dedicated function)
        gaControlledIntLedData[LED_CELL_12].s = (gConfig.u8Sens - 1) * 16;
        gaControlledIntLedData[LED_CELL_12].v = 255;
    }
    // [Cell 13,14 no function]
    {  // Mode switching
        gaControlledIntLedData[LED_CELL_15].h = gConfig.bEnableKeyboard ? GREEN : RED;
        gaControlledIntLedData[LED_CELL_15].v = 255;

        if (gu16PSoCDigitalPos & CELL_15_Msk) INV(gConfig.bEnableKeyboard);
    }
}

static uint32_t su32EnteredTestMenuAt = 0;
static void UI_TickServiceTest(void) {
    uint8_t u8V = 0;
    // Zero out the LED data
    for (uint8_t i = 0; i < LED_NUM_GROUND_LOGICAL; i++) {
        gaControlledIntLedData[i].h = 0;
        gaControlledIntLedData[i].s = 0;
        gaControlledIntLedData[i].v = 0;
    }

    uint8_t u8ForceTest = 0;
    if (u8TestIsActive) {
        // Send a test button, to trigger entry to the test menu
        if (MS_SINCE(su32EnteredTestMenuAt) < 100) {
            gu16IO4ForceButtons |= IO4_BUTTON_TEST;
            u8ForceTest = 1;
        }

        // Implement the on-screen buttons
        {  // Down
            if (gu16PSoCDigital & (CELL_0_Msk | CELL_1_Msk | CELL_2_Msk)) {
                u8V = 200;
            } else {
                u8V = 50;
            }
            gaControlledIntLedData[LED_CELL_0].v = u8V;
            gaControlledIntLedData[LED_DIVIDER_0_1].v = u8V;
            gaControlledIntLedData[LED_CELL_1].v = u8V;
            gaControlledIntLedData[LED_DIVIDER_1_2].v = u8V;
            gaControlledIntLedData[LED_CELL_2].v = u8V;
        }
        gaControlledIntLedData[LED_DIVIDER_2_3].v = 255;
        {  // Up
            if (gu16PSoCDigital & (CELL_3_Msk | CELL_4_Msk | CELL_5_Msk)) {
                u8V = 200;
            } else {
                u8V = 50;
            }
            gaControlledIntLedData[LED_CELL_3].v = u8V;
            gaControlledIntLedData[LED_DIVIDER_3_4].v = u8V;
            gaControlledIntLedData[LED_CELL_4].v = u8V;
            gaControlledIntLedData[LED_DIVIDER_4_5].v = u8V;
            gaControlledIntLedData[LED_CELL_5].v = u8V;
        }
        gaControlledIntLedData[LED_DIVIDER_5_6].v = 255;

        gaControlledIntLedData[LED_DIVIDER_12_13].v = 255;
        {  // OK
            if (gu16PSoCDigital & (CELL_13_Msk | CELL_14_Msk | CELL_15_Msk)) {
                u8V = 200;
            } else {
                u8V = 50;
            }
            gaControlledIntLedData[LED_CELL_13].v = u8V;
            gaControlledIntLedData[LED_DIVIDER_13_14].v = u8V;
            gaControlledIntLedData[LED_CELL_14].v = u8V;
            gaControlledIntLedData[LED_DIVIDER_14_15].v = u8V;
            gaControlledIntLedData[LED_CELL_15].v = u8V;
        }
    }

    // Implement our custom buttons for test and service
    gaControlledIntLedData[LED_DIVIDER_5_6].v = 255;
    {  // Test
        if (gu16PSoCDigital & (CELL_6_Msk | CELL_7_Msk)) {
            u8V = 200;
            gu16IO4ForceButtons |= IO4_BUTTON_TEST;
        } else {
            u8V = 50;
            // Don't un-press the test button if we're forcing it on
            if (!u8ForceTest) gu16IO4ForceButtons &= ~IO4_BUTTON_TEST;
        }
        gaControlledIntLedData[LED_CELL_6].v = u8V;
        gaControlledIntLedData[LED_DIVIDER_6_7].v = u8V;
        gaControlledIntLedData[LED_CELL_7].v = u8V;
    }
    gaControlledIntLedData[LED_DIVIDER_7_8].v = 255;
    {  // Service
        if (gu16PSoCDigital & (CELL_8_Msk | CELL_9_Msk)) {
            u8V = 200;
            gu16IO4ForceButtons |= IO4_BUTTON_SERVICE;
        } else {
            u8V = 50;
            gu16IO4ForceButtons &= ~IO4_BUTTON_SERVICE;
        }
        gaControlledIntLedData[LED_CELL_8].v = u8V;
        gaControlledIntLedData[LED_DIVIDER_8_9].v = u8V;
        gaControlledIntLedData[LED_CELL_9].v = u8V;
    }
    gaControlledIntLedData[LED_DIVIDER_9_10].v = 255;
}

void UI_Tick(void) {
    // Handle hold trigger on FN1
    static uint8_t u8Fn1Held = 0;
    if (gu8DigitalButtons & DIGITAL_FN1_Msk) {
        if (u8Fn1Held < FN1_HOLD_TIME) u8Fn1Held++;
    } else {
        // We released the button after holding it for long enough to be in the configuration UI, so
        // assume something changed
        if (u8Fn1Held >= FN1_HOLD_TIME) {
            // If FN2 was released while still in sensitivity adjustment, make sure the changes save
            if (su8SensTimeout) {
                PSoC_SetFingerCapacitanceFromConfig(1);
                su8SensTimeout = 0;
            }

            bConfigDirty = 1;
        }

        u16RequestedConsumerControl = 0;
        u32EnterPressStarted = 0;
        u8Fn1Held = 0;
    }

    // Handle double tap trigger on FN2
    static uint32_t u32LastFn2 = 0;
    static uint8_t u8LastDB = 0;
    const uint8_t u8PostDb = gu8DigitalButtons & (~u8LastDB);
    if (u8PostDb & DIGITAL_FN2_Msk) {
        if (u32LastFn2 && MS_SINCE(u32LastFn2) < FN2_TAP_TIME) {
            u8TestIsActive = !u8TestIsActive;

            if (u8TestIsActive) {
                su32EnteredTestMenuAt = gu32NowMs;
                gu16IO4ForceButtons = IO4_BUTTON_TEST;
            } else {
                su32EnteredTestMenuAt = 0;
                gu16IO4ForceButtons = 0;
            }
        }
        u32LastFn2 = gu32NowMs;
    }
    u8LastDB = gu8DigitalButtons;

    // Handle hold trigger on FN2
    static uint16_t u16Fn2Held = 0;
    if (gu8DigitalButtons & DIGITAL_FN2_Msk) {
        if (u16Fn2Held < FN2_HOLD_TIME) u16Fn2Held++;
    } else {
        u16Fn2Held = 0;
    }

    // Render the appropriate UI based on what's being done
    if (u8Fn1Held >= FN1_HOLD_TIME) {
        gbLedDataIsControlledInt = 1;
        gbUIOpen = 1;
        UI_TickSettings();
    } else if (u16Fn2Held >= FN2_HOLD_TIME || u8TestIsActive) {
        gbLedDataIsControlledInt = 1;
        gbUIOpen = 0;
        UI_TickServiceTest();
    } else {
        gbLedDataIsControlledInt = 0;
        gbUIOpen = 0;
    }
}
