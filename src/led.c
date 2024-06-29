#include "tasoller.h"

hsv_t gaControlledIntLedData[LED_NUM_GROUND] = { 0 };
uint8_t gbLedDataIsControlledInt = 0;
uint8_t gu8aControlledExtLedData[32 * 3];
uint8_t gbLedDataIsControlledExt = 0;

volatile uint8_t gu8LEDTx[LED_Tx_BUFFER];
static void (*s_I2C1HandlerFn)(uint32_t u32Status) = NULL;

volatile static uint8_t su8LedTxDataLock = 0;

// Helper definitions
#define SLAVE_RX_ADDR_ACK 0x60
#define SLAVE_RX_ACK 0x80
#define I2C_SLAVE_RX_NACK 0x88
#define I2C_SLAVE_TX_REPEAT_START_STOP 0xA0
#define SLAVE_TX_ACK 0xA8
#define I2C_SLAVE_TX_NACK 0xC0

void I2C1_IRQHandler(void) {
    if (I2C_GET_TIMEOUT_FLAG(I2C1)) {
        I2C_ClearTimeoutFlag(I2C1);
    } else {
        if (s_I2C1HandlerFn != NULL) (s_I2C1HandlerFn)(I2C1->I2CSTATUS);
    }
}

void I2C1_SlaveTx(uint32_t u32Status) {
    static uint8_t su8I2CReadAddr = 0;

    switch (u32Status) {
        case SLAVE_RX_ACK:
            su8I2CReadAddr = I2C1->I2CDAT;
            su8LedTxDataLock = 1;
            I2C_SET_CONTROL_REG(I2C1, I2C_I2CON_SI_AA);
            break;
        case SLAVE_TX_ACK:
            I2C_SET_DATA(I2C1, gu8LEDTx[su8I2CReadAddr]);
            I2C_SET_CONTROL_REG(I2C1, I2C_I2CON_SI_AA);
            break;

        case SLAVE_RX_ADDR_ACK:
        case I2C_SLAVE_TX_NACK:
        case I2C_SLAVE_RX_NACK:
        case I2C_SLAVE_TX_REPEAT_START_STOP:
            I2C_SET_CONTROL_REG(I2C1, I2C_I2CON_SI_AA);
            break;

        default:
            // Hmm?
            I2C_SET_CONTROL_REG(I2C1, I2C_I2CON_SI_AA);
            break;
    }
}

void LED_I2C1_Init(void) {
    I2C_Open(I2C1, 100000);
    I2C_SetSlaveAddr(I2C1, 0, 0x18, 0);
    I2C_SetSlaveAddr(I2C1, 1, 0x30, 0);
    I2C_SetSlaveAddr(I2C1, 2, 0x55, 0);
    I2C_SetSlaveAddr(I2C1, 3, 0x18, 0);
    I2C_SetSlaveAddrMask(I2C1, 0, 1);
    I2C_SetSlaveAddrMask(I2C1, 1, 4);
    I2C_SetSlaveAddrMask(I2C1, 2, 1);
    I2C_SetSlaveAddrMask(I2C1, 3, 4);
    I2C_EnableInt(I2C1);
    NVIC_EnableIRQ(I2C1_IRQn);

    // I2C1 enter no address SLV mode
    I2C_SET_CONTROL_REG(I2C1, I2C_I2CON_SI_AA);

    s_I2C1HandlerFn = I2C1_SlaveTx;
}

// static inline void LEDTxLock(void) {
//     gu8LEDTx[0] = 0;
// }
// static inline void LEDTxCommit(uint8_t u8Command, uint8_t u8NExpected) {
//     gu8LEDTx[0] = u8Command;
// }

/**
 * @brief Convert from RGB to HSV
 *
 * @param pu8aRGB Destination 3-tuple to receive RGB values
 * @param u16H Hue, ranging 0~LED_HUE_MAX
 * @param u8S Saturation, ranging 0~255
 * @param u8V Value, ranging 0~255
 */
