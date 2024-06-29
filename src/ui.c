#include "tasoller.h"

#define FN2_HOLD_TIME 250   // ms of FN2 to enter config
#define FN1_HOLD_TIME 2000  // ms of FN1 to enter calibration

// All three of these are scaled based on gConfig.u8Sens
#define CALIB_HANDS_MIN 100   // If we read less than this, ignore it
#define CALIB_HANDS_REQ 700   // All values are required to be greater than this
#define CALIB_HANDS_MAX 1300  // Scale from orange to green between [req] and [max]

#define RED 0
#define GREEN 120
#define BLUE 240

static void UI_TickSensitivity(void) {
    for (uint8_t i = 0; i < LED_NUM_GROUND; i++) {
        gaControlledIntLedData[i].u16H = 0;
        gaControlledIntLedData[i].u8S = (gConfig.u8Sens - 1) * 16;
        gaControlledIntLedData[i].u8V = 0;
    }

    for (uint8_t i = 0; i < gConfig.u8Sens; i++) gaControlledIntLedData[i * 2].u8V = 255;

    // Preferentially allow decreasing of sensitivity, in case it's been taken up too high
    if (gu32PSoCDigitalPos & PAD_26_Msk && gConfig.u8Sens > 1) {
        gConfig.u8Sens--;
    } else if (gu32PSoCDigitalPos & PAD_25_Msk && gConfig.u8Sens < 16) {
        gConfig.u8Sens++;
    }
}

static void UI_TickSettings(void) {
    // If either of the sensitivity settings are being changed, just render that
    static uint8_t su8SensTimeout = 0;
    if (gu16PSoCDigital & CELL_12_Msk) {
        su8SensTimeout = 150;
        UI_TickSensitivity();
        return;
    } else if (su8SensTimeout) {
        su8SensTimeout--;
        UI_TickSensitivity();

        // Save the sensitivity on exit
        if (su8SensTimeout == 0) PSoC_SetFingerCapacitanceFromConfig();
        return;
    }

    for (uint8_t i = 0; i < LED_NUM_GROUND; i++) {
        gaControlledIntLedData[i].u16H = 0;
        gaControlledIntLedData[i].u8S = 255;
        gaControlledIntLedData[i].u8V = 0;
    }

    {  // LED colour control
        gaControlledIntLedData[LED_CELL_0].u16H = gConfig.u16HueWingLeft;
        gaControlledIntLedData[LED_CELL_0].u8V = 255;
        gaControlledIntLedData[LED_CELL_1].u16H = gConfig.u16HueGround;
        gaControlledIntLedData[LED_CELL_1].u8V = 255;
        gaControlledIntLedData[LED_DIVIDER_1_2].u16H = gConfig.u16HueGroundActive;
        gaControlledIntLedData[LED_DIVIDER_1_2].u8V = 255;
        gaControlledIntLedData[LED_CELL_2].u16H = gConfig.u16HueGround;
        gaControlledIntLedData[LED_CELL_2].u8V = 255;
        gaControlledIntLedData[LED_CELL_3].u16H = gConfig.u16HueWingRight;
        gaControlledIntLedData[LED_CELL_3].u8V = 255;

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
            gaControlledIntLedData[LED_CELL_5].u16H = su16Hue / 5;
            gaControlledIntLedData[LED_CELL_5].u8V = 255;
        } else {
            gaControlledIntLedData[LED_CELL_5].u8S = 0;
            gaControlledIntLedData[LED_CELL_5].u8V = 255;
        }

        if (gu16PSoCDigitalPos & CELL_5_Msk) INV(gConfig.bEnableRainbow);
    }
    {  // Brightness
        if (gConfig.u8LedGroundBrightness) {
            gaControlledIntLedData[LED_CELL_6].u8S = 0;
            gaControlledIntLedData[LED_CELL_6].u8V = gConfig.u8LedGroundBrightness;
        } else {
            gaControlledIntLedData[LED_CELL_6].u8V = 255;
        }
        if (gConfig.u8LedWingBrightness) {
            gaControlledIntLedData[LED_CELL_7].u8S = 0;
            gaControlledIntLedData[LED_CELL_7].u8V = gConfig.u8LedWingBrightness;
        } else {
            gaControlledIntLedData[LED_CELL_7].u8V = 255;
        }

        if (gu32PSoCDigitalTrig & PAD_13_Msk) INCR(gConfig.u8LedGroundBrightness, 255);
        if (gu32PSoCDigitalTrig & PAD_14_Msk) DECR(gConfig.u8LedGroundBrightness, 0);
        if (gu32PSoCDigitalTrig & PAD_15_Msk) INCR(gConfig.u8LedWingBrightness, 255);
        if (gu32PSoCDigitalTrig & PAD_16_Msk) DECR(gConfig.u8LedWingBrightness, 0);
    }
    // [Cell 8,9,10,11 no function]
    {  // Sensitivity control (handled in dedicated function)
        gaControlledIntLedData[LED_CELL_12].u8S = (gConfig.u8Sens - 1) * 16;
        gaControlledIntLedData[LED_CELL_12].u8V = 255;
    }
    // [Cell 13 no function]
    {  // Mode switching
        gaControlledIntLedData[LED_CELL_14].u16H = gConfig.bEnableKeyboard ? GREEN : RED;
        gaControlledIntLedData[LED_CELL_14].u8V = 255;
        gaControlledIntLedData[LED_CELL_15].u16H = gConfig.bEnableIO4 ? GREEN : RED;
        gaControlledIntLedData[LED_CELL_15].u8V = 255;

        if (gu16PSoCDigitalPos & CELL_14_Msk) INV(gConfig.bEnableKeyboard);
        if (gu16PSoCDigitalPos & CELL_15_Msk) INV(gConfig.bEnableIO4);
    }
}

