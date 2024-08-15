#include "tasoller.h"

hsv_t gaControlledIntLedData[LED_NUM_GROUND_LOGICAL] = { 0 };
uint8_t gbLedDataIsControlledInt = 0;
rgb_t gaControlledExtLedData[32];
uint8_t gbLedDataIsControlledExt = 0;
uint8_t gbLedIsCustom = 0;

volatile uint8_t gu8LEDTx[LED_PACKET_MAX_SIZE];

typedef enum : uint8_t {
    I2C_SLAVE_TX_REPEAT_START_STOP = 0xA0,
    I2C_SLAVE_TX_ADDR_ACK = 0xA8,
    I2C_SLAVE_TX_ARBITRATION_LOST = 0xB0,
    I2C_SLAVE_TX_DATA_ACK = 0xB8,
    I2C_SLAVE_TX_DATA_NACK = 0xC0,
    I2C_SLAVE_TX_LAST_DATA_ACK = 0xC8,
    I2C_SLAVE_RX_ADDR_ACK = 0x60,
    I2C_SLAVE_RX_ARBITRATION_LOST = 0x68,
    I2C_SLAVE_RX_DATA_ACK = 0x80,
    I2C_SLAVE_RX_DATA_NACK = 0x88,
} I2C_Status_Slave;

volatile uint8_t* gpu8I2CRx = NULL;
volatile uint16_t u16I2CRxIndex = 0;
void I2C1_SlaveTx(I2C_Status_Slave eStatus) {
    if (!eStatus) {
        // Something went very wrong; restart the I2C controller
        I2C_Close(I2C1);
        LED_I2C1_Init();
        return;
    }

    static uint8_t u8Cmd = 0;
    static uint16_t su16I2CReadAddr = 0;

    /**
     * Bulk data receive request:
     * (B0)Rx: START+SLA+W
     * (B1)Tx: ACK
     * (B2)Rx: [u8Cmd]
     * (B3)Tx: ACK
     * (B4)Rx: (Repeat START)+SLA+R
     * (B5)Tx: ACK
     * (B6)Rx: ACK
     * (B7)Tx: [u8Data[i]]  |
     * (B8)Rx: ACK          | Looped until Rx:NACK
     *     Rx: STOP
     *
     * Single data receive request:
     * (S0)Rx: START+SLA+W
     * (S1)Tx: ACK
     * (S2)Rx: [u8Cmd]
     * (S3)Tx: ACK
     *     Rx: STOP
     * ---
     * (S4)Rx: START+SLA+R
     * (S5)Tx: ACK
     */
    switch (eStatus) {
        // === Receive address and command ===
        case I2C_SLAVE_RX_ADDR_ACK:                      // (B0,S0)
            I2C_SET_CONTROL_REG(I2C1, I2C_I2CON_SI_AA);  // (B1,S1)
            break;
        case I2C_SLAVE_RX_DATA_ACK:  // (B2,S2)
            uint8_t u8Data = I2C_GET_DATA(I2C1);
            if (u16I2CRxIndex == 0) {
                u8Cmd = u8Data;
                switch (u8Cmd) {
                    case LED_I2C_REG_PACKET:
                        // The master is requesting a read-out of all the data we have in our
                        // buffer.
                        su16I2CReadAddr = 0;
                        break;
                    default:
                        // If we don't recognise this command, treat it as a register read.
                        // (Back-compat with stock LED firmware)
                        su16I2CReadAddr = u8Cmd;
                        break;
                }
                u16I2CRxIndex++;
            } else {
                // TODO: Currently we don't expose u8Cmd anywhere
                if (gpu8I2CRx != NULL) {
                    gpu8I2CRx[u16I2CRxIndex - 1] = u8Data;
                    // TODO: Have some bounds checking, and NACK an out of bounds write
                }
                u16I2CRxIndex++;
            }

            I2C_SET_CONTROL_REG(I2C1, I2C_I2CON_SI_AA);  // (B3,S3)
            break;
        case I2C_SLAVE_RX_DATA_NACK:
            I2C_SET_CONTROL_REG(I2C1, I2C_I2CON_SI_AA);
            u16I2CRxIndex = 0;
            break;
        case I2C_SLAVE_TX_REPEAT_START_STOP:             // (B4)
            I2C_SET_CONTROL_REG(I2C1, I2C_I2CON_SI_AA);  // (B5)
            u16I2CRxIndex = 0;
            break;

        // === Transmit our data ===
        case I2C_SLAVE_TX_ADDR_ACK:                           // (B6)
        case I2C_SLAVE_TX_DATA_ACK:                           // We got an ACK, and need to continue
            I2C_SET_DATA(I2C1, gu8LEDTx[su16I2CReadAddr++]);  // (B7)
            I2C_SET_CONTROL_REG(I2C1, I2C_I2CON_SI_AA);
            break;
        case I2C_SLAVE_TX_LAST_DATA_ACK:  // We got an ACK, but it's time to stop
            I2C_SET_CONTROL_REG(I2C1, I2C_I2CON_SI);
            u16I2CRxIndex = 0;
            break;
        case I2C_SLAVE_TX_DATA_NACK:  // We got a NACK; master has read enough data
            I2C_SET_CONTROL_REG(I2C1, I2C_I2CON_SI_AA);
            u16I2CRxIndex = 0;
            break;

        // === Error cases ===
        case I2C_SLAVE_RX_ARBITRATION_LOST:  // SLA+W
        case I2C_SLAVE_TX_ARBITRATION_LOST:  // SLA+R
            I2C_SET_CONTROL_REG(I2C1, I2C_I2CON_SI_AA);
            u16I2CRxIndex = 0;
            break;
    }
}
void I2C1_IRQHandler(void) {
    if (I2C_GET_TIMEOUT_FLAG(I2C1)) {
        I2C_ClearTimeoutFlag(I2C1);
    } else {
        I2C1_SlaveTx(I2C1->I2CSTATUS);
    }
}