void HsvToRgb(uint8_t* pu8aRGB, uint16_t u16H, uint8_t u8S, uint8_t u8V) {
    if (u8S == 0) {
        pu8aRGB[0] = u8V;
        pu8aRGB[1] = u8V;
        pu8aRGB[2] = u8V;
        return;
    }

    uint8_t region = u16H / (LED_HUE_MAX / 6);
    uint8_t remainder = (u16H - (region * (LED_HUE_MAX / 6))) * (255 / (LED_HUE_MAX / 6));

    uint8_t p = (u8V * (255 - u8S)) >> 8;
    uint8_t q = (u8V * (255 - ((u8S * remainder) >> 8))) >> 8;
    uint8_t t = (u8V * (255 - ((u8S * (255 - remainder)) >> 8))) >> 8;

    switch (region) {
        case 0:
            pu8aRGB[0] = u8V;
            pu8aRGB[1] = t;
            pu8aRGB[2] = p;
            break;
        case 1:
            pu8aRGB[0] = q;
            pu8aRGB[1] = u8V;
            pu8aRGB[2] = p;
            break;
        case 2:
            pu8aRGB[0] = p;
            pu8aRGB[1] = u8V;
            pu8aRGB[2] = t;
            break;
        case 3:
            pu8aRGB[0] = p;
            pu8aRGB[1] = q;
            pu8aRGB[2] = u8V;
            break;
        case 4:
            pu8aRGB[0] = t;
            pu8aRGB[1] = p;
            pu8aRGB[2] = u8V;
            break;
        default:
            pu8aRGB[0] = u8V;
            pu8aRGB[1] = p;
            pu8aRGB[2] = q;
            break;
    }

    return;
}

// 0x00: Normal operation (all other values ignore ground data)
// 0x01: [Stock] Uses colours. Bar 1 full (from right)
// 0x02: [Stock] Uses colours. Bar 2 full (from right)
// 0x03: [Stock] Uses colours. Bar 3 full (from right)
// 0x04: [Stock] Uses colours. Bar 4 full (from right)
// 0x1X: [Stock] All white with black gaps. X bars black (from right)
// 0x1X: [CFW] Rainbow with black gaps. X bars black (from right)
// 0x20: [CFW] Flashes three times
// 0x80: [Stock] Flashes three times
static uint8_t su8LedSpecial = 0x05;

// 0: Separator bar every 4 (4 sections)
// 1: Separator bar every 2 (8 sections)
// 2: Separator bar every 1 (16 sections)
// 3: No separator bars
// 4~7: LEDs off
// For some reason this value can be |8, even though firmware suggests otherwise
static uint8_t su8LedSeparators = 1;

// 0: Invalid, but functions as 1
// 1: 32-key mode
// 2: 16-key mode (same as 32key mode)
// 3: 8-key mode (bars light in pairs)
// 4: 4-key mode (bars light in quads)
static uint8_t su8LedNKey = 1;

// 0x40: Turns off ground LEDs
// 0x80: Turns off wing LEDs
static uint8_t su8LedOff = 0;

// 0x8X: Separator 1~(X+1) lit (ie X ranges from 0~E; F is the same as E)
static uint8_t su8LedCfwRainbow = 0x8F;

