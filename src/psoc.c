#include "tasoller.h"

uint16_t gu16PSoCDiff[32] = { 0 };

uint32_t gu32PSoCDigital = 0;
uint32_t gu32PSoCDigitalPos = 0;
uint32_t gu32PSoCDigitalNeg = 0;
uint32_t gu32PSoCDigitalTrig = 0;
uint16_t gu16PSoCDigital = 0;
uint16_t gu16PSoCDigitalPos = 0;
uint16_t gu16PSoCDigitalNeg = 0;
uint16_t gu16PSoCDigitalTrig = 0;

uint8_t gu8PSoCSeenData = 0;

volatile uint8_t bPSoCDirty = 0;

static void* pu8PsocRxDestination = NULL;
static uint8_t pu8PsocRxDestinationLen = 0;
static volatile uint8_t* pu8PsocGotData = NULL;
static void PSoC_HandleRx(PSoC_CMD_RX eCmd, uint8_t u8Len, uint8_t* u8Data) {
    if (eCmd != PSoC_CMD_RX_SLAVE_DIFF && eCmd != PSoC_CMD_RX_MASTER_DIFF &&
        eCmd != PSoC_CMD_RX_CS_START) {
        static uint8_t debug = 0;
        debug++;
    }

    uint8_t u8PsocDataOffset = 0;
    switch (eCmd) {
        case PSoC_CMD_RX_REQUEST_FINGER_CAP:
            PSoC_SetFingerCapacitanceFromConfig();
            return;

        // We don't care about these
        case PSoC_CMD_RX_CS_START:
        case PSoC_CMD_RX_CS_END:
            return;

        case PSoC_CMD_RX_SLAVE_DIFF:
            u8PsocDataOffset = 16;
            gu8PSoCSeenData |= 1;
            // Falls through
        case PSoC_CMD_RX_MASTER_DIFF:
            if (eCmd == PSoC_CMD_RX_MASTER_DIFF) gu8PSoCSeenData |= 2;
            if (u8Len != 32) return;
            for (uint8_t i = 0; i < 16; i++) {
                gu16PSoCDiff[u8PsocDataOffset + i] = u8Data[(i * 2)] << 8;
                gu16PSoCDiff[u8PsocDataOffset + i] |= u8Data[(i * 2) + 1];
            }
            bPSoCDirty = 1;
            return;

        // Arbitrary data reception
        case PSoC_CMD_RX_MASTER_TOUCH_TH:
        case PSoC_CMD_RX_SLAVE_TOUCH_TH:
        case PSoC_CMD_RX_MASTER_FINGER_TH:
        case PSoC_CMD_RX_MASTER_HYSTERESIS:
        case PSoC_CMD_RX_SLAVE_FINGER_TH:
        case PSoC_CMD_RX_SLAVE_HYSTERESIS:
        case PSoC_CMD_RX_GET_FINGER_CAP:
        case PSoC_CMD_RX_SET_FINGER_CAP:
            if (pu8PsocRxDestination) {
                if (u8Len > pu8PsocRxDestinationLen) u8Len = pu8PsocRxDestinationLen;
                memcpy(pu8PsocRxDestination, u8Data, u8Len);
                // Make sure we don't go clobbering stuff later!
                pu8PsocRxDestination = NULL;
            }
            if (pu8PsocGotData) {
                *pu8PsocGotData = 1;
                // Make sure we don't go clobbering stuff later!
                pu8PsocGotData = NULL;
            }
            return;
    }
}
void UART1_IRQHandler(void) {
    static uint8_t su8RxData[32] = { 0 };
    static uint8_t su8State = 0;
    static PSoC_CMD_RX eCmd = 0;
    static uint8_t su8Len = 0;
    static uint8_t su8Index = 0;
    static uint8_t su8Sum = 0;

    // We only care about data interrupts
    if (!(UART1->ISR & (UART_ISR_RDA_INT_Msk | UART_ISR_TOUT_INT_Msk))) return;

    uint8_t u8Data = UART_READ(UART1);

    switch (su8State) {
        case 0:
            eCmd = u8Data;  // For debugging
            switch (u8Data) {
                // Just go away
                case PSoC_CMD_RX_CS_START:
                case PSoC_CMD_RX_CS_END:
                    return;
                // Commands with no payload
                case PSoC_CMD_RX_SET_FINGER_CAP:
                    PSoC_HandleRx(u8Data, 0, NULL);
                    return;

                // Commands that have a payload we need to receive
                case PSoC_CMD_RX_REQUEST_FINGER_CAP:
                case PSoC_CMD_RX_MASTER_DIFF:
                case PSoC_CMD_RX_SLAVE_DIFF:
                case PSoC_CMD_RX_MASTER_TOUCH_TH:
                case PSoC_CMD_RX_SLAVE_TOUCH_TH:
                case PSoC_CMD_RX_MASTER_FINGER_TH:
                case PSoC_CMD_RX_MASTER_HYSTERESIS:
                case PSoC_CMD_RX_SLAVE_FINGER_TH:
                case PSoC_CMD_RX_SLAVE_HYSTERESIS:
                case PSoC_CMD_RX_GET_FINGER_CAP:
                    eCmd = u8Data;
                    su8Sum = u8Data;
                    su8State++;
                    return;
            }
            return;
        case 1:
            su8Len = u8Data;
            su8Index = 0;
            su8Sum += u8Data;
            su8State++;
            // If there's no payload to receive, skip that step
            if (!su8Len) su8State++;
            // Packets too large for reception need dropped
            if (su8Len > sizeof su8RxData) su8State = 0;
            return;
        case 2:
            su8Sum += u8Data;
            su8RxData[su8Index++] = u8Data;
            if (su8Index == su8Len) su8State++;
            return;
        case 3:
            // Only process packets with a valid checksum
            if (su8Sum == u8Data) PSoC_HandleRx(eCmd, su8Len, su8RxData);
            su8State = 0;
            return;

        // If we somehow land in an invalid state (should be impossible), make sure we can recover
        default:
            su8State = 0;
            return;
    }
}

