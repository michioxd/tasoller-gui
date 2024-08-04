#pragma once
#pragma once

#include <stdint.h>

#define USB_STATE_FLOATING 2
#define USB_STATE_SUSPEND 1

// USB definition files
#include "hid_def.h"
#include "usb_inc/hid.h"
#include "usb_inc/keymap.h"
#include "usb_inc/usb.h"

// Interfaces
enum : uint8_t {
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

_Static_assert(USBD_HID_BUF_LEN >= sizeof(hid_kbd_report_t) &&
                   USBD_HID_BUF_LEN >= sizeof(hid_consumer_report_t) &&
                   USBD_HID_BUF_LEN >= sizeof(io4_hid_in_t) &&
                   USBD_HID_BUF_LEN >= sizeof(io4_hid_out_t),
               "HID USB buffer insufficient size for possible reports");

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

_Static_assert((SETUP_BUF_LEN + EP0_MAX_PKT_SIZE + EP1_MAX_PKT_SIZE + EP2_MAX_PKT_SIZE +
                EP3_MAX_PKT_SIZE + EP4_MAX_PKT_SIZE + EP5_MAX_PKT_SIZE + EP6_MAX_PKT_SIZE +
                EP7_MAX_PKT_SIZE) <= 512,
               "USB endpoint packet sizes exceeds 512-byte maximum");

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
#define USBD_SELF_POWERED_Pos 6
#define USBD_SELF_POWERED_Msk (1 << USBD_SELF_POWERED_Pos)
#define USBD_REMOTE_WAKEUP_Pos 5
#define USBD_REMOTE_WAKEUP_Msk (1 << USBD_REMOTE_WAKEUP_Pos)
#define USBD_MAX_POWER (500 / 2)

// Endpoint handler functions (usbd_user.c)
void EP_CDC_OUT_Handler(void);
void EP_CDC_IN_Handler(void);
void EP_HID_IO4_IN_Handler(void);
void EP_HID_MISC_IN_Handler(void);
void EP_HID_MISC_OUT_Handler(void);

// Custom USBD implementation (usbd_driver.c)
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

// General USB control
extern volatile uint8_t g_u8UsbState;
extern uint8_t g_u8Idle;
extern uint8_t g_u8Protocol;
