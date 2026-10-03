#include "tasoller.h"
#include "keymap.h"

void FMC_Open(void) { FMC->ISPCON |= FMC_ISPCON_ISPEN_Msk; }
void FMC_Close(void) { FMC->ISPCON &= ~FMC_ISPCON_ISPEN_Msk; }

int FMC_Proc(uint32_t u32Cmd, uint32_t addr_start, uint32_t addr_end, uint32_t *pu32Data) {
    uint32_t u32Addr, Reg;

    for (u32Addr = addr_start; u32Addr < addr_end; pu32Data++) {
        FMC->ISPCMD = u32Cmd;
        FMC->ISPADR = u32Addr;

        if (u32Cmd == FMC_ISPCMD_PROGRAM) FMC->ISPDAT = *pu32Data;

        FMC->ISPTRG = 0x1;
        __ISB();
        uint32_t timeout = 1000000;
        while (FMC->ISPTRG & 0x1) {
            if (--timeout == 0) return -1;
        }

        Reg = FMC->ISPCON;
        if (Reg & FMC_ISPCON_ISPFF_Msk) {
            FMC->ISPCON = Reg;
            return -1;
        }

        if (u32Cmd == FMC_ISPCMD_READ) *pu32Data = FMC->ISPDAT;

        if (u32Cmd == FMC_ISPCMD_PAGE_ERASE) {
            u32Addr += FMC_FLASH_PAGE_SIZE;
        } else {
            u32Addr += 4;
        }
    }

    return 0;
}

#define DATAFLASH_BASE (FMC->DFBADR)  // 1F000
// #define DATAFLASH_BASE (0x1F000)
// Dao has his data based at FE0. We're going to base ourself at 000 instead
#define DATAFLASH_EEPROM_BASE (DATAFLASH_BASE + 0x000)
#define DATAFLASH_VERSION (0x05)  // Gets merged into the magic number
#define DATAFLASH_MAGIC (0x54617300 | DATAFLASH_VERSION)

flash_t gConfig;

void FMC_ConfigDefaults(void) {
        gConfig.bEnableKeyboard = 0;
        gConfig.bEnableRainbow = 1;

        gConfig.u8Sens = 8;

        gConfig.u16HueTowerLeft = 330;
        gConfig.u16HueTowerRight = 180;

#ifdef LED_CORRECTION_ON_LED_MCU
        gConfig.u16HueGround = 60;
        gConfig.u16HueGroundActive = 300;
#else
        // The game uses 60° and 300°. Our red channel is approximately half as strong as the game,
        // so by shifting 30° we get the appearance of correct colours.
        gConfig.u16HueGround = 30;
        gConfig.u16HueGroundActive = 330;
#endif

        gConfig.u8LedGroundBrightness = 255;
        gConfig.u8LedTowerBrightness = 255;
        Keymap_Defaults(gConfig.u8Keymap);
        gConfig.u8KeyboardMode = KEYMAP_32K;
        gConfig.u8DividerMode = DIVIDER_4K;
        Keymap_ProfilesInit(&gConfig);
}

void FMC_EEPROM_Load(void) {
    FMC_Open();

    int result = FMC_ReadData(DATAFLASH_EEPROM_BASE, DATAFLASH_EEPROM_BASE + sizeof gConfig, (void *)&gConfig);
    if (result || (gConfig.u32Magic != DATAFLASH_MAGIC && gConfig.u32Magic != 0x54617304 &&
                   gConfig.u32Magic != 0x54617303 && gConfig.u32Magic != 0x54617302)) {
        gConfig.u8Flags = 0;
        FMC_ConfigDefaults();

        for (uint8_t i = 0; i < 32; i++) {
            gConfig.u16PSoCScaleMin[i] = 0;
            gConfig.u16PSoCScaleMax[i] = 2000;
        }

        bConfigDirty = 1;
    }
    Keymap_LoadProfiles(&gConfig);
    // If it's an invalid value, it's probably from uninitialized flash; don't boot to the
    // bootloader!
    if (!(gConfig.u8NextBootLEDBootloader == 0 || gConfig.u8NextBootLEDBootloader == 1)) {
        gConfig.u8NextBootLEDBootloader = 0;
    }

    FMC_Close();
}
uint8_t bConfigDirty = 0;
int FMC_EEPROM_Save(void) {
    Keymap_Switch(&gConfig, gConfig.u8KeyboardMode);
    flash_t saved, desired = gConfig;
    desired.u32Magic = DATAFLASH_MAGIC;
    // Data flash must be the existing final page, never APROM code/LDROM/CONFIG.
    if (DATAFLASH_BASE != 0x1F000) return -1;
    FMC_Open();
    int result = FMC_ReadData(DATAFLASH_EEPROM_BASE, DATAFLASH_EEPROM_BASE + sizeof saved, (void *)&saved);
    if (result) goto done;
    if (memcmp(&saved, &desired, sizeof saved) == 0) goto done;
    FMC->ISPCON |= FMC_ISPCON_APUEN_Msk;
    result = FMC_Erase_User(DATAFLASH_EEPROM_BASE);
    if (!result) result = FMC_WriteData(DATAFLASH_EEPROM_BASE, DATAFLASH_EEPROM_BASE + sizeof desired, (void *)&desired);
    if (!result) result = FMC_ReadData(DATAFLASH_EEPROM_BASE, DATAFLASH_EEPROM_BASE + sizeof saved, (void *)&saved);
    if (!result && memcmp(&saved, &desired, sizeof saved)) result = -1;
    FMC->ISPCON &= ~FMC_ISPCON_APUEN_Msk;
done:
    FMC_Close();
    if (!result) {
        gConfig.u32Magic = DATAFLASH_MAGIC;
        bConfigDirty = 0;
    }
    return result;
}

void FMC_EEPROM_Store(void) {
    if (!bConfigDirty) return;
    // One attempt per physical change, not an erase loop after storage failure.
    FMC_EEPROM_Save();
    bConfigDirty = 0;
}
