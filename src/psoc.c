#include "tasoller.h"

// For some reason, any level of optimisation
// #pragma GCC diagnostic push
// #pragma GCC diagnostic warning "-Wpedantic"
// #pragma GCC push_options
// #pragma GCC optimize ("O0")

// TODO: Anything higher than O1 is breaking this code. Probably something hasn't been marked as
// volatile that should be!

uint16_t gu16PSoCDiff[32] = { 0 };
// Touch threshold as calculated by SmartSense. Has hysteresis built in.
// 0 is an insane default, so instead we default to 100 which is far less likely to break things :)
uint16_t gu16PSoCThreshold[32] = {
    100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
    100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100,
};

uint32_t gu32PSoCDigital = 0;
uint32_t gu32PSoCDigitalPos = 0;
uint32_t gu32PSoCDigitalNeg = 0;
uint32_t gu32PSoCDigitalTrig = 0;
uint16_t gu16PSoCDigital = 0;
uint16_t gu16PSoCDigitalPos = 0;
uint16_t gu16PSoCDigitalNeg = 0;
uint16_t gu16PSoCDigitalTrig = 0;

uint32_t gu32LastCapSenseStart = 0;
uint32_t gu32LastCapSenseEnd = 0;

volatile uint8_t bPSoCDirtyVolatile = 0;
volatile uint8_t bPSoCAliveVolatile = 0;
volatile uint8_t bForceSliderSend = 0;

static volatile void* pu8PsocRxDestination = NULL;
static uint8_t pu8PsocRxDestinationLen = 0;
static volatile uint8_t* volatile pu8PsocGotData = NULL;
static volatile PSoC_CMD_RX eBlockingCommand = _PSoC_CMD_RX_NONE;
static void PSoC_HandleRx(PSoC_CMD_RX eCmd, uint8_t u8Len, uint8_t* u8Data) {
    if (eCmd != PSoC_CMD_RX_SLAVE_DIFF && eCmd != PSoC_CMD_RX_MASTER_DIFF &&
        eCmd != PSoC_CMD_RX_CS_START) {
        static uint8_t debug = 0;
        debug++;
    }

    switch (eCmd) {
        case PSoC_CMD_RX_INITIALISATION_COMPLETE:
            bPSoCAliveVolatile = 1;
            return;

        // Debug traces
        case PSoC_CMD_RX_CS_START:
            gu32LastCapSenseStart = gu32NowMs;
            return;
        case PSoC_CMD_RX_CS_END:
            gu32LastCapSenseEnd = gu32NowMs;
            return;

        case PSoC_CMD_RX_SLAVE_DIFF:
            // Request a refresh of our touch thresholds
            PSoC_GetDebug(PSoC_CMD_DEBUG_SLAVE_TOUCH_TH, NULL);

            if (u8Len != 32) return;
            for (uint8_t i = 0; i < 16; i++) {
                gu16PSoCDiff[16 + i] = u8Data[(i * 2)] << 8;
                gu16PSoCDiff[16 + i] |= u8Data[(i * 2) + 1];
            }
            bPSoCDirtyVolatile = 1;
            bPSoCAliveVolatile = 1;
            return;

        case PSoC_CMD_RX_MASTER_DIFF:
            // Request a refresh of our touch thresholds
            PSoC_GetDebug(PSoC_CMD_DEBUG_MASTER_TOUCH_TH, NULL);

            if (u8Len != 32) return;
            for (uint8_t i = 0; i < 16; i++) {
                gu16PSoCDiff[i] = u8Data[(i * 2)] << 8;
                gu16PSoCDiff[i] |= u8Data[(i * 2) + 1];
            }
            bPSoCDirtyVolatile = 1;
            bPSoCAliveVolatile = 1;
            return;

        // Arbitrary data reception
        case PSoC_CMD_RX_MASTER_TOUCH_TH:
            if (u8Len == 32) {
                for (uint8_t i = 0; i < 16; i++) {
                    gu16PSoCThreshold[i] = u8Data[(i * 2)] << 8;
                    gu16PSoCThreshold[i] |= u8Data[(i * 2) + 1];
                }
            }
            goto arbitrary_data_reception;
        case PSoC_CMD_RX_SLAVE_TOUCH_TH:
            if (u8Len == 32) {
                for (uint8_t i = 0; i < 16; i++) {
                    gu16PSoCThreshold[16 + i] = u8Data[(i * 2)] << 8;
                    gu16PSoCThreshold[16 + i] |= u8Data[(i * 2) + 1];
                }
            }
            goto arbitrary_data_reception;

        case PSoC_CMD_RX_MASTER_FINGER_TH:
        case PSoC_CMD_RX_MASTER_HYSTERESIS:
        case PSoC_CMD_RX_SLAVE_FINGER_TH:
        case PSoC_CMD_RX_SLAVE_HYSTERESIS:
        case PSoC_CMD_RX_GET_FINGER_CAP:
        case PSoC_CMD_RX_SET_FINGER_CAP:
        case PSoC_CMD_RX_ENABLE_DEBUG:
        arbitrary_data_reception:
            // Only handle the command if it's what we were actually blocking on
            if (eCmd != eBlockingCommand) {
                // This was a rogue command! (breakpoint here for PSoC diagnosis stuff)
                return;
            }

            if (pu8PsocRxDestination) {
                if (u8Len > pu8PsocRxDestinationLen) u8Len = pu8PsocRxDestinationLen;
                memcpy((void*)pu8PsocRxDestination, u8Data, u8Len);
                // Make sure we don't go clobbering stuff later!
                pu8PsocRxDestination = NULL;
            }
            if (pu8PsocGotData) {
                *pu8PsocGotData = 1;
                // Make sure we don't go clobbering stuff later!
                pu8PsocGotData = NULL;
            }
            return;

        // Nonsense, but just to satisfy the switch condition validation :)
        case _PSoC_CMD_RX_NONE:
            return;
    }
}

