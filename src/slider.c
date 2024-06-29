#include "tasoller.h"

#define SLIDER_SYNC 0xFF
#define SLIDER_MARK 0xFD

static uint8_t su8AutoEnabled = 0;
static uint8_t su8GotLedData = 0;
static uint32_t su32SinceLastControlled = 0;

typedef enum {
    SLIDER_PARSE_SYNC_WAIT = 0,
    SLIDER_PARSE_CMD,
    SLIDER_PARSE_NDATA,
    SLIDER_PARSE_DATA,
    SLIDER_PARSE_CHECKSUM,
} slider_parse_state;

typedef enum {
    SLIDER_CMD_AUTO = 0x01,
    SLIDER_CMD_SET_LED = 0x02,
    SLIDER_CMD_AUTO_START = 0x03,
    SLIDER_CMD_AUTO_STOP = 0x04,
    SLIDER_CMD_RESET = 0x10,
    SLIDER_CMD_GET_BOARD_INFO = 0xF0,
} slider_cmd;

static const uint8_t su8SliderVersion[32] = {
    '1',  '5', '3', '3', '0', ' ',  ' ', ' ', 0xA0,

    '0',  '6', '7', '1', '2', 0xFF,

    0x90,
};

static inline void Slider_Write(uint8_t u8Byte) {
    if (u8Byte == SLIDER_SYNC || u8Byte == SLIDER_MARK) {
        USB_VCOM_Write(SLIDER_MARK);
        u8Byte--;
    }
    USB_VCOM_Write(u8Byte);
}
static void Slider_Respond(slider_cmd u8SliderCmd, const uint8_t* pu8Packet, uint8_t u8NPacket) {
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
static void Slider_Process(slider_cmd u8SliderCmd, uint8_t* pu8Packet, uint8_t u8NPacket) {
    switch (u8SliderCmd) {
        case SLIDER_CMD_RESET:
            // Hmm? Do we not need to <su8AutoEnabled = 0;> here?
            Slider_Respond(SLIDER_CMD_RESET, NULL, 0);
            return;
        case SLIDER_CMD_GET_BOARD_INFO:
            Slider_Respond(SLIDER_CMD_GET_BOARD_INFO, su8SliderVersion, sizeof su8SliderVersion);
            return;

        case SLIDER_CMD_SET_LED:
            // TODO: What is the first byte of data? (00h and 28h observed)
            // Why are there 32 triples?
            if (u8NPacket == 1 + 0x60) {
                gbLedDataIsControlledExt = 1;
                su32SinceLastControlled = 0;
                memcpy(gu8aControlledExtLedData, &pu8Packet[1], 3 * 32);
            }
            su8GotLedData = 1;
            // No response
            return;

        case SLIDER_CMD_AUTO_START:
            su8AutoEnabled = 1;
            su8GotLedData = 1;
            // No response
            return;
        case SLIDER_CMD_AUTO_STOP:
            // Purge any Tx buffer from the auto sending
            if (su8AutoEnabled) USB_VCOM_PurgeTx();
            su8AutoEnabled = 0;
            Slider_Respond(SLIDER_CMD_AUTO_STOP, NULL, 0);
            return;

        // This is an outbound-only command, so we should never see it here!
        case SLIDER_CMD_AUTO:
        default:
            return;
    }
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
            u8Mark = 1;
            continue;
        } else if (u8Mark) {
            u8Mark = 0;
            // TODO: If u8Byte is 0xFD we should technically give up here
            u8Byte++;
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
                if (u8Sum == 0) Slider_Process(u8SliderCmd, u8Packet, u8NPacket);

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

        Slider_Respond(SLIDER_CMD_AUTO, gu8GroundData, sizeof gu8GroundData);
    }
}
