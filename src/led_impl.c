#include "tasoller.h"

static inline uint8_t LED_ScaleU8(uint8_t u8V, uint8_t u8Scale) {
    // When using our custom firmware, don't perform scaling here because it'll be performed on the
    // LED processor for us instead
#if LED_FIRMWARE_TYPE == LED_FW_OURS
    return u8V;
#else
    return ((uint16_t)u8V * (uint16_t)u8Scale) / 255;
#endif
}

static const uint8_t su8aWingSensors[LED_NUM_WING] = {
    DIGITAL_AIR1_Msk, DIGITAL_AIR1_Msk, DIGITAL_AIR1_Msk, DIGITAL_AIR1_Msk,  //
    DIGITAL_AIR2_Msk, DIGITAL_AIR2_Msk, DIGITAL_AIR2_Msk, DIGITAL_AIR2_Msk,  //
    DIGITAL_AIR3_Msk, DIGITAL_AIR3_Msk, DIGITAL_AIR3_Msk, DIGITAL_AIR3_Msk,  //
    DIGITAL_AIR4_Msk, DIGITAL_AIR4_Msk, DIGITAL_AIR4_Msk, DIGITAL_AIR4_Msk,  //
    DIGITAL_AIR5_Msk, DIGITAL_AIR5_Msk, DIGITAL_AIR5_Msk, DIGITAL_AIR5_Msk,  //
    DIGITAL_AIR6_Msk, DIGITAL_AIR6_Msk, DIGITAL_AIR6_Msk, DIGITAL_AIR6_Msk,  //
};

#define SATURATION_ACTIVE 255
#define SATURATION_INACTIVE 240
#define VALUE_ACTIVE (gConfig.u8LedWingBrightness)
#define VALUE_INACTIVE (gConfig.u8LedWingBrightness / 2)

void LED_Wings_Reactive_HSV(_led_wings_hsv* pWings) {
    for (uint8_t i = 0; i < LED_NUM_WING; i++) {
        // Left wing
        if (gu8DigitalButtons & su8aWingSensors[LED_NUM_WING - i - 1]) {
            pWings->aWingL[i].h = gConfig.u16HueWingLeft;
            pWings->aWingL[i].s = SATURATION_ACTIVE;
            pWings->aWingL[i].v = VALUE_ACTIVE;
        } else {
            pWings->aWingL[i].h = gConfig.u16HueWingLeft;
            pWings->aWingL[i].s = SATURATION_INACTIVE;
            pWings->aWingL[i].v = VALUE_INACTIVE;
        }

        // Right wing
        if (gu8DigitalButtons & su8aWingSensors[i]) {
            pWings->aWingR[i].h = gConfig.u16HueWingRight;
            pWings->aWingR[i].s = SATURATION_ACTIVE;
            pWings->aWingR[i].v = VALUE_ACTIVE;
        } else {
            pWings->aWingR[i].h = gConfig.u16HueWingRight;
            pWings->aWingR[i].s = SATURATION_INACTIVE;
            pWings->aWingR[i].v = VALUE_INACTIVE;
        }
    }
}
void LED_Ground_Rainbow_HSV(hsv_t* aGround) {
    // 5 ticks * 360 hue = 1800 calls for one cycle (1.8s)
    static uint16_t u16Hue = 0;
    static uint8_t u8Ticker = 0;
    if (++u8Ticker == 5) {
        u8Ticker = 0;
        u16Hue++;
        if (u16Hue == LED_HUE_MAX) u16Hue = 0;
    }

    for (uint8_t i = 0; i < LED_NUM_GROUND_LOGICAL; i++) {
        uint16_t h = 0;
        uint8_t v = 190;
        uint8_t nCell = i >> 1;
        if (i % 2 == 0) {
            if (gu16PSoCDigital & (1 << nCell)) {
                v = 255;
                h = LED_HUE_MAX / 2;
            }
        } else if (nCell % 4 == 3) {
            h = LED_HUE_MAX / 2;
        }

        aGround[i].h = (u16Hue + h + (i * (LED_HUE_MAX / LED_NUM_GROUND_LOGICAL))) % LED_HUE_MAX;
        aGround[i].s = v;
        aGround[i].v = v - 63;
    }
}
void LED_Ground_Static_HSV(hsv_t* aGround) {
    for (uint8_t i = 0; i < LED_NUM_GROUND_LOGICAL; i++) {
        const uint8_t nCell = i >> 1;
        if (i % 2 == 0) {
            // This is a cell. Light it according to the touch input
            if (gu16PSoCDigital & (1 << nCell)) {
                aGround[i].h = gConfig.u16HueGroundActive;
                aGround[i].s = 255;
                aGround[i].v = 255;
            } else {
                aGround[i].h = gConfig.u16HueGround;
                aGround[i].s = 255;
                aGround[i].v = 255;
            }
        } else if (nCell % 4 == 3) {
            // This is a separating divider. Light it with the active colour
            aGround[i].h = gConfig.u16HueGroundActive;
            aGround[i].s = 255;
            aGround[i].v = 255;
        } else {
            // This is a non-separating divider. Light it based on the two cells either side
            if (gu16PSoCDigital & (1 << nCell) && gu16PSoCDigital & (1 << (nCell + 1))) {
                aGround[i].h = gConfig.u16HueGroundActive;
                aGround[i].s = 255;
                aGround[i].v = 255;
            } else {
                aGround[i].h = gConfig.u16HueGround;
                aGround[i].s = 255;
                aGround[i].v = 255;
            }
        }
    }
}
void LED_Ground_Internal_HSV(hsv_t* aGround) {
    for (uint8_t i = 0; i < LED_NUM_GROUND_LOGICAL; i++) {
        aGround[i].h = gaControlledIntLedData[LED_NUM_GROUND_LOGICAL - i - 1].h;
        aGround[i].s = gaControlledIntLedData[LED_NUM_GROUND_LOGICAL - i - 1].s;
        aGround[i].v = gaControlledIntLedData[LED_NUM_GROUND_LOGICAL - i - 1].v;
    }
}

