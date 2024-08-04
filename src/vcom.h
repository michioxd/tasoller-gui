#pragma once

#include <stdint.h>

typedef struct __packed {
    uint32_t u32DTERate;   // Baud rate
    uint8_t u8CharFormat;  // Stop bit
    uint8_t u8ParityType;  // Parity
    uint8_t u8DataBits;    // Data bits
} STR_VCOM_LINE_CODING;
extern STR_VCOM_LINE_CODING gLineCoding;
extern volatile int8_t gi8BulkOutReady;

extern volatile uint8_t *gpu8RxBuf;
extern volatile uint32_t gu32RxSize;
extern volatile uint32_t gu32TxSize;

extern volatile uint8_t gu8VComDTEPresent;
extern volatile uint8_t gu8VComReady;

void USB_VCOM_Write(uint8_t u8Char);
uint16_t USB_VCOM_Available(void);
uint8_t USB_VCOM_Read(void);
void USB_VCOM_Tick(void);
void USB_VCOM_PurgeTx(void);
