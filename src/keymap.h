#pragma once
#include <stdint.h>
#include <string.h>
#include "usb_inc/keymap.h"
#include "fmc_user.h"

#define KEYMAP_SIZE 40
enum { KEYMAP_32K, KEYMAP_16K, KEYMAP_9K, KEYMAP_8K, KEYMAP_4K, KEYMAP_2K };
enum { DIVIDER_NONE, DIVIDER_SINGLE, DIVIDER_ALL, DIVIDER_2K,
       DIVIDER_4K, DIVIDER_8K, DIVIDER_9K };

// Representative is the lower pad index in the visually leftmost cell.
static inline uint8_t Keymap_Representative(uint8_t mode, uint8_t pad) {
    if (mode == KEYMAP_32K) return pad;
    uint8_t cell = (31 - pad) / 2, start;
    if (mode == KEYMAP_9K) start = cell == 0 ? 0 : cell == 15 ? 15 : 1 + ((cell - 1) / 2) * 2;
    else {
        uint8_t width = mode == KEYMAP_16K ? 1 : mode == KEYMAP_8K ? 2 : mode == KEYMAP_4K ? 4 : 8;
        start = (cell / width) * width;
    }
    return 30 - 2 * start;
}

// Expand the current map's representatives, never invent new default usages.
static inline void Keymap_Normalize(uint8_t *map, uint8_t mode) {
    uint8_t previous[32];
    memcpy(previous, map, sizeof previous);
    for (uint8_t i = 0; i < 32; i++) map[i] = previous[Keymap_Representative(mode, i)];
}

static inline uint8_t Keymap_Divider(uint8_t divider, uint8_t index) {
    if (index >= 31 || !(index & 1)) return 0;
    switch (divider) {
        case DIVIDER_SINGLE:
        case DIVIDER_2K: return index == 15;
        case DIVIDER_ALL: return 1;
        case DIVIDER_4K: return index % 8 == 7;
        case DIVIDER_8K: return index % 4 == 3;
        case DIVIDER_9K: return index % 4 == 1;
        default: return 0;
    }
}

static inline uint16_t Keymap_VisualCellMask(uint8_t cell) {
    return (uint16_t)(1u << (15 - cell));
}

static inline void Keymap_Defaults(uint8_t *map) {
    static const uint8_t defaults[KEYMAP_SIZE] = {
        KEY_I, KEY_9, KEY_8, KEY_K, KEY_U, KEY_M, KEY_7, KEY_J,
        KEY_Y, KEY_N, KEY_6, KEY_H, KEY_T, KEY_B, KEY_5, KEY_G,
        KEY_R, KEY_V, KEY_4, KEY_F, KEY_E, KEY_C, KEY_3, KEY_D,
        KEY_W, KEY_X, KEY_2, KEY_S, KEY_Q, KEY_Z, KEY_1, KEY_A,
        KEY_F1, KEY_F2, KEY_0, KEY_O, KEY_L, KEY_P, KEY_COMMA, KEY_PERIOD,
    };
    memcpy(map, defaults, sizeof defaults);
}

static inline uint8_t Keymap_Valid(const uint8_t *map) {
    for (uint8_t i = 0; i < KEYMAP_SIZE; i++)
        if (map[i] && (map[i] < 0x04 || map[i] > 0xA4)) return 0;
    return 1;
}

static inline uint8_t Keymap_GroupValid(const uint8_t *map, uint8_t mode, uint8_t divider) {
    if (mode > KEYMAP_2K || divider > DIVIDER_9K || !Keymap_Valid(map)) return 0;
    for (uint8_t i = 0; i < 32; i++)
        if (map[i] != map[Keymap_Representative(mode, i)]) return 0;
    return 1;
}

static inline uint8_t Keymap_ProfileValid(const uint8_t *pads, uint8_t mode) {
    uint8_t map[KEYMAP_SIZE] = {0};
    memcpy(map, pads, 32);
    return Keymap_GroupValid(map, mode, DIVIDER_NONE);
}

