#ifndef SLIDER_HOST_CHECK
#include "tasoller.h"
#endif
#include "config_api.h"

#define SLIDER_SYNC 0xFF
#define SLIDER_MARK 0xFD
#define SLIDER_MARKED_SYNC 0xFE
#define SLIDER_MARKED_MARK 0xFC

static uint8_t su8AutoEnabled = 0;
static uint8_t su8AutoEnabledRaw = 0;
static uint8_t su8AutoEnabledByte = 0;
static uint32_t su32SinceLastControlled = 0;

uint8_t gu8GameBrightness = 40;

static uint16_t su16ByteRawSliderOffset = 0;
static uint8_t su8ByteRawSliderShift = 0;

typedef enum {
    SLIDER_PARSE_SYNC_WAIT = 0,
    SLIDER_PARSE_CMD,
    SLIDER_PARSE_NDATA,
    SLIDER_PARSE_DATA,
    SLIDER_PARSE_CHECKSUM,
} slider_parse_state;

// Hardware information for a Chunithm slider
static const slider_cmd_Tx_hw_info sSliderHwInfo = {
    "15330   ", 0xA0, "06712", 0xFF, 0x90, 0, 0,
};
void Slider_Exception(uint8_t u8SliderCmd, slider_exception u8Exc);

static inline void Slider_Write(uint8_t u8Byte) {
    if (u8Byte == SLIDER_SYNC || u8Byte == SLIDER_MARK) {
        USB_VCOM_Write(SLIDER_MARK);
        u8Byte--;
    }
    USB_VCOM_Write(u8Byte);
}
static void Slider_Respond(slider_cmd_Tx u8SliderCmd, const uint8_t* pu8Packet, uint8_t u8NPacket) {
    uint8_t u8Sum = SLIDER_SYNC + u8SliderCmd + u8NPacket;
    USB_VCOM_Write(SLIDER_SYNC);  // We don't want to escape sync!
    Slider_Write(u8SliderCmd);
    Slider_Write(u8NPacket);

    for (uint16_t i = 0; i < u8NPacket; i++) {
        u8Sum += pu8Packet[i];
        Slider_Write(pu8Packet[i]);
    }

    Slider_Write(-u8Sum);
}
uint8_t u8Null64[64] = { 0 };
static void Slider_Send_Report(void) {
    if (gbUIOpen) {
        Slider_Respond(SLIDER_CMD_Tx_REPORT, u8Null64, sizeof gu8GroundData);
    } else {
        Slider_Respond(SLIDER_CMD_Tx_REPORT, gu8GroundData, sizeof gu8GroundData);
    }
}
static void Slider_Send_Report_Raw(void) {
    if (gbUIOpen) {
        Slider_Respond(SLIDER_CMD_Tx_RAW, u8Null64, sizeof gu16PSoCDiff);
    } else {
        Slider_Respond(SLIDER_CMD_Tx_RAW, (void*)gu16PSoCDiff, sizeof gu16PSoCDiff);
    }
}
static void Slider_Send_Report_Byte(void) {
    slider_cmd_Tx_raw buffer;
    for (uint8_t i = 0; i < 32; i++) {
        if (gbUIOpen) {
            buffer.u16Raw[i] = 0;
        } else {
            buffer.u16Raw[i] = (gu16PSoCDiff[i] >> su8ByteRawSliderShift) - su16ByteRawSliderOffset;
        }
    }
    Slider_Respond(SLIDER_CMD_Tx_RAW, (void*)&buffer, sizeof buffer);
}
static void Slider_Process(slider_cmd_Rx u8SliderCmd, uint8_t* pu8Packet, uint8_t u8NPacket) {
    if (u8SliderCmd >= CONFIG_GET_INFO && u8SliderCmd <= CONFIG_SET_PROFILE) {
        uint8_t response[2 + KEYMAP_SIZE] = { CONFIG_API_VERSION, CONFIG_OK };
        uint8_t length = 2;
        if (u8SliderCmd == CONFIG_SET_KEYMAP ? (u8NPacket != 41 && u8NPacket != 43) :
            u8NPacket != (u8SliderCmd == CONFIG_SET ? 15 :
                          u8SliderCmd == CONFIG_GET_PROFILE ? 2 :
                          u8SliderCmd == CONFIG_SET_PROFILE ? 34 : 1))
            response[1] = CONFIG_LENGTH;
        else if (pu8Packet[0] != CONFIG_API_VERSION)
            response[1] = CONFIG_VERSION;
        else if (u8SliderCmd == CONFIG_SET && Config_Validate(pu8Packet + 1, 14))
            response[1] = CONFIG_VALUE;
        else if (u8SliderCmd == CONFIG_SET_KEYMAP &&
             (u8NPacket == 41 ? !Keymap_Valid(pu8Packet + 1) :
              !Keymap_GroupValid(pu8Packet + 3, pu8Packet[1], pu8Packet[2])))
            response[1] = CONFIG_VALUE;
          else if ((u8SliderCmd == CONFIG_GET_PROFILE || u8SliderCmd == CONFIG_SET_PROFILE) &&
                 (pu8Packet[1] > KEYMAP_2K ||
                (u8SliderCmd == CONFIG_SET_PROFILE && !Keymap_ProfileValid(pu8Packet + 2, pu8Packet[1]))))
            response[1] = CONFIG_VALUE;
        else if ((u8SliderCmd == CONFIG_SET || u8SliderCmd == CONFIG_SAVE ||
                  u8SliderCmd == CONFIG_RESET || u8SliderCmd == CONFIG_SET_KEYMAP ||
                  u8SliderCmd == CONFIG_SET_PROFILE) && gbUIOpen)
            response[1] = CONFIG_BUSY;
        else {
            switch ((uint8_t)u8SliderCmd) {
                case CONFIG_GET_INFO: {
                    static const uint8_t info[16] = { 3, 1, 3, 0, 0x7F, 0, 0, 0,
                                                      'T', 'A', 'S', '-', 'H', 'O', 'S', 'T' };
                    memcpy(response + 2, info, sizeof info);
                    length += sizeof info;
                    break;
                }
                case CONFIG_GET:
                    Config_Encode(&gConfig, response + 2);
                    length += CONFIG_API_SIZE;
                    break;
                case CONFIG_SET:
                case CONFIG_RESET: {
                    uint8_t previousSens = gConfig.u8Sens;
                    if (u8SliderCmd == CONFIG_SET) Config_Apply(&gConfig, pu8Packet + 1);
                    else FMC_ConfigDefaults();
                    if (previousSens != gConfig.u8Sens) PSoC_SetFingerCapacitanceFromConfig(0);
                    break;
                }
                case CONFIG_SAVE:
                    if (FMC_EEPROM_Save()) response[1] = CONFIG_STORAGE;
                    break;
                case CONFIG_GET_KEYMAP:
                    memcpy(response + 2, gConfig.u8Keymap, KEYMAP_SIZE);
                    length += KEYMAP_SIZE;
                    break;
                case CONFIG_SET_KEYMAP:
                    // Complete validation above; all consumers run in the main loop.
                    Keymap_Switch(&gConfig, u8NPacket == 43 ? pu8Packet[1] : KEYMAP_32K);
                    memcpy(gConfig.u8Keymap, pu8Packet + (u8NPacket == 43 ? 3 : 1), KEYMAP_SIZE);
                    Keymap_SetProfile(&gConfig, gConfig.u8KeyboardMode, gConfig.u8Keymap);
                    if (u8NPacket == 43) gConfig.u8DividerMode = pu8Packet[2];
                    break;
                case CONFIG_GET_PROFILE:
                    memcpy(response + 2, pu8Packet[1] == gConfig.u8KeyboardMode ?
                           gConfig.u8Keymap : gConfig.u8KeymapProfiles[pu8Packet[1]], 32);
                    length += 32;
                    break;
                case CONFIG_SET_PROFILE:
                    Keymap_SetProfile(&gConfig, pu8Packet[1], pu8Packet + 2);
                    break;
                case CONFIG_GET_MODES:
                    response[2] = gConfig.u8KeyboardMode;
                    response[3] = gConfig.u8DividerMode;
                    length += 2;
                    break;
                case CONFIG_GET_INPUT:
                    memcpy(response + 2, gu8GroundData, 32);
                    response[34] = gu8DigitalButtons;
                    length += 33;
                    break;
            }
        }
        Slider_Respond((slider_cmd_Tx)u8SliderCmd, response, length);
        return;
    }
    switch (u8SliderCmd) {
        case SLIDER_CMD_Rx_RESET:
            // These three weren't present previously, but PSoC firmware suggests they should be
            su8AutoEnabled = 0;
            su8AutoEnabledRaw = 0;
            su8AutoEnabledByte = 0;
            // Real firmware can throw a bus exception, but we won't
            Slider_Respond(SLIDER_CMD_Tx_RESET, NULL, 0);
            return;
        case SLIDER_CMD_Rx_HW_INFO:
            Slider_Respond(SLIDER_CMD_Tx_HW_INFO, (void*)&sSliderHwInfo, sizeof sSliderHwInfo);
            return;
        case SLIDER_CMD_Rx_CPU_STATUS:
            slider_cmd_Tx_cpu_status Status = {
                {
                    .bGlobalInterrupt = 1,
                    .bWatchdogReset = 0,  // Report everything okay
                    .bPowerOnReset = 0,   // Cleared in entry0
                    .bSleep = 0,          // Obvious
                    .bStop = 0,           // Obvious
                },
                {
                    .bBootMultiple = 0,
                    .bSlowImo = 0,
                    .bEcoExistsWritten = 1,  // Set implicitly in entry0
                    .bEcoExists = 1,         // Set in entry0
                    .bSramWatchdog = 0,
                },
            };
            Slider_Respond(SLIDER_CMD_Tx_CPU_STATUS, (void*)&Status, sizeof Status);
            return;

        case SLIDER_CMD_Rx_LED:
        case SLIDER_CMD_Rx_REPORT_PING_PONG:
        case SLIDER_CMD_Rx_RAW_PING_PONG:
        case SLIDER_CMD_Rx_BRAW_PING_PONG:
            // We have 32 triples here because Chunithm's slider is barely different from Project
            // Diva's! We only care about the first 31 of them.
            if (u8NPacket == sizeof(slider_cmd_Rx_led)) {
                gu8GameBrightness = ((slider_cmd_Rx_led*)pu8Packet)->u8Brightness;

                gbLedDataIsControlledExt = 1;
                su32SinceLastControlled = 0;
                memcpy(gaControlledExtLedData, &((slider_cmd_Rx_led*)pu8Packet)->aBRG, 3 * 32);
            }
            // Reprocess this packet as a report request where applicable
            if (u8SliderCmd == SLIDER_CMD_Rx_REPORT_PING_PONG) {
                Slider_Process(SLIDER_CMD_Rx_REPORT, pu8Packet, u8NPacket);
            } else if (u8SliderCmd == SLIDER_CMD_Rx_RAW_PING_PONG) {
                Slider_Process(SLIDER_CMD_Rx_RAW, pu8Packet, u8NPacket);
            } else if (u8SliderCmd == SLIDER_CMD_Rx_BRAW_PING_PONG) {
                Slider_Process(SLIDER_CMD_Rx_BRAW, pu8Packet, u8NPacket);
            }
            return;

        case SLIDER_CMD_Rx_REPORT:
            Slider_Send_Report();
            return;
        case SLIDER_CMD_Rx_RAW:
            Slider_Send_Report_Raw();
            return;
        case SLIDER_CMD_Rx_BRAW:
            Slider_Send_Report_Byte();
            return;

        case SLIDER_CMD_Rx_REPORT_ENABLE:
            su8AutoEnabled = 1;
            // No response
            return;
        case SLIDER_CMD_Rx_RAW_ENABLE:
            su8AutoEnabledRaw = 1;
            // No response
            return;
        case SLIDER_CMD_Rx_BRAW_ENABLE:
            su8AutoEnabledByte = 1;
            // No response
            return;
        case SLIDER_CMD_Rx_REPORT_DISABLE:
            // Purge any Tx buffer from the auto sending
            if (su8AutoEnabled || su8AutoEnabledRaw || su8AutoEnabledByte) USB_VCOM_PurgeTx();
            su8AutoEnabled = 0;
            su8AutoEnabledRaw = 0;
            su8AutoEnabledByte = 0;
            Slider_Respond(SLIDER_CMD_Tx_REPORT_DISABLE, NULL, 0);
            return;

        case SLIDER_CMD_Rx_BRAW_SET_OFFSET:
            su16ByteRawSliderOffset = ((slider_cmd_Rx_braw_set_offset*)pu8Packet)->u16Offset;
            Slider_Respond(SLIDER_CMD_Tx_BRAW_SET_OFFSET, NULL, 0);
            return;
        case SLIDER_CMD_Rx_BRAW_SET_SHIFT:
            su8ByteRawSliderShift = ((slider_cmd_Rx_braw_set_shift*)pu8Packet)->u8Shift;
            Slider_Respond(SLIDER_CMD_Tx_BRAW_SET_SHIFT, NULL, 0);
            return;

        case SLIDER_CMD_Rx_DEBUG:
            uint8_t u8aData[32];

            switch ((slider_debug_cmd_Rx)pu8Packet[0]) {
                case SLIDER_DEBUG_CMD_Rx_GET_FINGER_CAP:
                    uint16_t u16FingerCap = PSoC_GetFingerCapacitance();
                    Slider_Respond(SLIDER_CMD_Tx_DEBUG, (uint8_t*)&u16FingerCap,
                                   sizeof u16FingerCap);
                    break;
                case SLIDER_DEBUG_CMD_Rx_TRACE_RESET:
                    // TODO: Broken. Blocks forever.
                    PSoC_SetDebug(PSoC_DebugFlag_TraceReset, 1);
                    Slider_Respond(SLIDER_CMD_Tx_DEBUG, NULL, 0);
                    break;
                case SLIDER_DEBUG_CMD_Rx_GET_LAST_CS_START:
                    Slider_Respond(SLIDER_CMD_Tx_DEBUG, (uint8_t*)&gu32LastCapSenseStart,
                                   sizeof gu32LastCapSenseStart);
                    break;
                case SLIDER_DEBUG_CMD_Rx_GET_LAST_CS_END:
                    Slider_Respond(SLIDER_CMD_Tx_DEBUG, (uint8_t*)&gu32LastCapSenseEnd,
                                   sizeof gu32LastCapSenseEnd);
                    break;

                case SLIDER_DEBUG_CMD_Rx_PSoC_REQUEST_DEBUG:
                    if (u8NPacket == 2) {
                        PSoC_GetDebug(pu8Packet[1], u8aData);
                    }
                    Slider_Respond(SLIDER_CMD_Tx_DEBUG, u8aData, sizeof u8aData);
                    break;

                case SLIDER_DEBUG_CMD_Rx_HOST_FMC_READ:
                    if (u8NPacket == 5) {
                        // We can't use a cast to uint32_t* because of unaligned reads!
                        uint32_t u32Base = pu8Packet[1];
                        u32Base |= pu8Packet[2] << 8;
                        u32Base |= pu8Packet[3] << 16;
                        u32Base |= pu8Packet[4] << 24;
                        memset(u8aData, 0xFF, sizeof u8aData);
                        FMC_Open();
                        FMC_ReadData(u32Base, u32Base + sizeof u8aData, (void*)u8aData);
                        FMC_Close();
                    }
                    Slider_Respond(SLIDER_CMD_Tx_DEBUG, u8aData, sizeof u8aData);
                    break;
                case SLIDER_DEBUG_CMD_Rx_LED_FMC_READ:
                    uint32_t u32Data = 0xFFFFFFFF;
                    if (u8NPacket == 5) {
                        uint32_t u32Offset = pu8Packet[1];
                        u32Offset |= pu8Packet[2] << 8;
                        u32Offset |= pu8Packet[3] << 16;
                        u32Offset |= pu8Packet[4] << 24;

                        LED_FMC_Read(u32Offset, &u32Data);
                    }
                    Slider_Respond(SLIDER_CMD_Tx_DEBUG, (uint8_t*)&u32Data, sizeof u32Data);
                    break;

                case SLIDER_DEBUG_CMD_Rx_HOST_ENTER_LDROM:
                    // Remember if we're going to want to kick the LED board into LDROM next reboot
                    gConfig.u8NextBootLEDBootloader = pu8Packet[1];
                    bConfigDirty = 1;
                    FMC_EEPROM_Store();

                    SYS_EnterLDROM();
                    break;
                case SLIDER_DEBUG_CMD_Rx_LED_ENTER_LDROM:
                    SYS_WaitBootloaderLED();
                    break;
                case SLIDER_DEBUG_CMD_Rx_LED_CHECK:
                    Slider_Respond(SLIDER_CMD_Tx_DEBUG, (uint8_t*)&gbLedIsCustom,
                                   sizeof gbLedIsCustom);
                    break;

                case SLIDER_DEBUG_CMD_Rx_GET_DIGITAL:
                    Slider_Respond(SLIDER_CMD_Tx_DEBUG, &gu8DigitalButtons,
                                   sizeof gu8DigitalButtons);
                    break;

                default:
                    Slider_Exception(u8SliderCmd, SLIDER_EXCEPTION_BUS_ERROR);
                    break;
            }
            return;

        default:
            Slider_Exception(u8SliderCmd, SLIDER_EXCEPTION_BUS_ERROR);
            break;
    }
}
void Slider_Exception(uint8_t u8SliderCmd, slider_exception u8Exc) {
    slider_cmd_TxRx_exception Packet = {
        u8SliderCmd,
        u8Exc,
    };
    Slider_Respond(SLIDER_CMD_Tx_EXCEPTION, (void*)&Packet, sizeof Packet);
}