/**
 * @brief Returns 1 on successful reception of a command, 0 otherwise
 *
 * @param eCommand
 * @return uint8_t
 */
static volatile uint8_t su8Ready = 0;
static uint8_t PSoC_Await_Command(PSoC_CMD_RX eCommand) {
    su8Ready = 0;
    pu8PsocGotData = &su8Ready;

    // TODO: What timeout does this actually work out to be?
    // With O0, it looks like /33 = 1 second (ie the loop is 33 clocks)
    // Haven't tested Os which is what we'd want for prod
    uint32_t u32Timeout = (SystemCoreClock / 33) / 16;  // 62.5ms

    eBlockingCommand = eCommand;
    // Block on reception of the command
    while (!su8Ready) {
        // Timeout condition
        if (--u32Timeout == 0) {
            eBlockingCommand = _PSoC_CMD_RX_NONE;
            return 0;
        }
    }
    eBlockingCommand = _PSoC_CMD_RX_NONE;
    return 1;
}
static uint8_t PSoC_Valid_Len(PSoC_CMD_RX eCmd, uint8_t u8Len) {
    switch (eCmd) {
        case PSoC_CMD_RX_CS_START:
        case PSoC_CMD_RX_CS_END:
            // We should never be seeing a length for these!
            return 0;

        // TODO: This doesn't seem to be a packet!
        case PSoC_CMD_RX_INITIALISATION_COMPLETE:
            return 0;

        // Commands that have a payload we need to receive
        case PSoC_CMD_RX_SET_FINGER_CAP:
        case PSoC_CMD_RX_ENABLE_DEBUG:
            return u8Len == 2;
        case PSoC_CMD_RX_MASTER_DIFF:
        case PSoC_CMD_RX_SLAVE_DIFF:
        case PSoC_CMD_RX_MASTER_TOUCH_TH:
        case PSoC_CMD_RX_SLAVE_TOUCH_TH:
        case PSoC_CMD_RX_MASTER_FINGER_TH:
        case PSoC_CMD_RX_MASTER_HYSTERESIS:
        case PSoC_CMD_RX_SLAVE_FINGER_TH:
        case PSoC_CMD_RX_SLAVE_HYSTERESIS:
        case PSoC_CMD_RX_GET_FINGER_CAP:
            return u8Len == 0x20;

        // Nonsense
        case _PSoC_CMD_RX_NONE:
            return 0;
    }
    return 0;
}

