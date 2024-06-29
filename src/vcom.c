#include "tasoller.h"

#define BUF_SIZE_RX 512
#define BUF_SIZE_TX 512

volatile uint8_t gu8VcomReady = 0;

static volatile uint8_t gau8ComRbuf[BUF_SIZE_RX];
static volatile uint16_t gu16ComRbytes = 0;
static volatile uint16_t gu16ComRhead = 0;
static volatile uint16_t gu16ComRtail = 0;

static volatile uint8_t gau8ComTbuf[BUF_SIZE_TX];
static volatile uint16_t gu16ComTbytes = 0;
static volatile uint16_t gu16ComThead = 0;
static volatile uint16_t gu16ComTtail = 0;

uint8_t gau8RxBuf[64] = { 0 };
volatile uint8_t *gpu8RxBuf = 0;
volatile uint32_t gu32RxSize = 0;
volatile uint32_t gu32TxSize = 0;

volatile int8_t gi8BulkOutReady = 0;

void _USB_VCOM_Tick_Tx(void) {
    uint32_t u32Len;
    if (gu32TxSize != 0) return;

    // Check wether we have new COM Rx data to send to USB or not
    if (!gu16ComRbytes) {
        // Prepare a zero packet if previous packet size is USBD_CDC_IN_MAX_SIZE and
        // no more data to send at this moment to note Host the transfer has been done
        u32Len = USBD_GET_PAYLOAD_LEN(EP_CDC_IN);
        if (u32Len == USBD_CDC_IN_MAX_SIZE) USBD_SET_PAYLOAD_LEN(EP_CDC_IN, 0);
        return;
    }

    u32Len = gu16ComRbytes;
    if (u32Len > USBD_CDC_IN_MAX_SIZE) u32Len = USBD_CDC_IN_MAX_SIZE;

    for (uint32_t i = 0; i < u32Len; i++) {
        if (gu16ComRhead >= BUF_SIZE_RX) gu16ComRhead = 0;
        gau8RxBuf[i] = gau8ComRbuf[gu16ComRhead++];
    }

    // __set_PRIMASK(1);
    gu16ComRbytes -= u32Len;
    // __set_PRIMASK(0);

    gu32TxSize = u32Len;
    USBD_MemCopy((uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP_CDC_IN)), (uint8_t *)gau8RxBuf,
                 u32Len);
    USBD_SET_PAYLOAD_LEN(EP_CDC_IN, u32Len);
}
void _USB_VCOM_Tick_Rx(void) {
    // Process the Bulk out data when bulk out data is ready.
    if (!gi8BulkOutReady) return;
    if (gu32RxSize > BUF_SIZE_TX - gu16ComTbytes) return;

    for (uint32_t i = 0; i < gu32RxSize; i++) {
        gau8ComTbuf[gu16ComTtail++] = gpu8RxBuf[i];
        if (gu16ComTtail >= BUF_SIZE_TX) gu16ComTtail = 0;
    }

    // __set_PRIMASK(1);
    gu16ComTbytes += gu32RxSize;
    // __set_PRIMASK(0);

    gu32RxSize = 0;
    gi8BulkOutReady = 0;  // Clear bulk out ready flag

    // Ready to get next BULK out
    USBD_SET_PAYLOAD_LEN(EP_CDC_OUT, USBD_CDC_OUT_MAX_SIZE);
}
void USB_VCOM_Tick(void) {
    _USB_VCOM_Tick_Tx();
    _USB_VCOM_Tick_Rx();
}

uint8_t USB_VCOM_Read() {
    if (!gu16ComTbytes) return 0xff;
    gu16ComTbytes--;
    if (gu16ComThead >= BUF_SIZE_TX) gu16ComThead = 0;
    return gau8ComTbuf[gu16ComThead++];
}
void USB_VCOM_Write(uint8_t u8Data) {
    if (gu16ComRbytes < BUF_SIZE_RX) {
        // Enqueue the character
        gau8ComRbuf[gu16ComRtail++] = u8Data;
        if (gu16ComRtail >= BUF_SIZE_RX) gu16ComRtail = 0;
        gu16ComRbytes++;
    } else {
        // FIFO over run
    }
}

uint16_t USB_VCOM_Available(void) { return gu16ComTbytes; }
void USB_VCOM_PurgeTx(void) {
    gu16ComRbytes = 0;
    gu16ComRhead = 0;
    gu16ComRtail = 0;
}
