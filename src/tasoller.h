#pragma once
#include <NUC123.h>
#include <stdio.h>
#include <string.h>

#define ms *1000
#define kHz *1000

/* === LEDs === */
#define LED_Tx_BUFFER 254
// Defined as volatile due to the I2C interrupt reading at random times!
extern volatile uint8_t gu8LEDTx[LED_Tx_BUFFER];

/* === DAO-DRM === */
// * For now the bootloader check is being left enabled.
// * It'll help catch if I break that in my bootloader :P
#define ENABLE_BOOTLOADER_CHECK
#define BOOTLOADER_MAGIC 0x5555A320
#define BOOTLOADER_MAGIC_ADDR 0x100FF8

/* === USB definitions === */
#include "usb_inc/hid.h"
#include "usb_inc/keymap.h"
#include "usb_inc/usb.h"

/* === Local files === */
#include "fmc_user.h"
#include "led.h"
#include "pins.h"
#include "psoc.h"
#include "slider.h"

#define IO4_VENDOR "SEGA INTERACTIVE"
// Product description
#define _IO4_PRODUCT                          \
    "I/O CONTROL BD;" /* Board type        */ \
    "15257   ;"       /* Board number      */ \
    "01;"             /* Mode              */ \
    "90;"             /* Firmware revision */ \
    "1831;"           /* Firmware checksum */ \
    "6679A;"          /* Chip number       */ \
    "00;"             /* Config            */
// Functional description
#define _IO4_FUNCTION                             \
    "GOUT=14_"  /* General purpose output (20) */ \
    "ADIN=8,E_" /* ADC input (8)               */ \
    "ROTIN=4_"  /* Rotary input (4)            */ \
    "COININ=2_" /* Coin input (2)              */ \
    "SWIN=2,E_" /* Switch inputs (2)           */ \
    "UQ1=41,6;" /* Unique function 1 (?)       */
#define IO4_PRODUCT _IO4_PRODUCT _IO4_FUNCTION
#define IO4_VID 0x0CA3
#define IO4_PID 0x0021

#define INCR(x, y) ((x) = (x) < (y) ? (x) + 1 : (y))
#define DECR(x, y) ((x) = (x) > (y) ? (x) - 1 : (y))
#define MOD_INCR(x, y) ((x) = (x) == ((y)-1) ? 0 : ((x) + 1))
#define MOD_DECR(x, y) ((x) = (x) == 0 ? ((y)-1) : ((x)-1))
#define INV(x) ((x) = 1 - (x))

extern volatile uint8_t gu8VcomReady;
/**
 * WARNING: This is both used internally and sent verbatim over serial
 * It **MUST** be 32 bytes of ground data
 */
extern uint8_t gu8GroundData[32];

/* === For HID === */
enum {
    HID_REPORT_ID_IO4 = 1,
    HID_REPORT_ID_KEYBOARD,
    HID_REPORT_ID_DEBUG_A,
    HID_REPORT_ID_DEBUG_B,
    HID_REPORT_ID_IO4_CMD = 16,
};

#define NUM_FN 2
#define NUM_AIR 6
#define NUM_GROUND 32

typedef struct __attribute__((packed)) {
    uint8_t bReportId;
    uint8_t bKeyboard[NUM_FN + NUM_AIR + NUM_GROUND];
} hid_report_t;
typedef struct __attribute__((packed)) {
    uint8_t bReportId;
    uint16_t wADC[8];
    uint16_t wRotary[4];
    uint16_t wCoin[2];
    uint16_t wButtons[2];
    uint8_t bSystemStatus;
    uint8_t bUsbStatus;
    uint8_t bUnique[29];
} io4_hid_in_t;
typedef struct __attribute__((packed)) {
    uint8_t bReportId;
    uint8_t bCmd;
    uint8_t bData[62];
} io4_hid_out_t;
typedef struct __attribute__((packed)) {
    uint8_t bReportId;
    uint16_t wData[16];
} debug_hid_report_t;

extern uint8_t gu8DigitalButtons;

// void HID_Tick();
void USBD_HID_PrepareReport();
uint8_t *USBD_HID_GetReport(uint8_t u8ReportId, uint32_t *pu32Size);
void USBD_HID_SetReport(volatile uint8_t *pu8EpBuf, uint32_t u32Size);