void HsvToHost(rgb_t* pRGB, uint16_t u16H, uint8_t u8S, uint8_t u8V) {
    if (u8S == 0) {
        pRGB->host.r = u8V;
        pRGB->host.g = u8V;
        pRGB->host.b = u8V;
        return;
    }

    uint8_t region = u16H / (LED_HUE_MAX / 6);
    uint8_t remainder = (u16H - (region * (LED_HUE_MAX / 6))) * (255 / (LED_HUE_MAX / 6));

    uint8_t p = (u8V * (255 - u8S)) >> 8;
    uint8_t q = (u8V * (255 - ((u8S * remainder) >> 8))) >> 8;
    uint8_t t = (u8V * (255 - ((u8S * (255 - remainder)) >> 8))) >> 8;

    switch (region) {
        case 0:
            pRGB->host.r = u8V;
            pRGB->host.g = t;
            pRGB->host.b = p;
            break;
        case 1:
            pRGB->host.r = q;
            pRGB->host.g = u8V;
            pRGB->host.b = p;
            break;
        case 2:
            pRGB->host.r = p;
            pRGB->host.g = u8V;
            pRGB->host.b = t;
            break;
        case 3:
            pRGB->host.r = p;
            pRGB->host.g = q;
            pRGB->host.b = u8V;
            break;
        case 4:
            pRGB->host.r = t;
            pRGB->host.g = p;
            pRGB->host.b = u8V;
            break;
        default:
            pRGB->host.r = u8V;
            pRGB->host.g = p;
            pRGB->host.b = q;
            break;
    }

    return;
}