static inline void Keymap_ProfilesInit(flash_t *config) {
    for (uint8_t mode = 0; mode < 6; mode++) {
        uint8_t map[KEYMAP_SIZE];
        Keymap_Defaults(map);
        Keymap_Normalize(map, mode);
        memcpy(config->u8KeymapProfiles[mode], map, 32);
    }
    memcpy(config->u8KeymapProfiles[config->u8KeyboardMode], config->u8Keymap, 32);
}

// Known legacy records retain calibration, flags and the shared AIR/FN usages.
static inline void Keymap_LoadProfiles(flash_t *config) {
    if (config->u32Magic == 0x54617302) Keymap_Defaults(config->u8Keymap);
    if (config->u32Magic == 0x54617302 || config->u32Magic == 0x54617303) {
        config->u8KeyboardMode = KEYMAP_32K;
        config->u8DividerMode = DIVIDER_4K;
    }
    if (!Keymap_GroupValid(config->u8Keymap, config->u8KeyboardMode, config->u8DividerMode)) {
        if (!Keymap_Valid(config->u8Keymap)) Keymap_Defaults(config->u8Keymap);
        config->u8KeyboardMode = KEYMAP_32K;
        config->u8DividerMode = DIVIDER_4K;
    }
    if (config->u32Magic != 0x54617305) Keymap_ProfilesInit(config);
    else {
        for (uint8_t mode = 0; mode < 6; mode++) {
            if (!Keymap_ProfileValid(config->u8KeymapProfiles[mode], mode)) {
                uint8_t map[KEYMAP_SIZE];
                Keymap_Defaults(map);
                Keymap_Normalize(map, mode);
                memcpy(config->u8KeymapProfiles[mode], map, 32);
            }
        }
        memcpy(config->u8KeymapProfiles[config->u8KeyboardMode], config->u8Keymap, 32);
    }
}

static inline void Keymap_Switch(flash_t *config, uint8_t mode) {
    memcpy(config->u8KeymapProfiles[config->u8KeyboardMode], config->u8Keymap, 32);
    memcpy(config->u8Keymap, config->u8KeymapProfiles[mode], 32);
    config->u8KeyboardMode = mode;
}

static inline void Keymap_SetProfile(flash_t *config, uint8_t mode, const uint8_t *pads) {
    memcpy(config->u8KeymapProfiles[mode], pads, 32);
    if (mode == config->u8KeyboardMode) memmove(config->u8Keymap, pads, 32);
}

// Output is a zero-padded array of at most 40 distinct ordinary usages.
static inline void Keymap_Report(const uint8_t *map, uint32_t pads, uint8_t buttons,
                                 uint8_t *keys) {
    uint8_t count = 0;
    memset(keys, 0, KEYMAP_SIZE);
    for (uint8_t i = 0; i < KEYMAP_SIZE; i++) {
        uint8_t active = i < 32 ? !!(pads & (UINT32_C(1) << i))
                                : !!(buttons & (1u << (i - 32)));
        if (!active || !map[i]) continue;
        uint8_t j = 0;
        while (j < count && keys[j] != map[i]) j++;
        if (j == count) keys[count++] = map[i];
    }
}

typedef struct {
    uint8_t map[KEYMAP_SIZE], enabled, buttons, refresh;
    uint32_t pads;
} keymap_cache_t;

// 0: unchanged/disabled; 1: release all; 2: refreshed held-input report.
static inline uint8_t Keymap_Update(keymap_cache_t *cache, const uint8_t *map,
                                    uint8_t enabled, uint32_t pads, uint8_t buttons) {
    if (enabled != cache->enabled || memcmp(cache->map, map, KEYMAP_SIZE)) {
        memcpy(cache->map, map, KEYMAP_SIZE);
        cache->enabled = enabled;
        cache->refresh = 1;
        return 1;
    }
    if (!enabled || (!cache->refresh && cache->pads == pads && cache->buttons == buttons))
        return 0;
    cache->pads = pads;
    cache->buttons = buttons;
    cache->refresh = 0;
    return 2;
}