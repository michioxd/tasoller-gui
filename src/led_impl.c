#include <stdlib.h>

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

#define INDEX_IS_CELL(x) ((x) % 2 == 0)
#define INDEX_IS_ACTIVE_CELL(x) (gu16PSoCDigital & (1 << ((x) >> 1)))
#define INDEX_IS_SEPARATING_DIVIDER(x) (((x) >> 1) % 4 == 3)
#define INDEX_IS_LIT_DIVIDER(x) \
    (gu16PSoCDigital & (1 << ((x) >> 1)) && gu16PSoCDigital & (1 << (((x) >> 1) + 1)))

typedef enum {
    // A cell that's currently active
    IndexType_CellActive,
    // A cell that isn't currently active
    IndexType_Cell,
    // A divider that's being lit up because it's part of the visual grouping
    IndexType_SeparatingDivider,
    // A divider that's being lit up because both cells either side are active
    IndexType_LitDivider,
    // A divider that's not being lit up
    IndexType_Divider,
} IndexType_t;

#define INDEX_TYPE(x)                                                                      \
    (IndexType_t)((INDEX_IS_CELL(x)                                                        \
                       ? (INDEX_IS_ACTIVE_CELL(x) ? IndexType_CellActive : IndexType_Cell) \
                   : INDEX_IS_SEPARATING_DIVIDER(x) ? IndexType_SeparatingDivider          \
                   : INDEX_IS_LIT_DIVIDER(x)        ? IndexType_LitDivider                 \
                                                    : IndexType_Divider))

static const uint8_t su8aTowerSensors[LED_NUM_TOWER] = {
    DIGITAL_AIR1_Msk, DIGITAL_AIR1_Msk, DIGITAL_AIR1_Msk, DIGITAL_AIR1_Msk,  //
    DIGITAL_AIR2_Msk, DIGITAL_AIR2_Msk, DIGITAL_AIR2_Msk, DIGITAL_AIR2_Msk,  //
    DIGITAL_AIR3_Msk, DIGITAL_AIR3_Msk, DIGITAL_AIR3_Msk, DIGITAL_AIR3_Msk,  //
    DIGITAL_AIR4_Msk, DIGITAL_AIR4_Msk, DIGITAL_AIR4_Msk, DIGITAL_AIR4_Msk,  //
    DIGITAL_AIR5_Msk, DIGITAL_AIR5_Msk, DIGITAL_AIR5_Msk, DIGITAL_AIR5_Msk,  //
    DIGITAL_AIR6_Msk, DIGITAL_AIR6_Msk, DIGITAL_AIR6_Msk, DIGITAL_AIR6_Msk,  //
};

#define SATURATION_ACTIVE 255
#define SATURATION_INACTIVE 240
#define VALUE_ACTIVE (gConfig.u8LedTowerBrightness)
#define VALUE_INACTIVE (gConfig.u8LedTowerBrightness / 2)

void LED_Towers_Reactive_HSV(_led_towers_hsv* pTowers) {
    for (uint8_t i = 0; i < LED_NUM_TOWER; i++) {
        // Left tower
        if (gu8DigitalButtons & su8aTowerSensors[LED_NUM_TOWER - i - 1]) {
            pTowers->aTowerL[i].h = gConfig.u16HueTowerLeft;
            pTowers->aTowerL[i].s = SATURATION_ACTIVE;
            pTowers->aTowerL[i].v = VALUE_ACTIVE;
        } else {
            pTowers->aTowerL[i].h = gConfig.u16HueTowerLeft;
            pTowers->aTowerL[i].s = SATURATION_INACTIVE;
            pTowers->aTowerL[i].v = VALUE_INACTIVE;
        }

        // Right tower
        if (gu8DigitalButtons & su8aTowerSensors[i]) {
            pTowers->aTowerR[i].h = gConfig.u16HueTowerRight;
            pTowers->aTowerR[i].s = SATURATION_ACTIVE;
            pTowers->aTowerR[i].v = VALUE_ACTIVE;
        } else {
            pTowers->aTowerR[i].h = gConfig.u16HueTowerRight;
            pTowers->aTowerR[i].s = SATURATION_INACTIVE;
            pTowers->aTowerR[i].v = VALUE_INACTIVE;
        }
    }
}