void LED_I2C1_Init(void) {
    I2C_Open(I2C1, 400 kHz);
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
}

static const uint8_t _LED_GroundBrightness(void) {
    if (gbLedDataIsControlledInt) return gConfig.u8LedGroundBrightness;
    if (g_u8UsbState == USB_STATE_SUSPEND && gu32NowMs > 5000) return 0;
    if (gbLedDataIsControlledExt) {
        // The game is going to tell us how bright it wants the LEDs
        // For chunithm, that's 40/63 = 63.5% brightness.
        // TODO: Do we actually want to do this? Chunithm has no way for operators to change it
        // TODO: This won't be reflected in gu8LEDTx[1] with our custom firmware

        // The real range for this value is 0~63
        // Chunithm will always be sending a constant value of 40 though, as far as I'm aware.
        // Because of that, if we performed the scaling we'd be getting 40/63 = 63.5% brightness.
        // Instead, we're only going to scale on the off-chance that the brightness is actually
        // changed and it goes below 40.
        // During startup the brightness is set to 0, but all LEDs are zeroed too so... :D
        if (gu8GameBrightness < 40)
            return ((uint16_t)gConfig.u8LedGroundBrightness * (uint16_t)gu8GameBrightness) / 63;
        return gConfig.u8LedGroundBrightness;
    }
    return gConfig.u8LedGroundBrightness;
}
static const uint8_t _LED_TowerBrightness(void) {
    if (gbLedDataIsControlledInt) return gConfig.u8LedTowerBrightness;
    if (g_u8UsbState == USB_STATE_SUSPEND && gu32NowMs > 5000) return 0;
    if (gbLedDataIsControlledExt)
        return ((uint16_t)gConfig.u8LedTowerBrightness * (uint16_t)gu8IO4PWMScale) / 255;
    return gConfig.u8LedTowerBrightness;
}

