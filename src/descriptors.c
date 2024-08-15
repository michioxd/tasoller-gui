#include <stddef.h>

#include "sys/syslimits.h"
#include "tasoller.h"

#define __
static const uint8_t IO4_ReportDescriptor[] = {
    // Analog input (28 bytes)
    HID_USAGE_PAGE(GENERIC_DESKTOP),
    HID_USAGE(JOYSTICK),
    HID_COLLECTION(APPLICATION),
    __ HID_REPORT_ID(HID_REPORT_ID_IO4),
    __ HID_USAGE(POINTER),
    __ HID_COLLECTION(PHYSICAL),
    // 8 ADC channels
    __ __ HID_USAGE(X),
    __ __ HID_USAGE(Y),
    __ __ HID_USAGE(X),
    __ __ HID_USAGE(Y),
    __ __ HID_USAGE(X),
    __ __ HID_USAGE(Y),
    __ __ HID_USAGE(X),
    __ __ HID_USAGE(Y),
    // 4 Rotary channels
    __ __ HID_USAGE(RX),
    __ __ HID_USAGE(RY),
    __ __ HID_USAGE(RX),
    __ __ HID_USAGE(RY),
    // 2 Coin chutes
    __ __ HID_USAGE(SLIDER),
    __ __ HID_USAGE(SLIDER),
    __ __ HID_LOGICAL_MINIMUM(1, 0),
    __ __ HID_LOGICAL_MAXIMUM(4, 65534),
    __ __ HID_PHYSICAL_MINIMUM(1, 0),
    __ __ HID_PHYSICAL_MAXIMUM(4, 65534),
    __ __ HID_REPORT_COUNT(14),
    __ __ HID_REPORT_SIZE(16),
    __ __ HID_INPUT(DATA, VARIABLE, ABSOLUTE, NO_WRAP, LINEAR, PREFERRED_STATE, NO_NULL_POSITION),
    __ HID_END_COLLECTION(PHYSICAL),
    // Digital input (6 bytes = 48 bits)
    // [ 0~15]: Player 1 buttons
    // [16~31]: Player 2 buttons
    // [32~39]: System status
    //      -> 01h: ?
    //      -> 02h: ?
    //      -> 04h: ?
    //      -> 08h: ?
    //      -> 10h: Comm timeout set
    //      -> 20h: Sampling count set
    // [40~47]: USB status
    //      -> 01h: ?
    //      -> 02h: ?
    //      -> 04h: ? (is set on timeout)
    __ HID_USAGE_PAGE(SIMULATION),
    __ HID_USAGE_PAGE(BUTTONS),
    __ HID_USAGE_MINIMUM(1, 1),
    __ HID_USAGE_MAXIMUM(1, 48),
    __ HID_LOGICAL_MINIMUM(1, 0),
    __ HID_LOGICAL_MAXIMUM(1, 1),
    __ HID_PHYSICAL_MAXIMUM(1, 1),
    __ HID_REPORT_SIZE(1),
    __ HID_REPORT_COUNT(48),
    __ HID_INPUT(DATA, VARIABLE, ABSOLUTE, NO_WRAP, LINEAR, PREFERRED_STATE, NO_NULL_POSITION),

    // Reserved for future use. Pad with null. (29 bytes)
    __ HID_USAGE(UNDEFINED),
    __ HID_REPORT_SIZE(8),
    __ HID_REPORT_COUNT(29),
    __ HID_INPUT(CONSTANT, ARRAY, ABSOLUTE, NO_WRAP, LINEAR, PREFERRED_STATE, NO_NULL_POSITION),
    __ HID_USAGE_PAGE2(2, 0xFFA0),  // Vendor defined FF0A
    __ HID_USAGE(UNDEFINED),

    // General-purpose commands to the board. First byte is the command, then 62 data byte
    __ HID_REPORT_ID(HID_REPORT_ID_IO4_CMD),
    __ HID_COLLECTION(APPLICATION),
    __ __ HID_USAGE(UNDEFINED),
    __ __ HID_LOGICAL_MINIMUM(1, 0),
    __ __ HID_LOGICAL_MAXIMUM(2, 255),
    __ __ HID_REPORT_SIZE(8),
    __ __ HID_REPORT_COUNT(63),
    __ __ HID_OUTPUT(DATA, VARIABLE, ABSOLUTE, NO_WRAP, LINEAR, PREFERRED_STATE, NO_NULL_POSITION,
                     NON_VOLATILE),
    __ HID_END_COLLECTION(APPLICATION),

    HID_END_COLLECTION(APPLICATION),
};