// For CDC
typedef struct __attribute__((packed)) {
    uint32_t u32DTERate;   // Baud rate
    uint8_t u8CharFormat;  // Stop bit
    uint8_t u8ParityType;  // Parity
    uint8_t u8DataBits;    // Data bits
} STR_VCOM_LINE_CODING;
extern volatile int8_t gi8BulkOutReady;
extern STR_VCOM_LINE_CODING gLineCoding;
extern uint16_t gCtrlSignal;
extern volatile uint16_t comRbytes;
extern volatile uint16_t comRhead;
extern volatile uint16_t comRtail;
extern volatile uint16_t comTbytes;
extern volatile uint16_t comThead;
extern volatile uint16_t comTtail;
extern volatile uint8_t *gpu8RxBuf;
extern volatile uint32_t gu32RxSize;
extern volatile uint32_t gu32TxSize;
// For HID
extern uint8_t volatile gu8HIDIO4Ready;
extern uint8_t volatile gu8HIDMiscReady;
// General USB control
extern uint8_t volatile g_u8Suspend;
extern uint8_t g_u8Idle;
extern uint8_t g_u8Protocol;

extern const char *gszVendorInitial;
extern const char *gszVendor;
extern const char *gszProduct;
extern const usb_device_descr_t *gpDeviceDescriptor;
extern const usb_desc_config_t *gpConfigDescriptor;
extern const uint32_t gu32HidDescIO4Offset;
extern const uint32_t gu32HidDescMiscOffset;
extern const uint32_t gu32UsbHidIO4ReportLen;
extern const uint32_t gu32UsbHidMiscReportLen;
extern const uint8_t *gpu8UsbHidIO4Report;
extern const uint8_t *gpu8UsbHidMiscReport;

/*-------------------------------------------------------------*/

// Interfaces
enum {
    USBD_ITF_CDC_CMD,
    USBD_ITF_CDC_DAT,
    USBD_ITF_HID_IO4,
    USBD_ITF_HID_MISC,
    _USBD_ITF_MAX,
};

// Endpoint number mapping
#define EP_CTRL_IN EP0
#define EP_CTRL_OUT EP1
#define EP_CDC_IN EP2
#define EP_CDC_OUT EP3
#define EP_CDC_CMD EP4
#define EP_HID_IO4_IN EP5
#define EP_HID_MISC_IN EP6
#define EP_HID_MISC_OUT EP7

#define _USBD_INTSTS(x) USBD_INTSTS_EP##x
#define USBD_INTSTS(x) _USBD_INTSTS(x)

// Must match the above!!
#define USBD_INTSTS_CTRL_IN USBD_INTSTS(EP_CTRL_IN)
#define USBD_INTSTS_CTRL_OUT USBD_INTSTS(EP_CTRL_OUT)
#define USBD_INTSTS_CDC_IN USBD_INTSTS(EP_CDC_IN)
#define USBD_INTSTS_CDC_OUT USBD_INTSTS(EP_CDC_OUT)
#define USBD_INTSTS_CDC_CMD USBD_INTSTS(EP_CDC_CMD)
#define USBD_INTSTS_HID_IO4_IN USBD_INTSTS(EP_HID_IO4_IN)
#define USBD_INTSTS_HID_MISC_IN USBD_INTSTS(EP_HID_MISC_IN)
#define USBD_INTSTS_HID_MISC_OUT USBD_INTSTS(EP_HID_MISC_OUT)

#define USBD_CDC_EP_IN (1 | EP_INPUT)
#define USBD_CDC_EP_OUT (2 | EP_OUTPUT)
#define USBD_CDC_EP_CMD (3 | EP_INPUT)
#define USBD_HID_IO4_EP_IN (4 | EP_INPUT)
#define USBD_HID_MISC_EP_IN (5 | EP_INPUT)
#define USBD_HID_MISC_EP_OUT (6 | EP_OUTPUT)

#define USBD_SETUP_BUF_LEN (8)
#define USBD_CDC_CMD_MAX_SIZE (16)
#define USBD_CDC_IN_MAX_SIZE (64)   // Device -> Host
#define USBD_CDC_OUT_MAX_SIZE (64)  // Host -> Device
#define USBD_HID_BUF_LEN (64)