void Slider_TickSerial(void) {
    /**
     * Byte 0: FF
     * Byte 1: Command
     * Byte 2: Length (n)
     * Byte [3~2+n]: Data
     * Byte [3+n]: Checksum
     *
     * Byte FD is escape; the next byte should +1'd
     */

    static slider_parse_state su8State = SLIDER_PARSE_SYNC_WAIT;
    static uint8_t u8Mark = 0;
    static uint8_t u8NPacket = 0;
    static uint8_t u8NRead = 0;
    static uint8_t u8Sum = 0;
    static uint8_t u8SliderCmd = 0;
    static uint32_t lastByteMs = 0;

    static uint8_t u8Packet[0x61];  // The largest inbound packet expected is to set LEDs

    if (su8State != SLIDER_PARSE_SYNC_WAIT && (uint32_t)(gu32NowMs - lastByteMs) >= 250) {
        su8State = SLIDER_PARSE_SYNC_WAIT;
        u8Mark = 0;
    }
    // Make sure we flush the buffer!
    while (USB_VCOM_Available()) {
        uint8_t u8Byte = USB_VCOM_Read();
        lastByteMs = gu32NowMs;
        // Raw FF always resynchronizes; an escaped FF reaches the switch as data.
        if (u8Byte == SLIDER_SYNC) {
            su8State = SLIDER_PARSE_CMD;
            u8Sum = SLIDER_SYNC;
            u8Mark = 0;
            continue;
        }
        if (su8State == SLIDER_PARSE_SYNC_WAIT) continue;
        if (u8Mark) {
            u8Mark = 0;
            if (u8Byte != SLIDER_MARKED_SYNC && u8Byte != SLIDER_MARKED_MARK) {
                su8State = SLIDER_PARSE_SYNC_WAIT;
                continue;
            }
            u8Byte++;
        } else if (u8Byte == SLIDER_MARK) {
            u8Mark = 1;
            continue;
        }

        u8Sum += u8Byte;
        switch (su8State) {
            case SLIDER_PARSE_SYNC_WAIT:
                u8Sum = 0xff;
                if (u8Byte == SLIDER_SYNC) su8State = SLIDER_PARSE_CMD;
                break;
            case SLIDER_PARSE_CMD:
                u8SliderCmd = u8Byte;
                su8State = SLIDER_PARSE_NDATA;
                break;
            case SLIDER_PARSE_NDATA:
                u8NPacket = u8Byte;
                u8NRead = 0;
                su8State = SLIDER_PARSE_DATA;

                // If this is more data than we could handle, just give up
                if (u8NPacket > sizeof u8Packet) su8State = SLIDER_PARSE_SYNC_WAIT;
                // If there's nothing to do.. do nothing!
                if (u8NPacket == 0) su8State = SLIDER_PARSE_CHECKSUM;
                break;
            case SLIDER_PARSE_DATA:
                u8Packet[u8NRead++] = u8Byte;
                if (u8NRead == u8NPacket) su8State = SLIDER_PARSE_CHECKSUM;
                break;
            case SLIDER_PARSE_CHECKSUM:
                // Only handle the packet if the sum equaled out
                if (u8Sum == 0) {
                    Slider_Process(u8SliderCmd, u8Packet, u8NPacket);
                } else {
                    Slider_Exception(u8SliderCmd, SLIDER_EXCEPTION_CHECKSUM);
                }

                su8State = SLIDER_PARSE_SYNC_WAIT;
                break;
        }
    }
}