static inline void _LED_SetPower(void) {
    PIN_LED_GROUND_PWR = _LED_GroundBrightness() ? 1 : 0;
    PIN_LED_TOWER_PWR = _LED_TowerBrightness() ? 1 : 0;
}

void LED_Write(void) {
    Pled_rx_custom_rgb pTxRGB = (Pled_rx_custom_rgb)gu8LEDTx;
    Pled_rx_custom_hsv pTxHSV = (Pled_rx_custom_hsv)gu8LEDTx;
    Pled_rx_custom_mixed pTxMix = (Pled_rx_custom_mixed)gu8LEDTx;
    // We might not use all of these (we aren't using RGB at the moment!) but they're just
    // convenience aliases.
    (void)pTxRGB;
    (void)pTxHSV;
    (void)pTxMix;

    // If we're internally controlled, data will be HSV
    if (gbLedDataIsControlledInt) {
        pTxHSV->u8Cmd = LED_CMD_CUSTOM_HSV;
        pTxHSV->u8GroundBrightness = _LED_GroundBrightness();
        pTxHSV->u8TowerBrightness = _LED_TowerBrightness();
        _LED_SetPower();

        LED_Ground_Internal_HSV(pTxHSV->aGround);
        LED_Towers_Reactive_HSV(&pTxHSV->Towers);
    } else if (gbLedDataIsControlledExt) {
        // RGB control from the game
        pTxRGB->u8Cmd = LED_CMD_CUSTOM_RGB;
        pTxRGB->u8GroundBrightness = _LED_GroundBrightness();
        pTxRGB->u8TowerBrightness = _LED_TowerBrightness();
        _LED_SetPower();

        LED_Ground_Controlled_RGB(pTxRGB->aGround);
        LED_Towers_Controlled_RGB(&pTxRGB->Towers);
    } else {
        // User-set coloring scheme
        pTxHSV->u8Cmd = LED_CMD_CUSTOM_HSV;
        pTxHSV->u8GroundBrightness = _LED_GroundBrightness();
        pTxHSV->u8TowerBrightness = _LED_TowerBrightness();
        _LED_SetPower();

        if (gConfig.bEnableRainbow) {
            LED_Ground_Rainbow_HSV(pTxHSV->aGround);
        } else {
            LED_Ground_Static_HSV(pTxHSV->aGround);
        }
        LED_Towers_Reactive_HSV(&pTxHSV->Towers);
    }
}

#define I2C_WaitTimeout (SystemCoreClock / 5);
#define WAIT_INDEX(cond)                        \
    do {                                        \
        u32TimeOutCnt = I2C_WaitTimeout;        \
        while (u16I2CRxIndex cond) {            \
            if (--u32TimeOutCnt == 0) return 0; \
        }                                       \
    } while (0)

uint8_t LED_FMC_Read(uint32_t u32Offset, uint32_t* pu32Data) {
    static volatile uint32_t u32Data;
    u32Data = 0xFFFFFFFF;
    uint32_t u32TimeOutCnt;

    // Wait for anything in the buffer to be read
    WAIT_INDEX(!= 0);
    WAIT_INDEX(== 0);
    // Request a 4-byte read from the LED board
    gpu8I2CRx = (volatile uint8_t*)&u32Data;
    gu8LEDTx[0] = 0;
    gu8LEDTx[1] = u32Offset & 0xff;
    gu8LEDTx[2] = (u32Offset >> 8) & 0xff;
    gu8LEDTx[3] = (u32Offset >> 16) & 0xff;
    gu8LEDTx[4] = (u32Offset >> 24) & 0xff;
    gu8LEDTx[0] = LED_CMD_FMC_READ;

    // Wait for our packet to be sent
    WAIT_INDEX(!= 0);
    WAIT_INDEX(== 0);
    // Wait for the LED board to send its response
    WAIT_INDEX(!= 5);

    if (pu32Data) *pu32Data = u32Data;
    return 1;
}
