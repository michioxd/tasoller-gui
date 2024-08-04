#pragma once
#include <NUC123.h>
#include <stdio.h>
#include <string.h>

#include "_compiler.h"

#define ms *1000
#define kHz *1000
#define MHz *1000000

extern uint32_t gu32NowMs;
extern uint8_t gbUIOpen;

#define BYTESWAP_U16(x) ((((x) & 0xFF) << 8) | ((x) >> 8))

// === DAO-DRM ===
// * For now the bootloader check is being left enabled.
// * It'll help catch if I break that in my bootloader :P
#define ENABLE_BOOTLOADER_CHECK
#define BOOTLOADER_MAGIC 0x5555A320
#define BOOTLOADER_MAGIC_ADDR 0x100FF8

// === Local files ===
#include "fmc_user.h"
#include "led.h"
#include "pins.h"
#include "psoc.h"
#include "slider.h"
#include "usb_def.h"
#include "vcom.h"
#include "io4.h"

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
#define _IO4_FUNCTION                                  \
    "GOUT=14_"  /* General purpose output (20)      */ \
    "ADIN=8,E_" /* ADC input (8)                    */ \
    "ROTIN=4_"  /* Rotary input (4)                 */ \
    "COININ=2_" /* Coin input (2)                   */ \
    "SWIN=2,E_" /* Switch inputs (2)                */ \
    "UQ1=41,6;" /* Unique 1 (Command: 41h, bytes 6) */
#define IO4_PRODUCT _IO4_PRODUCT _IO4_FUNCTION
#define IO4_VID 0x0CA3
#define IO4_PID 0x0021

/**
 * WARNING: This is both used internally and sent verbatim over serial
 * It **MUST** be 32 bytes of ground data
 */
extern uint8_t gu8GroundData[32];

extern uint8_t gu8DigitalButtons;

// === descriptors.c, used in usbd_driver.c ===
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

// === sys.c ===
void SYS_Init(void);
void SYS_Bootloader_Check(void);
void SYS_ModuleInit(void);
void SYS_EnterLDROM(void);
void SYS_WaitBootloaderLED(void);
extern volatile uint8_t gu8Do250usTick;
extern volatile uint8_t gu8Do1msTick;

// === ui.c ===
void UI_Tick(void);

// === Misc ===
#define HEX_NIBBLE(x) ("0123456789ABCDEF"[(x)&0xf])
#define DELAY_CYCLES_2 __asm__ volatile("NOP\nNOP\n");

#define INCR(x, y) ((x) = (x) < (y) ? (x) + 1 : (y))
#define DECR(x, y) ((x) = (x) > (y) ? (x)-1 : (y))
#define MOD_INCR(x, y) ((x) = (x) == ((y)-1) ? 0 : ((x) + 1))
#define MOD_DECR(x, y) ((x) = (x) == 0 ? ((y)-1) : ((x)-1))
#define INV(x) ((x) = 1 - (x))

#define MS_SINCE(x)                                                      \
    ((gu32NowMs >= (x)) ? /* We haven't wrapped yet */ (gu32NowMs - (x)) \
                        : /* We've wrapped over (50 days!) */ (gu32NowMs + (0xFFFFFFFF - (x))))