static inline void PSoC_Cmd(PSoC_CMD_TX eCmd, uint8_t u8D0, uint8_t u8D1, uint8_t u8Blocking) {
    static uint8_t su8Ready = 0;
    if (u8Blocking) {
        su8Ready = 0;
        pu8PsocGotData = &su8Ready;
    }

    static uint8_t u8Packet[5];
    u8Packet[0] = eCmd;
    u8Packet[1] = 2;
    u8Packet[2] = u8D0;
    u8Packet[3] = u8D1;
    u8Packet[4] = eCmd + 2 + u8D0 + u8D1;
    for (uint8_t i = 0; i < 5; i++) {
        while (UART_IS_TX_FULL(UART1))
            ;
        UART_WRITE(UART1, u8Packet[i]);
    }

    if (u8Blocking) {
        while (!su8Ready)
            ;
    }
}

uint16_t PSoC_GetFingerCapacitance(void) {
    uint16_t u16FingerCap;

    pu8PsocRxDestination = &u16FingerCap;
    pu8PsocRxDestinationLen = sizeof u16FingerCap;
    PSoC_Cmd(PSoC_CMD_TX_GET_FINGER_CAP, 0, 0, 1);

    return u16FingerCap;
    // Swap endian
    // return (u16FingerCap >> 8) | ((u16FingerCap & 0xff) << 8);
}
void PSoC_GetDebug(PSoC_CMD_DEBUG u8Cmd, uint8_t* pu8Data, volatile uint8_t* pu8Ready) {
    pu8PsocGotData = pu8Ready;
    pu8PsocRxDestination = pu8Data;
    pu8PsocRxDestinationLen = 32;
    PSoC_Cmd(PSoC_CMD_TX_GET_DEBUG, 0, u8Cmd, 0);
}
void PSoC_SetFingerCapacitance(uint16_t u16FingerCap) {
    if (u16FingerCap > 1023) u16FingerCap = 1023;
    // PSoC_Cmd(PSoC_CMD_TX_SET_FINGER_CAP, u16FingerCap >> 8, u16FingerCap & 0xff, 1);
    PSoC_Cmd(PSoC_CMD_TX_SET_FINGER_CAP, u16FingerCap >> 8, u16FingerCap & 0xff, 1);
}
void PSoC_SetFingerCapacitanceFromConfig(void) {
    PSoC_SetFingerCapacitance(PSoC_FINGER_CAP_MIN + (16 - gConfig.u8Sens) * PSoC_FINGER_CAP_STEP);
}
void PSoC_EnableDebug(uint8_t u8D1, uint8_t u8D2) {
    // TODO: Finish figuring out what values these two bytes need
    PSoC_Cmd(PSoC_CMD_TX_ENABLE_DEBUG, u8D1, u8D2, 1);
}

