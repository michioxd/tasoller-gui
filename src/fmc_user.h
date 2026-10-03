#pragma once

#include <stdint.h>
#include "_compiler.h"

#define Config0 FMC_CONFIG_BASE
#define Config1 FMC_CONFIG_BASE + 4

#define ISPGO 0x01
#define _FMC_ENABLE_CFG_UPDATE() \
    (FMC->ISPCTL |= FMC_ISPCTL_CFGUEN_Msk)  // Enable CONFIG Update Function
#define _FMC_DISABLE_CFG_UPDATE() \
    (FMC->ISPCTL &= ~FMC_ISPCTL_CFGUEN_Msk)  // Disable CONFIG Update Function

int FMC_Proc(uint32_t u32Cmd, uint32_t addr_start, uint32_t addr_end, uint32_t *data);

#define FMC_EraseAP(u32Start, u32Size) \
    FMC_Proc(FMC_ISPCMD_PAGE_ERASE, (u32Start), (u32Start) + (u32Size), NULL)
#define FMC_ReadData(u32Start, u32End, pu32Data) \
    FMC_Proc(FMC_ISPCMD_READ, (u32Start), (u32End), (pu32Data))
#define FMC_WriteData(u32Start, u32End, pu32Data) \
    FMC_Proc(FMC_ISPCMD_PROGRAM, (u32Start), (u32End), (pu32Data))
#define FMC_ReadU32(u32Addr, pData) FMC_ReadData((u32Addr), (u32Addr) + 4, (pData))
#define FMC_Erase_User(u32Addr) FMC_Proc(FMC_ISPCMD_PAGE_ERASE, (u32Addr), (u32Addr) + 4, 0)

void FMC_Open(void);
void FMC_Close(void);
void FMC_EEPROM_Load(void);
void FMC_EEPROM_Store(void);
int FMC_EEPROM_Save(void);
void FMC_ConfigDefaults(void);

#define FMC_EEPROM_VERSION 0x01
typedef struct __attribute__((aligned(4), packed)) {
    uint32_t u32Magic;

    // Flags
    union {
        struct __packed {
            uint8_t bRsv00 : 1;
            uint8_t bEnableKeyboard : 1;
            uint8_t bEnableRainbow : 1;
        };
        uint8_t u8Flags;
    };

    // Only needs 4 bits but we aren't short atm
    uint8_t u8Sens;  // [1~16], Higher = more sensitive

    uint16_t u16HueTowerLeft;
    uint16_t u16HueTowerRight;
    uint16_t u16HueGround;
    uint16_t u16HueGroundActive;

    uint8_t u8LedGroundBrightness;
    uint8_t u8LedTowerBrightness;

    // Calibration data
    uint16_t u16PSoCScaleMin[32];
    uint16_t u16PSoCScaleMax[32];

    // State data
    uint8_t u8NextBootLEDBootloader;
    uint8_t u8Keymap[40];
    uint8_t u8KeyboardMode;
    uint8_t u8DividerMode;
    uint8_t u8KeymapProfiles[6][32];
} flash_t;
_Static_assert(sizeof(flash_t) == 380 && sizeof(flash_t) <= 512, "config must fit one data-flash page");
extern flash_t gConfig;
extern uint8_t bConfigDirty;
