#include "tasoller.h"

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
        while (FMC->ISPTRG & 0x1)
            ;  // Wait for ISP command done.

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
#define DATAFLASH_VERSION (0x02)  // Gets merged into the magic number
#define DATAFLASH_MAGIC (0x54617300 | DATAFLASH_VERSION)

flash_t gConfig;

void FMC_EEPROM_Load(void) {
    FMC_Open();

    FMC_ReadData(DATAFLASH_EEPROM_BASE, DATAFLASH_EEPROM_BASE + sizeof gConfig, (void *)&gConfig);
    if (gConfig.u32Magic != DATAFLASH_MAGIC) {
        // Zeroing flags first means GCC knows we don't care about the other bits
        gConfig.u8Flags = 0;
        gConfig.bEnableKeyboard = 0;
        gConfig.bEnableRainbow = 1;

        gConfig.u8Sens = 8;

        gConfig.u16HueWingLeft = 330;
        gConfig.u16HueWingRight = 180;
        // TODO: These are the DJ DAO defaults, but the game looks like the hue should be 60 and 300
        gConfig.u16HueGround = 45;
        gConfig.u16HueGroundActive = 330;

        gConfig.u8LedGroundBrightness = 255;
        gConfig.u8LedWingBrightness = 255;

        for (uint8_t i = 0; i < 32; i++) {
            gConfig.u16PSoCScaleMin[i] = 0;
            gConfig.u16PSoCScaleMax[i] = 2000;
        }

        bConfigDirty = 1;
    }

    FMC_Close();
}
uint8_t bConfigDirty = 0;
void FMC_EEPROM_Store(void) {
    if (!bConfigDirty) return;
    bConfigDirty = 0;

    FMC_Open();
    FMC->ISPCON |= FMC_ISPCON_LDUEN_Msk;
    FMC->ISPCON |= FMC_ISPCON_APUEN_Msk;
    FMC_Erase(DATAFLASH_EEPROM_BASE);

    gConfig.u32Magic = DATAFLASH_MAGIC;
    FMC_WriteData(DATAFLASH_EEPROM_BASE, DATAFLASH_EEPROM_BASE + sizeof gConfig, (void *)&gConfig);

    // We don't actually have anything to do if we fail, so there's no point checking for now ig
    // // Check if our write succeeded or not
    // if (FMC_Read(DATAFLASH_EEPROM_BASE) != DATAFLASH_MAGIC)
    //     ;

    FMC->ISPCON &= ~FMC_ISPCON_APUEN_Msk;
    FMC->ISPCON &= ~FMC_ISPCON_LDUEN_Msk;
    FMC_Close();
}