static inline uint8_t PSoC_Validate_Sum(PSoC_CMD_RX eCmd, uint8_t u8Observed, uint8_t u8Received) {
    switch (eCmd) {
        // These four commands are broken, and their checksum is always zero
        case PSoC_CMD_RX_MASTER_FINGER_TH:
        case PSoC_CMD_RX_MASTER_HYSTERESIS:
        case PSoC_CMD_RX_SLAVE_FINGER_TH:
        case PSoC_CMD_RX_SLAVE_HYSTERESIS:
            return u8Received == 0;
        default:
            return u8Received == u8Observed;
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
            switch (u8Data) {
                // Commands with no payload
                case PSoC_CMD_RX_CS_START:
                case PSoC_CMD_RX_CS_END:
                    // PSoC_HandleRx(u8Data, 0, NULL);
                    return;
                case PSoC_CMD_RX_INITIALISATION_COMPLETE:
                    PSoC_HandleRx(u8Data, 0, NULL);
                    return;

                // Commands that have a payload we need to receive
                case PSoC_CMD_RX_SET_FINGER_CAP:
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

                // Unknown command
                default:
                    su8State = 0;
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

            if (!PSoC_Valid_Len(eCmd, su8Len)) {
                // This isn't the right length for this command, so we know we're out of sync
                su8State = 0;
            }
            return;
        case 2:
            su8Sum += u8Data;
            su8RxData[su8Index++] = u8Data;
            if (su8Index == su8Len) {
                su8State++;
            }
            return;
        case 3:
            // Only process packets with a valid checksum
            if (PSoC_Validate_Sum(eCmd, su8Sum, u8Data)) {
                PSoC_HandleRx(eCmd, su8Len, su8RxData);
                su8State = 0;
            } else {
                // For breakpoints
                su8State = 0;
            }
            return;

        // If we somehow land in an invalid state (should be impossible), make sure we can recover
        default:
            su8State = 0;
            return;
    }
}

static inline void _UART_Write(uint8_t u8Data) {
    while (UART_IS_TX_FULL(UART1))
        ;
    UART_WRITE(UART1, u8Data);
}
static inline void PSoC_Cmd(PSoC_CMD_TX eCmd, uint8_t u8D0, uint8_t u8D1, PSoC_CMD_RX eBlocking) {
    // The protocol for the PSoCs has no sync byte
    // This means it's quite likely we're going to sometimes be parsing in the middle of a packets,
    // causing us to potentially lose future packets that we wanted to receive.
    //
    // To avoid this situation, we're going to repeatedly send the packet on a timeout.
    //
    // If the PSoC is totally non-responsive this will deadlock. For now that's probably fine. We
    // can add a retry counter down the line if it causes problems.
    do {
        _UART_Write(eCmd);
        _UART_Write(2);
        _UART_Write(u8D0);
        _UART_Write(u8D1);
        _UART_Write(eCmd + 2 + u8D0 + u8D1);

        if (eBlocking == _PSoC_CMD_RX_NONE) break;
        if (PSoC_Await_Command(eBlocking)) break;

        // The issue with sync bytes goes both ways. Our packets are 5 bytes long, and the PSoC will
        // error out if the second byte isn't a 2 (and stop reading), so by ensuring we send an
        // odd-length stream we'll be back in sync eventually
        //
        // It can take quite a few failed packets to bring the PSoC back into sync. Once we have the
        // PSoC in sync, it should stay in sync for the duration of the program execution. We
        // perform a blocking sensitivity assignment at the start of the program, which is hopefully
        // where this resync should ocur if required.
        //
        //
        // ...that's all in theory though.
        // In practice it seems if we ever break sync the PSoC will need a hard reset.
    } while (1);
}

uint16_t PSoC_GetFingerCapacitance(void) {
    static volatile uint16_t u16FingerCap = 0;
    pu8PsocRxDestination = &u16FingerCap;
    pu8PsocRxDestinationLen = sizeof u16FingerCap;
    PSoC_Cmd(PSoC_CMD_TX_GET_FINGER_CAP, 0, 0, PSoC_CMD_RX_GET_FINGER_CAP);

    return BYTESWAP_U16(u16FingerCap);
}
void PSoC_GetDebug(PSoC_CMD_DEBUG u8Cmd, uint8_t* pu8Data) {
    pu8PsocRxDestination = pu8Data;
    pu8PsocRxDestinationLen = pu8Data ? 32 : 0;

    PSoC_CMD_RX eRet = _PSoC_CMD_RX_NONE;
    if (pu8Data != NULL) {
        switch (u8Cmd) {
            case PSoC_CMD_DEBUG_MASTER_TOUCH_TH:
                eRet = PSoC_CMD_RX_MASTER_TOUCH_TH;
                break;
            case PSoC_CMD_DEBUG_SLAVE_TOUCH_TH:
                eRet = PSoC_CMD_RX_SLAVE_TOUCH_TH;
                break;
            case PSoC_CMD_DEBUG_MASTER_FINGER_TH:
                eRet = PSoC_CMD_RX_MASTER_FINGER_TH;
                break;
            case PSoC_CMD_DEBUG_SLAVE_FINGER_TH:
                eRet = PSoC_CMD_RX_SLAVE_FINGER_TH;
                break;
            case PSoC_CMD_DEBUG_MASTER_HYSTERESIS:
                eRet = PSoC_CMD_RX_MASTER_HYSTERESIS;
                break;
            case PSoC_CMD_DEBUG_SLAVE_HYSTERESIS:
                eRet = PSoC_CMD_RX_SLAVE_HYSTERESIS;
                break;
        }
    }
    PSoC_Cmd(PSoC_CMD_TX_GET_DEBUG, 0, u8Cmd, eRet);
}
void PSoC_SetFingerCapacitance(uint16_t u16FingerCap, uint8_t u8Blocking) {
    if (u16FingerCap > 1023) u16FingerCap = 1023;
    PSoC_Cmd(PSoC_CMD_TX_SET_FINGER_CAP, u16FingerCap >> 8, u16FingerCap & 0xff,
             u8Blocking ? PSoC_CMD_RX_SET_FINGER_CAP : _PSoC_CMD_RX_NONE);
}
void PSoC_SetFingerCapacitanceFromConfig(uint8_t u8Blocking) {
    PSoC_SetFingerCapacitance(PSoC_FINGER_CAP_MIN + (16 - gConfig.u8Sens) * PSoC_FINGER_CAP_STEP,
                              u8Blocking);
}
void PSoC_SetDebug(PSoC_DebugFlag eFlag, uint8_t bEnable) {
    PSoC_Cmd(PSoC_CMD_TX_ENABLE_DEBUG, eFlag, bEnable, PSoC_CMD_RX_ENABLE_DEBUG);
}

uint8_t gu8GroundData[32];

void PSoC_PostProcessing(void) {
    // Process the raw PSoC data to compute our external 0-255 values
    for (uint8_t i = 0; i < 32; i++) {
        uint8_t j = (15 - (i / 2)) * 2 + (i & 1);

        const uint16_t u16Pad = gu16PSoCDiff[i];
        const uint16_t u16TouchTh = gu16PSoCThreshold[i];

        // Chunithm defaults to a threshold of 20
        // As such, we need to scale u16Pad such that (0,u16TouchTh)=(0,20)
        //
        // This can go over 255, so make sure to clamp it
        uint16_t u16Val = (u16Pad * 20) / u16TouchTh;
        if (u16Val > 255)
            gu8GroundData[j] = 255;
        else
            gu8GroundData[j] = u16Val;

        // const uint16_t u16Min = gConfig.u16PSoCScaleMin[i];
        // const uint16_t u16Max = gConfig.u16PSoCScaleMax[i];

        // if (u16Pad < u16Min) {
        //     gu8GroundData[j] = 0;
        // } else if (u16Pad > u16Max) {
        //     gu8GroundData[j] = 255;
        // } else {
        //     // Multiply by 255 first to retain precision
        //     // This can overflow u16, hence the u32 cast pre-mult
        //     gu8GroundData[j] = ((uint32_t)(u16Pad - u16Min) * 255) / (u16Max - u16Min);
        // }
    }

    bForceSliderSend = 1;
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

// #pragma GCC diagnostic pop
// #pragma GCC pop_options