uint8_t gu8GroundData[32];

// These are used to uniformly narrow the min/max gap
// #define SCALE_OFFSET_MIN 150
// #define SCALE_OFFSET_MIN 100
// #define SCALE_OFFSET_MAX 450

void PSoC_PostProcessing(void) {
    // Process the raw PSoC data to compute our external 0-255 values
    for (uint8_t i = 0; i < 32; i++) {
        uint8_t j = (15 - (i / 2)) * 2 + (i & 1);

        const uint16_t u16Pad = gu16PSoCDiff[i];

        const uint16_t u16Min = gConfig.u16PSoCScaleMin[i];  // + SCALE_OFFSET_MIN;
        const uint16_t u16Max = gConfig.u16PSoCScaleMax[i];  // - SCALE_OFFSET_MAX;

        if (u16Pad < u16Min) {
            gu8GroundData[j] = 0;
        } else if (u16Pad > u16Max) {
            gu8GroundData[j] = 255;
        } else {
            // Multiply by 255 first to retain precision
            // This can overflow u16, hence the u32 cast pre-mult
            gu8GroundData[j] = ((uint32_t)(u16Pad - u16Min) * 255) / (u16Max - u16Min);
        }
    }
}
/**
 * @brief Calculate digital input data from PSoC values
 *
 * Must be called at exactly 1ms intervals
 *
 * Trigger will activate once, a 200ms delay, then again every 5ms.
 */
void PSoC_DigitalCalc(void) {
    // Calculate digital data for 32 pads
    uint32_t u32Previous = gu32PSoCDigital;
    gu32PSoCDigital = 0;
    for (uint8_t i = 0; i < 32; i++) {
        if (gu8GroundData[i] > PSoC_INTERNAL_DIGITAL_TH) {
            gu32PSoCDigital |= (1 << i);
        }
    }
    gu32PSoCDigitalPos = gu32PSoCDigital & ~u32Previous;
    gu32PSoCDigitalNeg = u32Previous & ~gu32PSoCDigital;
    gu32PSoCDigitalTrig = gu32PSoCDigitalPos;

    // Calculate digital data for 16 cells
    uint16_t u16Previous = gu16PSoCDigital;
    gu16PSoCDigital = 0;
    for (uint8_t i = 0; i < 16; i++) {
        if (gu8GroundData[i * 2] > PSoC_INTERNAL_DIGITAL_TH ||
            gu8GroundData[i * 2 + 1] > PSoC_INTERNAL_DIGITAL_TH) {
            gu16PSoCDigital |= (1 << i);
        }
    }
    gu16PSoCDigitalPos = gu16PSoCDigital & ~u16Previous;
    gu16PSoCDigitalNeg = u16Previous & ~gu16PSoCDigital;
    gu16PSoCDigitalTrig = gu16PSoCDigitalPos;

    static uint8_t u8TriggerCounter = 0;
    if (++u8TriggerCounter != 5) return;
    u8TriggerCounter = 0;

    static uint8_t u8a16TriggerCounter[16] = { 0 };
    for (uint8_t i = 0; i < 16; i++) {
        if (!(gu16PSoCDigital & (1 << i))) {
            u8a16TriggerCounter[i] = 0;
        } else if (u8a16TriggerCounter[i] == 200 / 5) {
            // High-speed repeat
            gu16PSoCDigitalTrig |= (1 << i);
        } else {
            // Counter up to high-speed
            u8a16TriggerCounter[i]++;
        }
    }

    static uint8_t u8a32TriggerCounter[32] = { 0 };
    for (uint8_t i = 0; i < 32; i++) {
        if (!(gu32PSoCDigital & (1 << i))) {
            u8a32TriggerCounter[i] = 0;
        } else if (u8a32TriggerCounter[i] == 200 / 5) {
            // High-speed repeat
            gu32PSoCDigitalTrig |= (1 << i);
        } else {
            // Counter up to high-speed
            u8a32TriggerCounter[i]++;
        }
    }
}