void LED_Wings_Reactive_RGB(_led_wings_rgb* pWings) {
    rgb_t u8aRgbActive;
    rgb_t u8aRgbInactive;

    uint8_t i;

    // Left wing
    HsvToHost(&u8aRgbActive, gConfig.u16HueWingLeft, SATURATION_ACTIVE, VALUE_ACTIVE);
    HsvToHost(&u8aRgbInactive, gConfig.u16HueWingLeft, SATURATION_INACTIVE, VALUE_INACTIVE);
    for (i = 0; i < LED_NUM_WING; i++) {
        // GRB
        if (gu8DigitalButtons & su8aWingSensors[LED_NUM_WING - i - 1]) {
            pWings->aWingL[i].host.r = u8aRgbActive.host.r;
            pWings->aWingL[i].host.g = u8aRgbActive.host.g;
            pWings->aWingL[i].host.b = u8aRgbActive.host.b;
        } else {
            pWings->aWingL[i].host.r = u8aRgbInactive.host.r;
            pWings->aWingL[i].host.g = u8aRgbInactive.host.g;
            pWings->aWingL[i].host.b = u8aRgbInactive.host.b;
        }
    }

    // Right wing
    HsvToHost(&u8aRgbActive, gConfig.u16HueWingRight, SATURATION_ACTIVE, VALUE_ACTIVE);
    HsvToHost(&u8aRgbInactive, gConfig.u16HueWingRight, SATURATION_INACTIVE, VALUE_INACTIVE);
    for (i = 0; i < LED_NUM_WING; i++) {
        // GRB
        if (gu8DigitalButtons & su8aWingSensors[i]) {
            pWings->aWingR[i].host.r = u8aRgbActive.host.r;
            pWings->aWingR[i].host.g = u8aRgbActive.host.g;
            pWings->aWingR[i].host.b = u8aRgbActive.host.b;
        } else {
            pWings->aWingR[i].host.r = u8aRgbInactive.host.r;
            pWings->aWingR[i].host.g = u8aRgbInactive.host.g;
            pWings->aWingR[i].host.b = u8aRgbInactive.host.b;
        }
    }
}
void LED_Ground_Rainbow_RGB(rgb_t* aGround) {
    // TODO: Even bother?
    return;
}
void LED_Ground_Static_RGB(rgb_t* aGround) {
    // TODO: Even bother?
    rgb_t rgbGround;
    rgb_t rgbGroundActive;
    HsvToHost(&rgbGround, gConfig.u16HueGround, 255, gConfig.u8LedGroundBrightness);
    HsvToHost(&rgbGroundActive, gConfig.u16HueGroundActive, 255, gConfig.u8LedGroundBrightness);
    for (uint8_t i = 0; i < LED_NUM_GROUND_LOGICAL; i++) {
        const uint8_t nCell = i >> 1;
        if (i % 2 == 0) {
            // This is a cell. Light it according to the touch input
            if (gu16PSoCDigital & (1 << nCell)) {
                aGround[i].host.r = rgbGroundActive.host.r;
                aGround[i].host.g = rgbGroundActive.host.g;
                aGround[i].host.b = rgbGroundActive.host.b;
            } else {
                aGround[i].host.r = rgbGround.host.r;
                aGround[i].host.g = rgbGround.host.g;
                aGround[i].host.b = rgbGround.host.b;
            }
        } else if (nCell % 4 == 3) {
            // This is a separating divider. Light it with the active colour
            aGround[i].host.r = rgbGroundActive.host.r;
            aGround[i].host.g = rgbGroundActive.host.g;
            aGround[i].host.b = rgbGroundActive.host.b;
        } else {
            // This is a non-separating divider. Light it based on the two cells either side
            if (gu16PSoCDigital & (1 << nCell) && gu16PSoCDigital & (1 << (nCell + 1))) {
                aGround[i].host.r = rgbGroundActive.host.r;
                aGround[i].host.g = rgbGroundActive.host.g;
                aGround[i].host.b = rgbGroundActive.host.b;
            } else {
                aGround[i].host.r = rgbGround.host.r;
                aGround[i].host.g = rgbGround.host.g;
                aGround[i].host.b = rgbGround.host.b;
            }
        }
    }
}

void LED_Wings_Controlled_RGB(_led_wings_rgb* pWings) {
    // TODO: Get data from game when gbLedDataIsControlledExt (HID, probably)

    for (uint8_t i = 0; i < LED_NUM_WING; i++) {
        // The PWM output is wired as BGR on PWM3~5
        pWings->aWingL[i].host.b = gu8IO4PWMOutput[2];
        pWings->aWingR[i].host.b = gu8IO4PWMOutput[2];
        pWings->aWingL[i].host.r = gu8IO4PWMOutput[3];
        pWings->aWingR[i].host.r = gu8IO4PWMOutput[3];
        pWings->aWingL[i].host.g = gu8IO4PWMOutput[4];
        pWings->aWingR[i].host.g = gu8IO4PWMOutput[4];
    }
}
void LED_Ground_Controlled_RGB(rgb_t* aGround) {
    // Swap from BRG(game) to GRB(host)
    for (uint8_t i = 0; i < LED_NUM_GROUND_LOGICAL; i++) {
        aGround[i].host.r = LED_ScaleU8(gaControlledExtLedData[i].game.r, 255);
        aGround[i].host.g = LED_ScaleU8(gaControlledExtLedData[i].game.g, 255);
        aGround[i].host.b = LED_ScaleU8(gaControlledExtLedData[i].game.b, 255);
    }
}