#define _FINGER_DESCRIPTOR                         \
    HID_USAGE(FINGER),                        /**/ \
        HID_COLLECTION(LOGICAL),              /**/ \
        HID_USAGE(TIP_SWITCH),                /**/ \
        HID_LOGICAL_MINIMUM(1, 0),            /**/ \
        HID_LOGICAL_MAXIMUM(1, 1),            /**/ \
        HID_REPORT_SIZE(1),                   /**/ \
        HID_REPORT_COUNT(1),                  /**/ \
        HID_INPUT(DATA, VARIABLE, ABSOLUTE),  /**/ \
        HID_REPORT_COUNT(7),                  /**/ \
        HID_INPUT(CONSTANT, ARRAY, ABSOLUTE), /**/ \
        HID_REPORT_SIZE(8),                   /**/ \
        HID_USAGE(CONTACT_IDENTIFIER),        /**/ \
        HID_REPORT_COUNT(1),                  /**/ \
        HID_INPUT(DATA, VARIABLE, ABSOLUTE),  /**/ \
        HID_USAGE_PAGE(GENERIC_DESKTOP),      /**/ \
        HID_LOGICAL_MAXIMUM(2, 127),          /**/ \
        HID_REPORT_SIZE(8),                   /**/ \
        HID_REPORT_COUNT(2),                  /**/ \
        HID_PHYSICAL_MINIMUM(1, 0),           /**/ \
        HID_PHYSICAL_MAXIMUM(2, 127),         /**/ \
        HID_USAGE(X),                         /**/ \
        HID_USAGE(Y),                         /**/ \
        HID_INPUT(DATA, VARIABLE, ABSOLUTE),  /**/ \
        HID_USAGE_PAGE(DIGITIZER),            /**/ \
        HID_USAGE(WIDTH),                     /**/ \
        HID_USAGE(HEIGHT),                    /**/ \
        HID_INPUT(DATA, VARIABLE, ABSOLUTE),  /**/ \
        HID_END_COLLECTION(LOGICAL)

static const uint8_t Keyboard_ReportDescriptor[] = {
    // Keyboard input report
    HID_USAGE_PAGE(GENERIC_DESKTOP),
    HID_USAGE(KEYBOARD),
    HID_COLLECTION(APPLICATION),
    __ HID_REPORT_ID(HID_REPORT_ID_KEYBOARD),
    __ HID_USAGE_PAGE(KEYBOARD),
    __ HID_LOGICAL_MINIMUM(1, 0),
    __ HID_LOGICAL_MAXIMUM(2, 231),
    __ HID_USAGE_MINIMUM(1, 0),
    __ HID_USAGE_MAXIMUM(1, 231),
    __ HID_REPORT_SIZE(8),
    __ HID_REPORT_COUNT(NUM_FN + NUM_AIR + NUM_GROUND),
    __ HID_INPUT(DATA, ARRAY, ABSOLUTE),
    HID_END_COLLECTION(APPLICATION),

    // Consumer control report
    HID_USAGE_PAGE(CONSUMER),
    HID_USAGE(CONSUMER_CONTROL),
    HID_COLLECTION(APPLICATION),
    __ HID_REPORT_ID(HID_REPORT_ID_CONSUMER_CONTROL),
    __ HID_USAGE_PAGE(CONSUMER),
    __ HID_USAGE_MINIMUM(1, 0),
    __ HID_USAGE_MAXIMUM(2, 0x0FFF),
    __ HID_LOGICAL_MINIMUM(1, 0),
    __ HID_LOGICAL_MAXIMUM(2, 0x0FFF),
    __ HID_REPORT_SIZE(16),
    __ HID_REPORT_COUNT(2),
    __ HID_INPUT(DATA, ARRAY, ABSOLUTE, NO_WRAP, LINEAR, PREFERRED_STATE, NO_NULL_POSITION),
    HID_END_COLLECTION(APPLICATION),

    // Report for sending the enter key
    HID_USAGE_PAGE(GENERIC_DESKTOP),
    HID_USAGE(KEYBOARD),
    HID_COLLECTION(APPLICATION),
    __ HID_REPORT_ID(HID_REPORT_ID_ENTER),
    __ HID_USAGE_PAGE(KEYBOARD),
    __ HID_LOGICAL_MINIMUM(1, 0),
    __ HID_LOGICAL_MAXIMUM(2, 231),
    __ HID_USAGE_MINIMUM(1, 0),
    __ HID_USAGE_MAXIMUM(1, 231),
    __ HID_REPORT_SIZE(8),
    __ HID_REPORT_COUNT(1),
    __ HID_INPUT(DATA, ARRAY, ABSOLUTE),
    HID_END_COLLECTION(APPLICATION),

#ifdef ENABLE_TOUCH_INPUT
    // Touch input report
    HID_USAGE_PAGE(DIGITIZER),
    HID_USAGE(TOUCH_SCREEN),
    HID_COLLECTION(APPLICATION),
    HID_REPORT_ID(HID_REPORT_ID_TOUCH),

    _FINGER_DESCRIPTOR,
    _FINGER_DESCRIPTOR,
    _FINGER_DESCRIPTOR,
    _FINGER_DESCRIPTOR,
    _FINGER_DESCRIPTOR,
    _FINGER_DESCRIPTOR,
    _FINGER_DESCRIPTOR,
    _FINGER_DESCRIPTOR,

    HID_USAGE_PAGE(DIGITIZER),
    HID_USAGE(CONTACT_COUNT),
    HID_LOGICAL_MAXIMUM(1, 127),
    HID_REPORT_COUNT(1),
    HID_REPORT_SIZE(8),
    HID_INPUT(DATA, VARIABLE, ABSOLUTE),
    // Aren't we meant to have contact count maximum? (id 55h)

    HID_END_COLLECTION(APPLICATION),
#endif
};