void LED_WriteBasicGrounds(void) {
    gu8LEDTx[0] = LED_CMD_BASIC;

    // 32 bits of grounds
    // (01,02)=key1, (04,08)=key2 (10,20)=key3, (40,80)=key4
    gu8LEDTx[1] = 0;
    gu8LEDTx[2] = 0;
    gu8LEDTx[3] = 0;
    gu8LEDTx[4] = 0;
    for (uint8_t i = 0; i < 4; i++) {
        for (uint8_t j = 0; j < 8; j++) {
            if (gu8GroundData[i * 8 + j] > PSoC_INTERNAL_DIGITAL_TH) gu8LEDTx[i + 1] |= (1 << j);
        }
    }

#ifdef LED_FIRMWARE_CFW
    gu8LEDTx[5] = 0;  // Wing fill
    for (uint8_t i = 0; i < 6; i++)
        if (gu8DigitalButtons & (1 << (i + 2))) gu8LEDTx[5] |= 1 << i;
    gu8LEDTx[6] = su8LedCfwRainbow;
    gu8LEDTx[7] = 0;  // Unused
#else
    // Wings
    gu8LEDTx[5] = 0;  // Wing fill
    for (uint8_t i = 0; i < 6; i++)
        if (gu8DigitalButtons & (1 << (i + 2))) gu8LEDTx[5] |= 1 << i;
    gu8LEDTx[6] = gConfig.u16HueWingLeft / LED_HUE_SCALE;   // Hue left (default 330)
    gu8LEDTx[7] = gConfig.u16HueWingRight / LED_HUE_SCALE;  // Hue right (default 180)
#endif

    // Unused in CFW
    gu8LEDTx[8] = gConfig.u16HueGround / LED_HUE_SCALE;        // Hue ground inactive (default 45)
    gu8LEDTx[9] = gConfig.u16HueGroundActive / LED_HUE_SCALE;  // Hue ground active (default 330)

    // In CFW only su8LedOff is respected
    gu8LEDTx[10] = (su8LedSeparators << 4) | su8LedOff | su8LedNKey;
    gu8LEDTx[11] = su8LedSpecial;
}

