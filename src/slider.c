#include "tasoller.h"

#define SLIDER_SYNC 0xFF
#define SLIDER_MARK 0xFD
#define SLIDER_MARKED_SYNC 0xFE
#define SLIDER_MARKED_MARK 0xFC

static uint8_t su8AutoEnabled = 0;
static uint8_t su8AutoEnabledRaw = 0;
static uint8_t su8AutoEnabledByte = 0;
static uint32_t su32SinceLastControlled = 0;

uint8_t gu8GameBrightness = 40;  // TODO: Actually make use of this

static uint16_t su16ByteRawSliderOffset = 0;
static uint8_t su8ByteRawSliderShift = 0;

typedef enum {
    SLIDER_PARSE_SYNC_WAIT = 0,
    SLIDER_PARSE_CMD,
    SLIDER_PARSE_NDATA,
    SLIDER_PARSE_DATA,
    SLIDER_PARSE_CHECKSUM,
} slider_parse_state;

static const slider_cmd_Tx_hw_info sSliderHwInfo = {
    "15330   ", 0xA0, "06712", 0xFF, 0x90, 0, 0,
};

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
static void Slider_Process(slider_cmd_Rx u8SliderCmd, uint8_t* pu8Packet, uint8_t u8NPacket) {
    switch (u8SliderCmd) {
        case SLIDER_CMD_Rx_RESET:
            // These three weren't present previously, but PSoC firmware suggests they should be
            // TODO: Validate this against the game!
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
                memcpy(gu8aControlledExtLedData, &((slider_cmd_Rx_led*)pu8Packet)->aBRG, 3 * 32);
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
            Slider_Respond(SLIDER_CMD_Tx_REPORT, gu8GroundData, sizeof gu8GroundData);
            return;
        case SLIDER_CMD_Rx_RAW:
            // TODO:
            return;
        case SLIDER_CMD_Rx_BRAW:
            // TODO:
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

    static uint8_t u8Packet[0x61];  // The largest inbound packet expected is to set LEDs

    // Make sure we flush the buffer!
    while (USB_VCOM_Available()) {
        uint8_t u8Byte = USB_VCOM_Read();
        if (u8Byte == SLIDER_MARK) {
            // Multiple marks in a row get folded down into a single mark
            u8Mark = 1;
            continue;
        } else if (u8Mark) {
            u8Mark = 0;
            // Only unescape if the byte was actually escaped
            // A mark followed by any other byte is a no-op
            if (u8Byte == SLIDER_MARKED_SYNC || u8Byte == SLIDER_MARKED_MARK) {
                u8Byte++;
            }
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
        // If we haven't had an LED packet in 5 seconds, call it quits
        if (++su32SinceLastControlled == 5 * 1000) gbLedDataIsControlledExt = 0;
    }

    if (su8AutoEnabled) {
        static uint8_t u8Counter = 0;
        // Only actually send an update every 8ms
        if (++u8Counter != 8) return;

        u8Counter = 0;

        Slider_Respond(SLIDER_CMD_Tx_REPORT, gu8GroundData, sizeof gu8GroundData);
    }
}