usb_device_descr_t gIO4DeviceDescriptor = {
    sizeof(usb_device_descr_t),
    DESC_DEVICE,
    0x0200,
    USB_CLASS_UNSPECIFIED,
    0,
    0,
    EP0_MAX_PKT_SIZE,
    // Unfortunately we're forced to always use this VID if we want IO4 support to work
    IO4_VID,
    IO4_PID,
    0x0100,
    USB_STRING_VENDOR,
    USB_STRING_PRODUCT,
    USB_STRING_SERIAL,
    1,
};

// We have a unified descriptor that covers
typedef struct __packed {
    const usb_desc_config_t Config;

    const usb_desc_iad_t CDC_IAD;

    const usb_desc_interface_t CDC_CMD_Interface;
    const usb_desc_cdc_header_t CDC_Header;
    const usb_desc_cdc_call_t CDC_Call;
    const usb_desc_cdc_acm_t CDC_ACM;
    const usb_desc_cdc_union_t CDC_Union;
    const usb_desc_endpoint_t CDC_CMD_Endpoint;

    const usb_desc_interface_t CDC_Interface;
    const usb_desc_endpoint_t CDC_IN_Endpoint;
    const usb_desc_endpoint_t CDC_OUT_Endpoint;

    const usb_desc_interface_t HID_IO4_Interface;
    const usb_desc_hid_t HID_IO4;
    const usb_desc_endpoint_t HID_IO4_EndpointIn;
    /**
     * We're meant to have an OUT endpoint for IO4, but IO4 uses the Win32 WriteFile API, which will
     * happily fall back to SET_REPORT calls on the control endpoints if there's no OUT endpoint.
     *
     * IO4 receives OUT packets at a 47ms interval on average, 25%=46.8ms, 75%=61.3ms
     * As such, we're not going to saturate our control endpoint by bootlegging off it for HID!
     */
    // const usb_desc_endpoint_t HID_EndpointOut;

    const usb_desc_interface_t HID_Misc_Interface;
    const usb_desc_hid_t HID_Misc;
    const usb_desc_endpoint_t HID_Misc_EndpointIn;
    const usb_desc_endpoint_t HID_Misc_EndpointOut;
} config_desc_t;
static const config_desc_t gConfigDescriptor = {
    // Config
    {
        sizeof(usb_desc_config_t),
        DESC_CONFIG,
        sizeof gConfigDescriptor,
        _USBD_ITF_MAX,
        0x01,
        0x00,
        0x80 | USBD_REMOTE_WAKEUP_Msk,
        USBD_MAX_POWER,
    },

    // CDC IAD
    {
        sizeof(usb_desc_iad_t),
        DESC_IAD,
        USBD_ITF_CDC_CMD,
        2,
        USB_CLASS_CDC,
        CDC_COMM_SUBCLASS_ABSTRACT_CONTROL_MODEL,
        CDC_COMM_PROTOCOL_NONE,
        USB_STRING_CDC,
    },

    // CDC Control
    {
        sizeof(usb_desc_interface_t),
        DESC_INTERFACE,
        USBD_ITF_CDC_CMD,
        0x00,
        1,
        USB_CLASS_CDC,
        CDC_COMM_SUBCLASS_ABSTRACT_CONTROL_MODEL,
        CDC_COMM_PROTOCOL_NONE,
        0,
    },
    {
        sizeof(usb_desc_cdc_header_t),
        DESC_CS_INTERFACE,
        CDC_FUNC_DESC_HEADER,
        0x0110,
    },
    {
        sizeof(usb_desc_cdc_call_t),
        DESC_CS_INTERFACE,
        CDC_FUNC_DESC_CALL_MANAGEMENT,
        0,
        USBD_ITF_CDC_DAT,
    },
    {
        sizeof(usb_desc_cdc_acm_t),
        DESC_CS_INTERFACE,
        CDC_FUNC_DESC_ABSTRACT_CONTROL_MANAGEMENT,
        // Supports set line coding
        2,
    },
    {
        sizeof(usb_desc_cdc_union_t),
        DESC_CS_INTERFACE,
        CDC_FUNC_DESC_UNION,
        USBD_ITF_CDC_CMD,
        USBD_ITF_CDC_DAT,
    },
    {
        sizeof(usb_desc_endpoint_t),
        DESC_ENDPOINT,
        USBD_CDC_EP_CMD,
        EP_INT,
        USBD_CDC_CMD_MAX_SIZE,
        1,
    },
    // CDC Data
    {
        sizeof(usb_desc_interface_t),
        DESC_INTERFACE,
        USBD_ITF_CDC_DAT,
        0,
        2,
        USB_CLASS_CDC_DATA,
        0,
        0,
        0,
    },
    {
        sizeof(usb_desc_endpoint_t),
        DESC_ENDPOINT,
        USBD_CDC_EP_IN,
        EP_BULK,
        USBD_CDC_IN_MAX_SIZE,
        0,
    },
    {
        sizeof(usb_desc_endpoint_t),
        DESC_ENDPOINT,
        USBD_CDC_EP_OUT,
        EP_BULK,
        USBD_CDC_OUT_MAX_SIZE,
        0,
    },

    // IO4 HID
    {
        sizeof(usb_desc_interface_t),
        DESC_INTERFACE,
        USBD_ITF_HID_IO4,
        0x00,
        1,
        USB_CLASS_HID,
        0,
        HID_KEYBOARD,
        USB_STRING_HID_IO4,
    },
    {
        sizeof(usb_desc_hid_t),
        DESC_HID,
        0x0110,
        0x00,
        0x01,
        DESC_HID_RPT,
        sizeof IO4_ReportDescriptor,
    },
    {
        sizeof(usb_desc_endpoint_t),
        DESC_ENDPOINT,
        USBD_HID_IO4_EP_IN,
        EP_INT,
        USBD_HID_BUF_LEN_IO4,
        HID_IO4_INT_IN_INTERVAL,
    },

    // Misc HID
    {
        sizeof(usb_desc_interface_t),
        DESC_INTERFACE,
        USBD_ITF_HID_MISC,
        0x00,
        2,
        USB_CLASS_HID,
        0,
        HID_KEYBOARD,
        USB_STRING_HID_MISC,
    },
    {
        sizeof(usb_desc_hid_t),
        DESC_HID,
        0x0110,
        0x00,
        0x01,
        DESC_HID_RPT,
        sizeof Keyboard_ReportDescriptor,
    },
    {
        sizeof(usb_desc_endpoint_t),
        DESC_ENDPOINT,
        USBD_HID_MISC_EP_IN,
        EP_INT,
        USBD_HID_BUF_LEN_IN,
        HID_DEFAULT_INT_IN_INTERVAL,
    },
    {
        sizeof(usb_desc_endpoint_t),
        DESC_ENDPOINT,
        USBD_HID_MISC_EP_OUT,
        EP_INT,
        USBD_HID_BUF_LEN_OUT,
        HID_DEFAULT_INT_IN_INTERVAL,
    },
};

const char* gszVendor = IO4_VENDOR;
const char* gszProduct = "TASOLLER";

const usb_device_descr_t* gpDeviceDescriptor = &gIO4DeviceDescriptor;
const usb_desc_config_t* gpConfigDescriptor = &gConfigDescriptor.Config;
const uint32_t gu32HidDescIO4Offset = (offsetof(config_desc_t, HID_IO4));
const uint32_t gu32HidDescMiscOffset = (offsetof(config_desc_t, HID_Misc));
const uint32_t gu32UsbHidIO4ReportLen = sizeof IO4_ReportDescriptor;
const uint32_t gu32UsbHidMiscReportLen = sizeof Keyboard_ReportDescriptor;
const uint8_t* gpu8UsbHidIO4Report = (uint8_t*)IO4_ReportDescriptor;
const uint8_t* gpu8UsbHidMiscReport = (uint8_t*)Keyboard_ReportDescriptor;