static const uint8_t su8aWingSensors[LED_NUM_LEFT] = {
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

static void LED_OffGround(void) {
    memset((uint8_t*)&gu8LEDTx[LED_DATA_OFFSET], 0, LED_NUM_GROUND * 3);
}
static void LED_OffWings(void) {
    memset((uint8_t*)&gu8LEDTx[LED_DATA_OFFSET + LED_NUM_GROUND * 3], 0,
           (LED_NUM_LEFT + LED_NUM_RIGHT) * 3);
}
static void LED_AirWings(void) {
    uint8_t u8aRgbActive[3];
    uint8_t u8aRgbInactive[3];

    uint8_t j, i = LED_NUM_GROUND;

    // Left wing
    HsvToRgb(u8aRgbActive, gConfig.u16HueWingLeft, SATURATION_ACTIVE, VALUE_ACTIVE);
    HsvToRgb(u8aRgbInactive, gConfig.u16HueWingLeft, SATURATION_INACTIVE, VALUE_INACTIVE);
    for (j = 0; i < LED_NUM_GROUND + LED_NUM_LEFT; i++, j++) {
        // GRB
        if (gu8DigitalButtons & su8aWingSensors[LED_NUM_LEFT - j - 1]) {
            gu8LEDTx[LED_DATA_OFFSET + i * 3 + 0] = u8aRgbActive[1];
            gu8LEDTx[LED_DATA_OFFSET + i * 3 + 1] = u8aRgbActive[0];
            gu8LEDTx[LED_DATA_OFFSET + i * 3 + 2] = u8aRgbActive[2];
        } else {
            gu8LEDTx[LED_DATA_OFFSET + i * 3 + 0] = u8aRgbInactive[1];
            gu8LEDTx[LED_DATA_OFFSET + i * 3 + 1] = u8aRgbInactive[0];
            gu8LEDTx[LED_DATA_OFFSET + i * 3 + 2] = u8aRgbInactive[2];
        }
    }

    // Right wing
    HsvToRgb(u8aRgbActive, gConfig.u16HueWingRight, SATURATION_ACTIVE, VALUE_ACTIVE);
    HsvToRgb(u8aRgbInactive, gConfig.u16HueWingRight, SATURATION_INACTIVE, VALUE_INACTIVE);
    for (j = 0; i < LED_NUM_GROUND + LED_NUM_LEFT + LED_NUM_RIGHT; i++, j++) {
        // GRB
        if (gu8DigitalButtons & su8aWingSensors[j]) {
            gu8LEDTx[LED_DATA_OFFSET + i * 3 + 0] = u8aRgbActive[1];
            gu8LEDTx[LED_DATA_OFFSET + i * 3 + 1] = u8aRgbActive[0];
            gu8LEDTx[LED_DATA_OFFSET + i * 3 + 2] = u8aRgbActive[2];
        } else {
            gu8LEDTx[LED_DATA_OFFSET + i * 3 + 0] = u8aRgbInactive[1];
            gu8LEDTx[LED_DATA_OFFSET + i * 3 + 1] = u8aRgbInactive[0];
            gu8LEDTx[LED_DATA_OFFSET + i * 3 + 2] = u8aRgbInactive[2];
        }
    }
}

static uint8_t LED_ScaleU8(uint8_t u8V, uint8_t u8Scale) {
    return ((uint16_t)u8V * (uint16_t)u8Scale) / 255;
}

static void LED_GroundRainbow(void) {
    uint8_t u8aRGB[3];

    // 5 ticks * 360 hue = 1800 calls for one cycle (1.8s)
    static uint16_t u16Hue = 0;
    static uint8_t u8Ticker = 0;
    if (++u8Ticker == 5) {
        u8Ticker = 0;
        u16Hue++;
        if (u16Hue == LED_HUE_MAX) u16Hue = 0;
    }

    /**
     * There are 48 LEDs for ground, but we only send 31 values
     * They're mapped to the LEDs as follows:
     *   00a11b22c33d44f55g66h77i...
     *
     * That is, we can't split-colour a key :P
     */
    for (uint8_t i = 0; i < LED_NUM_GROUND; i++) {
        uint8_t v = 190;
        uint8_t h = 0;
        uint8_t nCell = i >> 1;
        if (i % 2 == 0) {
            if (gu16PSoCDigital & (1 << nCell)) {
                v = 255;
                h = LED_HUE_MAX / 2;
            }
        } else if (nCell % 4 == 3) {
            h = LED_HUE_MAX / 2;
        }

        // GRB
        HsvToRgb(u8aRGB, (u16Hue + h + (i * (LED_HUE_MAX / LED_NUM_GROUND))) % LED_HUE_MAX, v,
                 LED_ScaleU8(v - 63, gConfig.u8LedGroundBrightness));

        gu8LEDTx[LED_DATA_OFFSET + i * 3 + 0] = u8aRGB[1];
        gu8LEDTx[LED_DATA_OFFSET + i * 3 + 1] = u8aRGB[0];
        gu8LEDTx[LED_DATA_OFFSET + i * 3 + 2] = u8aRGB[2];
    }
}
static void LED_GroundStatic(void) {
    uint8_t u8aGround[3];
    uint8_t u8aGroundActive[3];

    HsvToRgb(u8aGround, gConfig.u16HueGround, 255, gConfig.u8LedGroundBrightness);
    HsvToRgb(u8aGroundActive, gConfig.u16HueGroundActive, 255, gConfig.u8LedGroundBrightness);
    for (uint8_t i = 0; i < LED_NUM_GROUND; i++) {
        const uint8_t nCell = i >> 1;
        if (i % 2 == 0) {
            // This is a cell. Light it according to the touch input
            if (gu16PSoCDigital & (1 << nCell)) {
                gu8LEDTx[LED_DATA_OFFSET + i * 3 + 0] = u8aGroundActive[1];
                gu8LEDTx[LED_DATA_OFFSET + i * 3 + 1] = u8aGroundActive[0];
                gu8LEDTx[LED_DATA_OFFSET + i * 3 + 2] = u8aGroundActive[2];
            } else {
                gu8LEDTx[LED_DATA_OFFSET + i * 3 + 0] = u8aGround[1];
                gu8LEDTx[LED_DATA_OFFSET + i * 3 + 1] = u8aGround[0];
                gu8LEDTx[LED_DATA_OFFSET + i * 3 + 2] = u8aGround[2];
            }
        } else if (nCell % 4 == 3) {
            // This is a separating divider. Light it with the active colour
            gu8LEDTx[LED_DATA_OFFSET + i * 3 + 0] = u8aGroundActive[1];
            gu8LEDTx[LED_DATA_OFFSET + i * 3 + 1] = u8aGroundActive[0];
            gu8LEDTx[LED_DATA_OFFSET + i * 3 + 2] = u8aGroundActive[2];
        } else {
            // This is a non-separating divider. Light it based on the two cells either side
            if (gu16PSoCDigital & (1 << nCell) && gu16PSoCDigital & (1 << (nCell + 1))) {
                gu8LEDTx[LED_DATA_OFFSET + i * 3 + 0] = u8aGroundActive[1];
                gu8LEDTx[LED_DATA_OFFSET + i * 3 + 1] = u8aGroundActive[0];
                gu8LEDTx[LED_DATA_OFFSET + i * 3 + 2] = u8aGroundActive[2];
            } else {
                gu8LEDTx[LED_DATA_OFFSET + i * 3 + 0] = u8aGround[1];
                gu8LEDTx[LED_DATA_OFFSET + i * 3 + 1] = u8aGround[0];
                gu8LEDTx[LED_DATA_OFFSET + i * 3 + 2] = u8aGround[2];
            }
        }
    }
}

void LED_WriteRGB(void) {
    gu8LEDTx[0] = LED_CMD_RGB_FULL;

#ifdef LED_FIRMWARE_CFW
    // "CFW" added this byte
    gu8LEDTx[1] = su8LedSpecial | su8LedOff;
#endif

    // Even when grounds are disabled, internal control overrides that
    if (gbLedDataIsControlledInt) {
        PIN_LED_GROUND_PWR = 1;

        uint8_t u8aRGB[3];
        // Convert from HSV to GRB
        for (uint8_t i = 0; i < LED_NUM_GROUND; i++) {
            HsvToRgb(u8aRGB, gaControlledIntLedData[LED_NUM_GROUND - i - 1].u16H,
                     gaControlledIntLedData[LED_NUM_GROUND - i - 1].u8S,
                     gaControlledIntLedData[LED_NUM_GROUND - i - 1].u8V);

            gu8LEDTx[LED_DATA_OFFSET + i * 3 + 0] = u8aRGB[1];
            gu8LEDTx[LED_DATA_OFFSET + i * 3 + 1] = u8aRGB[0];
            gu8LEDTx[LED_DATA_OFFSET + i * 3 + 2] = u8aRGB[2];
        }
    } else if (gConfig.u8LedGroundBrightness) {
        PIN_LED_GROUND_PWR = 1;

        if (gbLedDataIsControlledExt) {
            // Swap from BRG to GRB
            for (uint8_t i = 0; i < LED_NUM_GROUND; i++) {
                gu8LEDTx[LED_DATA_OFFSET + i * 3 + 0] =
                    LED_ScaleU8(gu8aControlledExtLedData[i * 3 + 2], gConfig.u8LedGroundBrightness);
                gu8LEDTx[LED_DATA_OFFSET + i * 3 + 1] =
                    LED_ScaleU8(gu8aControlledExtLedData[i * 3 + 1], gConfig.u8LedGroundBrightness);
                gu8LEDTx[LED_DATA_OFFSET + i * 3 + 2] =
                    LED_ScaleU8(gu8aControlledExtLedData[i * 3 + 0], gConfig.u8LedGroundBrightness);
            }
        } else if (gConfig.bEnableRainbow) {
            LED_GroundRainbow();
        } else {
            LED_GroundStatic();
        }
    } else {
        PIN_LED_GROUND_PWR = 0;
        LED_OffGround();
    }

    if (gConfig.u8LedWingBrightness) {
        PIN_LED_WING_PWR = 1;
        // TODO: Get data from game when gbLedDataIsControlledExt (HID, probably)
        LED_AirWings();
    } else {
        PIN_LED_WING_PWR = 0;
        LED_OffWings();
    }
}
