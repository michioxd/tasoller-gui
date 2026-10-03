#pragma once
#include "fmc_user.h"
#include "keymap.h"

#define CONFIG_API_VERSION 1
#define CONFIG_API_SIZE 14
#define CONFIG_GET_INFO 0xF2
#define CONFIG_GET 0xF3
#define CONFIG_SET 0xF4
#define CONFIG_SAVE 0xF5
#define CONFIG_RESET 0xF6
#define CONFIG_GET_KEYMAP 0xF7
#define CONFIG_SET_KEYMAP 0xF8
#define CONFIG_GET_INPUT 0xF9
#define CONFIG_GET_MODES 0xFA
#define CONFIG_GET_PROFILE 0xFB
#define CONFIG_SET_PROFILE 0xFC

enum { CONFIG_OK, CONFIG_LENGTH, CONFIG_VERSION, CONFIG_VALUE, CONFIG_BUSY, CONFIG_STORAGE };

static inline uint8_t Config_Validate(const uint8_t *data, uint8_t length) {
    if (length != CONFIG_API_SIZE) return CONFIG_LENGTH;
    if (data[0] != 1 || (data[1] & ~3) || data[3] || data[2] < 1 || data[2] > 16)
        return CONFIG_VALUE;
    for (uint8_t i = 4; i < 12; i += 2)
        if (((uint16_t)data[i] | ((uint16_t)data[i + 1] << 8)) > 359) return CONFIG_VALUE;
    return CONFIG_OK;
}

static inline void Config_Encode(const flash_t *config, uint8_t *data) {
    const uint16_t hues[] = { config->u16HueTowerLeft, config->u16HueTowerRight,
                             config->u16HueGround, config->u16HueGroundActive };
    data[0] = 1;
    data[1] = config->bEnableKeyboard | (config->bEnableRainbow << 1);
    data[2] = config->u8Sens;
    data[3] = 0;
    for (uint8_t i = 0; i < 4; i++) {
        data[4 + 2 * i] = hues[i];
        data[5 + 2 * i] = hues[i] >> 8;
    }
    data[12] = config->u8LedGroundBrightness;
    data[13] = config->u8LedTowerBrightness;
}

// Caller validates the complete payload first; no calibration/boot state is exposed.
static inline void Config_Apply(flash_t *config, const uint8_t *data) {
    config->bEnableKeyboard = data[1] & 1;
    config->bEnableRainbow = (data[1] >> 1) & 1;
    config->u8Sens = data[2];
    config->u16HueTowerLeft = data[4] | ((uint16_t)data[5] << 8);
    config->u16HueTowerRight = data[6] | ((uint16_t)data[7] << 8);
    config->u16HueGround = data[8] | ((uint16_t)data[9] << 8);
    config->u16HueGroundActive = data[10] | ((uint16_t)data[11] << 8);
    config->u8LedGroundBrightness = data[12];
    config->u8LedTowerBrightness = data[13];
}