static inline void _FillControlled(uint16_t u16H, uint8_t u8Fill) {
    for (uint8_t i = 0; i < LED_NUM_GROUND; i++) {
        gaControlledIntLedData[i].u16H = u16H;
        gaControlledIntLedData[i].u8S = 255;
        gaControlledIntLedData[i].u8V = i < u8Fill ? 255 : 0;
    }
}

static uint8_t bCalibrationActive = 0;
static void UI_TickCalibration(void) {
    static uint16_t u16Timer = 0;
    static uint16_t su16MaxNoHands[32] = { 0 };
    static uint16_t su16MaxHands[32] = { 0 };

    if (!bCalibrationActive) {
        // Calibration start
        u16Timer = 0;
        bCalibrationActive = 1;
        memset(su16MaxNoHands, 0, sizeof su16MaxNoHands);
        memset(su16MaxHands, 0, sizeof su16MaxHands);
    }
    // Only tick the timer when we're using it, to avoid overflow
    if (u16Timer < 5000) u16Timer++;

    if (u16Timer < 3000) {
        // Flash red for the first 3 seconds
        _FillControlled(RED, ((u16Timer / 250) & 1) ? 0 : LED_NUM_GROUND);
    } else if (u16Timer < 4000) {
        // Fill up the cells with blue
        _FillControlled(BLUE, (u16Timer - 3000) / (1000 / LED_NUM_GROUND));
        for (uint8_t i; i < 32; i++) {
            su16MaxNoHands[i] = Maximum(su16MaxNoHands[i], gu16PSoCDiff[i]);
        }
    } else if (u16Timer < 5000) {
        // Flash green for the next second
        _FillControlled(GREEN, ((u16Timer / 250) & 1) ? 0 : LED_NUM_GROUND);
    } else {
        for (uint8_t i = 0; i < 32; i++) {
            // As well as the raw minimum, force a small constant threshold minimum too
            su16MaxHands[i] = Maximum(su16MaxHands[i], gu16PSoCDiff[i] + (gConfig.u8Sens * 5));
        }

        for (uint8_t i = 0; i < LED_NUM_GROUND; i++) {
            gaControlledIntLedData[i].u8S = 255;
            gaControlledIntLedData[i].u8V = 255;
        }

        uint16_t u16CalibMin = CALIB_HANDS_MIN + (gConfig.u8Sens * 50);
        uint16_t u16CalibReq = CALIB_HANDS_REQ + (gConfig.u8Sens * 50);
        uint16_t u16CalibMax = CALIB_HANDS_MAX + (gConfig.u8Sens * 50);

        uint8_t bOk = 1;
        // Iterate over the cells
        for (uint8_t i = 0; i < 16; i++) {
            if (su16MaxHands[i * 2] < u16CalibMin || su16MaxHands[i * 2 + 1] < u16CalibMin ||
                su16MaxHands[i * 2] < su16MaxNoHands[i * 2] ||
                su16MaxHands[i * 2 + 1] < su16MaxNoHands[i * 2 + 1]) {
                // Not enough data
                gaControlledIntLedData[i * 2].u8V = 0;
                bOk = 0;
            } else if (su16MaxHands[i * 2] < u16CalibReq || su16MaxHands[i * 2 + 1] < u16CalibReq) {
                // Data is too low
                gaControlledIntLedData[i * 2].u16H = RED;
                bOk = 0;
            } else if (su16MaxHands[i * 2] < u16CalibMax || su16MaxHands[i * 2 + 1] < u16CalibMax) {
                // We've got enough, but it could be better
                uint16_t u16Min = Minimum(su16MaxHands[i * 2], su16MaxHands[i * 2 + 1]);
                gaControlledIntLedData[i * 2].u16H =
                    ((u16Min - u16CalibReq) * GREEN) / (u16CalibMax - u16CalibReq);
            } else {
                // More than enough data
                gaControlledIntLedData[i * 2].u16H = GREEN;
            }
        }

        for (uint8_t i = 0; i < LED_NUM_GROUND; i++) {
            if (i % 2 == 1) {
                gaControlledIntLedData[i].u16H = bOk ? GREEN : RED;
            }
        }

        // Calibration complete
        if (gu8DigitalButtons & DIGITAL_FN1_Msk) {
            bCalibrationActive = 0;

            if (bOk) {
                memcpy(gConfig.u16PSoCScaleMin, su16MaxNoHands, sizeof su16MaxNoHands);
                memcpy(gConfig.u16PSoCScaleMax, su16MaxHands, sizeof su16MaxHands);
                bConfigDirty = 1;
            }
        }
    }
}

void UI_Tick(void) {
    static uint8_t u8Fn2Held = 0;
    if (gu8DigitalButtons & DIGITAL_FN2_Msk) {
        if (u8Fn2Held < FN2_HOLD_TIME) u8Fn2Held++;
    } else {
        // We released the button after holding it for long enough to be in the configuration UI, so
        // assume something changed
        if (u8Fn2Held >= FN2_HOLD_TIME) {
            bConfigDirty = 1;
        }

        u8Fn2Held = 0;
    }

    static uint16_t u16Fn1Held = 0;
    if (gu8DigitalButtons & DIGITAL_FN1_Msk) {
        if (u16Fn1Held < FN1_HOLD_TIME) u16Fn1Held++;
    } else {
        u16Fn1Held = 0;
    }

    if (bCalibrationActive || u16Fn1Held >= FN1_HOLD_TIME) {
        gbLedDataIsControlledInt = 1;
        UI_TickCalibration();
    } else if (u8Fn2Held >= FN2_HOLD_TIME) {
        gbLedDataIsControlledInt = 1;
        UI_TickSettings();
    } else {
        gbLedDataIsControlledInt = 0;
    }
}