// Endpoint packet max size (cannot total more than 512!)
#define EP0_MAX_PKT_SIZE 64
#define EP1_MAX_PKT_SIZE 64
#define EP2_MAX_PKT_SIZE USBD_CDC_IN_MAX_SIZE
#define EP3_MAX_PKT_SIZE USBD_CDC_OUT_MAX_SIZE
#define EP4_MAX_PKT_SIZE USBD_CDC_CMD_MAX_SIZE
#define EP5_MAX_PKT_SIZE USBD_HID_BUF_LEN
#define EP6_MAX_PKT_SIZE USBD_HID_BUF_LEN
#define EP7_MAX_PKT_SIZE USBD_HID_BUF_LEN

#define SETUP_BUF_BASE 0
#define SETUP_BUF_LEN 8

#if (SETUP_BUF_LEN + EP0_MAX_PKT_SIZE + EP1_MAX_PKT_SIZE + EP2_MAX_PKT_SIZE + EP3_MAX_PKT_SIZE + \
     EP4_MAX_PKT_SIZE + EP5_MAX_PKT_SIZE + EP6_MAX_PKT_SIZE + EP7_MAX_PKT_SIZE) > 512
#error USB endpoint packet sizes exceeds 512-byte maximum
#endif

#define EP0_BUF_BASE (SETUP_BUF_BASE + SETUP_BUF_LEN)
#define EP1_BUF_BASE (EP0_BUF_BASE + EP0_MAX_PKT_SIZE)
#define EP2_BUF_BASE (EP1_BUF_BASE + EP1_MAX_PKT_SIZE)
#define EP3_BUF_BASE (EP2_BUF_BASE + EP2_MAX_PKT_SIZE)
#define EP4_BUF_BASE (EP3_BUF_BASE + EP3_MAX_PKT_SIZE)
#define EP5_BUF_BASE (EP4_BUF_BASE + EP4_MAX_PKT_SIZE)
#define EP6_BUF_BASE (EP5_BUF_BASE + EP5_MAX_PKT_SIZE)
#define EP7_BUF_BASE (EP5_BUF_BASE + EP6_MAX_PKT_SIZE)

// Define Descriptor information
#define HID_IO4_INT_IN_INTERVAL 8
#define HID_DEFAULT_INT_IN_INTERVAL 1
#define HID_DEFAULT_INT_OUT_INTERVAL 1
#define USBD_SELF_POWERED 0
#define USBD_REMOTE_WAKEUP 0
#define USBD_MAX_POWER (500 / 2)

/*-------------------------------------------------------------*/
#define HEX_NIBBLE(x) ("0123456789ABCDEF"[(x)&0xf])

void SYS_Init(void);
void SYS_Bootloader_Check(void);
void SYS_ModuleInit(void);
extern volatile uint8_t gu8Do250usTick;
extern volatile uint8_t gu8Do1msTick;

void EP_CDC_CMD_Handler(void);
void EP_CDC_OUT_Handler(void);
void EP_CDC_IN_Handler(void);
void EP_HID_IO4_IN_Handler(void);
void EP_HID_MISC_IN_Handler(void);
void EP_HID_MISC_OUT_Handler(void);

void USB_VCOM_Write(uint8_t u8Char);
uint16_t USB_VCOM_Available(void);
uint8_t USB_VCOM_Read(void);
void USB_VCOM_Tick(void);
void USB_VCOM_PurgeTx(void);

void UI_Tick(void);

void DelayCycles(uint32_t u32Cycles);
#define DelayCycles_Small(x) \
    for (int i = 0; i < (x); i++) __asm__ volatile("" : "+g"(i) : :);

// Custom USBD implementation
void Tas_USBD_Open(void);
void Tas_USBD_Init(void);
void Tas_USBD_Start(void);
void Tas_USBD_ClassRequest(void);
void Tas_USBD_GetSetupPacket(usb_setup_t *buf);
void Tas_USBD_ProcessSetupPacket(void);
void Tas_USBD_PrepareCtrlIn(void *pu8Buf, uint32_t u32Size);
void Tas_USBD_CtrlIn(void);
void Tas_USBD_PrepareCtrlOut(void *pu8Buf, uint32_t u32Size,
                             void (*pCallback)(volatile uint8_t *, uint32_t));
void Tas_USBD_CtrlOut(void);
void Tas_USBD_SwReset(void);
extern volatile uint8_t *g_usbd_CtrlInPointer;
extern volatile uint32_t g_usbd_CtrlInSize;