void LED_Ground_Rainbow_HSV(hsv_t* aGround) {
    static struct {
        uint8_t value;
        uint8_t direction;
    } twinkle[LED_NUM_GROUND_LOGICAL] = { 0 };

    // A true blue or true green will have a different brightness to other colours (currently) so
    // avoid that awkward discoloration.
    static const uint16_t u16HueOffset = 2;

    static uint8_t u8Ticker = 0;
    if (++u8Ticker == 5) {
        u8Ticker = 0;
        // 5 ticks * 360 hue = 1800 calls for one cycle (1.8s)
        // u16HueOffset++ or smth, I forgot

        // Twinkle animation
        // for (uint8_t i = 0; i < LED_NUM_GROUND_LOGICAL; i++) {
        //     if (twinkle[i].direction) {
        //         if (rand() < RAND_MAX / 256) {
        //             twinkle[i].direction = 0;
        //         }
        //     } else {
        //         if (rand() < RAND_MAX / 512) {
        //             twinkle[i].direction = 1;
        //         }
        //     }
        //
        //     if (twinkle[i].direction) {
        //         if (twinkle[i].value < 255) twinkle[i].value++;
        //     } else {
        //         if (twinkle[i].value > 0) twinkle[i].value--;
        //     }
        // }

        // Fade-out animation
        for (uint8_t i = 0; i < LED_NUM_GROUND_LOGICAL; i++) {
            switch (INDEX_TYPE(i)) {
                case IndexType_CellActive:
                    // u8New = (gu8GroundData[(i >> 1) * 2] / 2 + gu8GroundData[(i >> 1) * 2 + 1] /
                    // 2); if (u8New > twinkle[i].value) twinkle[i].value = u8New; break;
                case IndexType_LitDivider:
                    // u8New =
                    //     (gu8GroundData[(i >> 1) * 2] / 4 + gu8GroundData[(i >> 1) * 2 + 1] / 4 +
                    //      gu8GroundData[(i >> 1) * 2 + 2] / 4 + gu8GroundData[(i >> 1) * 2 + 3] /
                    //      4);
                    // u8New = 255;
                    // if (u8New > twinkle[i].value) twinkle[i].value = u8New;

                    twinkle[i].value = 255;
                    break;
                default:
                    if (twinkle[i].value > 200)
                        twinkle[i].value -= 8;
                    else if (twinkle[i].value > 150)
                        twinkle[i].value -= 6;
                    else if (twinkle[i].value > 100)
                        twinkle[i].value -= 4;
                    else if (twinkle[i].value > 50)
                        twinkle[i].value -= 1;
                    if (twinkle[i].value < 50) twinkle[i].value = 50;
                    break;
            }
        }
    }

    for (uint8_t i = 0; i < LED_NUM_GROUND_LOGICAL; i++) {
        const uint16_t u16H =
            (u16HueOffset + (i * (LED_HUE_MAX / (LED_NUM_GROUND_LOGICAL - 1)))) % LED_HUE_MAX;

        if (1) {
            switch (INDEX_TYPE(i)) {
                case IndexType_CellActive:
                case IndexType_LitDivider:
                case IndexType_SeparatingDivider:
                    aGround[i].h = u16H;
                    aGround[i].s = 255;
                    aGround[i].v = 255;
                    break;
                case IndexType_Cell:
                case IndexType_Divider:
                    aGround[i].h = u16H;
                    aGround[i].s = 255;

                    // aGround[i].v = 20 + (twinkle[i].value / 4);
                    aGround[i].v = twinkle[i].value;
                    break;
            }

            continue;
        }

        const uint16_t u16Value = 50;
        const uint16_t u16ValueActive = 255;

        uint8_t u8Value;
        switch (INDEX_TYPE(i)) {
            case IndexType_CellActive:
                // aGround[i].h = u16H;
                // aGround[i].s = 255;
                // aGround[i].v = u16ValueActive;
                // break;
            case IndexType_Cell:
                aGround[i].h = u16H;
                aGround[i].s = 255;

                u8Value = gu8GroundData[(i >> 1) * 2] / 2 + gu8GroundData[(i >> 1) * 2 + 1] / 2;
                aGround[i].v = u8Value;
                break;
            case IndexType_SeparatingDivider:
                aGround[i].h = u16H;
                aGround[i].s = 255;
                aGround[i].v = u16ValueActive;
                break;
            case IndexType_LitDivider:
                // aGround[i].h = u16H;
                // aGround[i].s = 255;
                // aGround[i].v = u16ValueActive;
                // break;
            case IndexType_Divider:
                aGround[i].h = u16H;
                aGround[i].s = 255;

                u8Value = gu8GroundData[(i >> 1) * 2] / 4 + gu8GroundData[(i >> 1) * 2 + 1] / 4 +
                          gu8GroundData[(i >> 1) * 2 + 2] / 4 + gu8GroundData[(i >> 1) * 2 + 3] / 4;
                aGround[i].v = u8Value;
                break;
        }
    }
}
void LED_Ground_Static_HSV(hsv_t* aGround) {
    for (uint8_t i = 0; i < LED_NUM_GROUND_LOGICAL; i++) {
        switch (INDEX_TYPE(i)) {
            case IndexType_CellActive:
                aGround[i].h = gConfig.u16HueGroundActive;
                aGround[i].s = 255;
                aGround[i].v = 255;
                break;
            case IndexType_Cell:
                aGround[i].h = gConfig.u16HueGround;
                aGround[i].s = 255;
                aGround[i].v = 255;

                break;
            case IndexType_SeparatingDivider:
                aGround[i].h = gConfig.u16HueGroundActive;
                aGround[i].s = 255;
                aGround[i].v = 255;
                break;
            case IndexType_LitDivider:
                aGround[i].h = gConfig.u16HueGroundActive;
                aGround[i].s = 255;
                aGround[i].v = 255;
                break;
            case IndexType_Divider:
                aGround[i].h = gConfig.u16HueGround;
                aGround[i].s = 255;
                aGround[i].v = 255;
                break;
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

void LED_Towers_Reactive_RGB(_led_towers_rgb* pTowers) {
    rgb_t u8aRgbActive;
    rgb_t u8aRgbInactive;

    uint8_t i;

    // Left tower
    HsvToHost(&u8aRgbActive, gConfig.u16HueTowerLeft, SATURATION_ACTIVE, VALUE_ACTIVE);
    HsvToHost(&u8aRgbInactive, gConfig.u16HueTowerLeft, SATURATION_INACTIVE, VALUE_INACTIVE);
    for (i = 0; i < LED_NUM_TOWER; i++) {
        // GRB
        if (gu8DigitalButtons & su8aTowerSensors[LED_NUM_TOWER - i - 1]) {
            pTowers->aTowerL[i].host.r = u8aRgbActive.host.r;
            pTowers->aTowerL[i].host.g = u8aRgbActive.host.g;
            pTowers->aTowerL[i].host.b = u8aRgbActive.host.b;
        } else {
            pTowers->aTowerL[i].host.r = u8aRgbInactive.host.r;
            pTowers->aTowerL[i].host.g = u8aRgbInactive.host.g;
            pTowers->aTowerL[i].host.b = u8aRgbInactive.host.b;
        }
    }

    // Right tower
    HsvToHost(&u8aRgbActive, gConfig.u16HueTowerRight, SATURATION_ACTIVE, VALUE_ACTIVE);
    HsvToHost(&u8aRgbInactive, gConfig.u16HueTowerRight, SATURATION_INACTIVE, VALUE_INACTIVE);
    for (i = 0; i < LED_NUM_TOWER; i++) {
        // GRB
        if (gu8DigitalButtons & su8aTowerSensors[i]) {
            pTowers->aTowerR[i].host.r = u8aRgbActive.host.r;
            pTowers->aTowerR[i].host.g = u8aRgbActive.host.g;
            pTowers->aTowerR[i].host.b = u8aRgbActive.host.b;
        } else {
            pTowers->aTowerR[i].host.r = u8aRgbInactive.host.r;
            pTowers->aTowerR[i].host.g = u8aRgbInactive.host.g;
            pTowers->aTowerR[i].host.b = u8aRgbInactive.host.b;
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

void LED_Towers_Controlled_RGB(_led_towers_rgb* pTowers) {
    for (uint8_t i = 0; i < LED_NUM_TOWER; i++) {
        // The PWM output is wired as BGR on IO4 PWM3~5
        pTowers->aTowerL[i].host.b = gu8IO4PWMOutput[2];
        pTowers->aTowerR[i].host.b = gu8IO4PWMOutput[2];
        pTowers->aTowerL[i].host.r = gu8IO4PWMOutput[3];
        pTowers->aTowerR[i].host.r = gu8IO4PWMOutput[3];
        pTowers->aTowerL[i].host.g = gu8IO4PWMOutput[4];
        pTowers->aTowerR[i].host.g = gu8IO4PWMOutput[4];
    }
}
void LED_Ground_Controlled_RGB(rgb_t* aGround) {
    // Swap from BRG(game) to GRB(host)
    for (uint8_t i = 0; i < LED_NUM_GROUND_LOGICAL; i++) {
#ifdef LED_CORRECTION_ON_LED_MCU
        aGround[i].host.r = LED_ScaleU8(gaControlledExtLedData[i].game.r, 255);
        aGround[i].host.g = LED_ScaleU8(gaControlledExtLedData[i].game.g, 255);
        aGround[i].host.b = LED_ScaleU8(gaControlledExtLedData[i].game.b, 255);
#else
        aGround[i].host.r = LED_ScaleU8(gaControlledExtLedData[i].game.r, 255);
        aGround[i].host.g = LED_ScaleU8(gaControlledExtLedData[i].game.g / 2, 255);
        aGround[i].host.b = LED_ScaleU8(gaControlledExtLedData[i].game.b / 2, 255);
#endif
    }
}