void Slider_Tick1ms() {
    if (gbLedDataIsControlledExt) {
        // If we haven't had an LED packet in 1 second, call it quits
        if (++su32SinceLastControlled == 1000) gbLedDataIsControlledExt = 0;
    }

    static uint16_t u16Counter = 0;
    /**
     * I haven't totally tracked down the source of the interval timer on a real slider.
     * That said, from measurement it's 15.365ms or so. The exact time will be based on
     * a counter from one of the low speed clocks.
     *
     * The game makes calls to ReadFile on an 8ms interval, but this isn't the most stable.
     * I took a short capture, and my intervals were:
     *
     *   0% | 3.6ms
     *  10% | 7.0ms
     *  50% | 8.0ms
     *  90% | 9.0ms
     * 100% | 14.0ms
     *
     * This +-1ms appears to be caused by the use of Sleep() to regulate the interval, which
     * has an argument precision of 1ms (and an overall precision far worse!).
     *
     * Chunithm will retry a read four times, at which point it considers the slider to have
     * timed out (error 3100).
     *
     * Based on this, we should be safe to indeed run at a 15ms interval here.
     *
     * I received one report of a user getting a 3100 when they started the game. I am current
     * working on the basis that this is a one-off, however this will need re-addressed if this
     * issue becomes widespread.
     */
    if (++u16Counter != 15) return;
    u16Counter = 0;

    if (su8AutoEnabled) Slider_Send_Report();
    if (su8AutoEnabledRaw) Slider_Send_Report_Raw();
    if (su8AutoEnabledByte) Slider_Send_Report_Byte();
}
