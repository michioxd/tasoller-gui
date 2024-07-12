#include <stddef.h>

#include "tasoller.h"

static const uint8_t IO4_ReportDescriptor[] = {
    // Analog input (28 bytes)
    HID_USAGE_PAGE(GENERIC_DESKTOP),
    HID_USAGE(JOYSTICK),
    HID_COLLECTION(APPLICATION),
    HID_REPORT_ID(HID_REPORT_ID_IO4),
    HID_USAGE(POINTER),
    HID_COLLECTION(PHYSICAL),
    // 8 ADC channels
    HID_USAGE(X),
    HID_USAGE(Y),
    HID_USAGE(X),
    HID_USAGE(Y),
    HID_USAGE(X),
    HID_USAGE(Y),
    HID_USAGE(X),
    HID_USAGE(Y),
    // 4 Rotary channels
    HID_USAGE(RX),
    HID_USAGE(RY),
    HID_USAGE(RX),
    HID_USAGE(RY),
    // 2 Coin chutes
    HID_USAGE(SLIDER),
    HID_USAGE(SLIDER),
    HID_LOGICAL_MINIMUM(1, 0),
    HID_LOGICAL_MAXIMUM(4, 65534),
    HID_PHYSICAL_MINIMUM(1, 0),
    HID_PHYSICAL_MAXIMUM(4, 65534),
    HID_REPORT_COUNT(14),
    HID_REPORT_SIZE(16),
    HID_INPUT(DATA, VARIABLE, ABSOLUTE, NO_WRAP, LINEAR, PREFERRED_STATE, NO_NULL_POSITION),
    HID_END_COLLECTION(PHYSICAL),

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
    HID_USAGE_PAGE(SIMULATION),
    HID_USAGE_PAGE(BUTTONS),
    HID_USAGE_MINIMUM(1, 1),
    HID_USAGE_MAXIMUM(1, 48),
    HID_LOGICAL_MINIMUM(1, 0),
    HID_LOGICAL_MAXIMUM(1, 1),
    HID_PHYSICAL_MAXIMUM(1, 1),
    HID_REPORT_SIZE(1),
    HID_REPORT_COUNT(48),
    HID_INPUT(DATA, VARIABLE, ABSOLUTE, NO_WRAP, LINEAR, PREFERRED_STATE, NO_NULL_POSITION),

    // Reserved for future use. Pad with null. (29 bytes)
    HID_USAGE(UNDEFINED),
    HID_REPORT_SIZE(8),
    HID_REPORT_COUNT(29),
    HID_INPUT(CONSTANT, ARRAY, ABSOLUTE, NO_WRAP, LINEAR, PREFERRED_STATE, NO_NULL_POSITION),
    HID_USAGE_PAGE2(2, 0xFFA0),  // Vendor defined FF0A
    HID_USAGE(UNDEFINED),

    // General-purpose commands to the board. First byte is the command, then 62 data byte
    HID_REPORT_ID(HID_REPORT_ID_IO4_CMD),
    HID_COLLECTION(APPLICATION),
    HID_USAGE(UNDEFINED),
    HID_LOGICAL_MINIMUM(1, 0),
    HID_LOGICAL_MAXIMUM(1, 255),
    HID_REPORT_SIZE(8),
    HID_REPORT_COUNT(63),
    HID_OUTPUT(DATA, VARIABLE, ABSOLUTE, NO_WRAP, LINEAR, PREFERRED_STATE, NO_NULL_POSITION,
               NON_VOLATILE),
    HID_END_COLLECTION(APPLICATION),

    HID_END_COLLECTION(APPLICATION),
};

static const uint8_t Keyboard_ReportDescriptor[] = {
    // Keyboard input descriptor
    HID_USAGE_PAGE(GENERIC_DESKTOP),
    HID_USAGE(KEYBOARD),
    HID_COLLECTION(APPLICATION),
    HID_REPORT_ID(HID_REPORT_ID_KEYBOARD),

    HID_USAGE_PAGE(KEYBOARD),
    HID_LOGICAL_MINIMUM(1, 0),
    HID_LOGICAL_MAXIMUM(1, 231),
    HID_USAGE_MINIMUM(1, 0),
    HID_USAGE_MAXIMUM(1, 231),
    HID_REPORT_SIZE(8),
    HID_REPORT_COUNT(NUM_FN + NUM_AIR + NUM_GROUND),
    HID_INPUT(DATA, ARRAY, ABSOLUTE),

    HID_END_COLLECTION(APPLICATION),

    // Debugging reports descriptors (they dump the raw PSoC data)
        HID_USAGE_PAGE(GENERIC_DESKTOP),
    HID_USAGE(JOYSTICK),
    HID_COLLECTION(APPLICATION),

    HID_REPORT_ID(HID_REPORT_ID_DEBUG_A),
    HID_USAGE(POINTER),
    HID_COLLECTION(LOGICAL),
    HID_USAGE(X),
    HID_USAGE(Y),
    HID_USAGE(X),
    HID_USAGE(Y),
    HID_USAGE(X),
    HID_USAGE(Y),
    HID_USAGE(X),
    HID_USAGE(Y),
    HID_USAGE(X),
    HID_USAGE(Y),
    HID_USAGE(X),
    HID_USAGE(Y),
    HID_USAGE(X),
    HID_USAGE(Y),
    HID_USAGE(X),
    HID_USAGE(Y),
    HID_LOGICAL_MINIMUM(1, 0),
    HID_LOGICAL_MAXIMUM(4, 0xffff),
    HID_PHYSICAL_MINIMUM(1, 0),
    HID_PHYSICAL_MAXIMUM(4, 0xffff),
    HID_REPORT_COUNT(16),
    HID_REPORT_SIZE(16),
    HID_INPUT(DATA, VARIABLE, ABSOLUTE),
    HID_END_COLLECTION(LOGICAL),

    HID_REPORT_ID(HID_REPORT_ID_DEBUG_B),
    HID_USAGE(POINTER),
    HID_COLLECTION(LOGICAL),
    HID_USAGE(X),
    HID_USAGE(Y),
    HID_USAGE(X),
    HID_USAGE(Y),
    HID_USAGE(X),
    HID_USAGE(Y),
    HID_USAGE(X),
    HID_USAGE(Y),
    HID_USAGE(X),
    HID_USAGE(Y),
    HID_USAGE(X),
    HID_USAGE(Y),
    HID_USAGE(X),
    HID_USAGE(Y),
    HID_USAGE(X),
    HID_USAGE(Y),
    HID_LOGICAL_MINIMUM(1, 0),
    HID_LOGICAL_MAXIMUM(4, 0xffff),
    HID_PHYSICAL_MINIMUM(1, 0),
    HID_PHYSICAL_MAXIMUM(4, 0xffff),
    HID_REPORT_SIZE(16),
    HID_REPORT_COUNT(16),
    HID_INPUT(DATA, VARIABLE, ABSOLUTE),
    HID_END_COLLECTION(LOGICAL),

    HID_END_COLLECTION(APPLICATION),
};

static const uint8_t Debug_ReportDescriptor[] = {
    HID_USAGE_PAGE(GENERIC_DESKTOP),
    HID_USAGE(JOYSTICK),
    HID_COLLECTION(APPLICATION),

    HID_REPORT_ID(HID_REPORT_ID_DEBUG_A),
    HID_USAGE(POINTER),
    HID_COLLECTION(LOGICAL),
    HID_USAGE(X),
    HID_USAGE(Y),
    HID_USAGE(X),
    HID_USAGE(Y),
    HID_USAGE(X),
    HID_USAGE(Y),
    HID_USAGE(X),
    HID_USAGE(Y),
    HID_USAGE(X),
    HID_USAGE(Y),
    HID_USAGE(X),
    HID_USAGE(Y),
    HID_USAGE(X),
    HID_USAGE(Y),
    HID_USAGE(X),
    HID_USAGE(Y),
    HID_LOGICAL_MINIMUM(1, 0),
    HID_LOGICAL_MAXIMUM(4, 0xffff),
    HID_PHYSICAL_MINIMUM(1, 0),
    HID_PHYSICAL_MAXIMUM(4, 0xffff),
    HID_REPORT_COUNT(16),
    HID_REPORT_SIZE(16),
    HID_INPUT(DATA, VARIABLE, ABSOLUTE),
    HID_END_COLLECTION(LOGICAL),

    HID_REPORT_ID(HID_REPORT_ID_DEBUG_B),
    HID_USAGE(POINTER),
    HID_COLLECTION(LOGICAL),
    HID_USAGE(X),
    HID_USAGE(Y),
    HID_USAGE(X),
    HID_USAGE(Y),
    HID_USAGE(X),
    HID_USAGE(Y),
    HID_USAGE(X),
    HID_USAGE(Y),
    HID_USAGE(X),
    HID_USAGE(Y),
    HID_USAGE(X),
    HID_USAGE(Y),
    HID_USAGE(X),
    HID_USAGE(Y),
    HID_USAGE(X),
    HID_USAGE(Y),
    HID_LOGICAL_MINIMUM(1, 0),
    HID_LOGICAL_MAXIMUM(4, 0xffff),
    HID_PHYSICAL_MINIMUM(1, 0),
    HID_PHYSICAL_MAXIMUM(4, 0xffff),
    HID_REPORT_SIZE(16),
    HID_REPORT_COUNT(16),
    HID_INPUT(DATA, VARIABLE, ABSOLUTE),
    HID_END_COLLECTION(LOGICAL),

    HID_END_COLLECTION(APPLICATION),
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
     * Chunithm never uses the output capability of IO4, so we have no high-frequency data that
     * might choke our control endpoints--we just need to support the initial configuration packets.
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
        0x80 | (USBD_SELF_POWERED << 6) | (USBD_REMOTE_WAKEUP << 5),
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
        0x01,
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
        0x01,
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
        USBD_HID_BUF_LEN,
        HID_IO4_INT_IN_INTERVAL,
    },

    // Misc HID
    {
        sizeof(usb_desc_interface_t),
        DESC_INTERFACE,
        USBD_ITF_HID_MISC,
        0x00,
        0x01,
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
        USBD_HID_BUF_LEN,
        HID_DEFAULT_INT_IN_INTERVAL,
    },
    {
        sizeof(usb_desc_endpoint_t),
        DESC_ENDPOINT,
        USBD_HID_MISC_EP_OUT,
        EP_INT,
        USBD_HID_BUF_LEN,
        HID_DEFAULT_INT_IN_INTERVAL,
    },
};

const char* gszVendorInitial = "Bottersnike";
const char* gszVendor = IO4_VENDOR;
const char* gszProduct = "TASOLLER";

const usb_device_descr_t *gpDeviceDescriptor = &gIO4DeviceDescriptor;
const usb_desc_config_t *gpConfigDescriptor = &gConfigDescriptor.Config;
const uint32_t gu32HidDescIO4Offset = (offsetof(config_desc_t, HID_IO4));
const uint32_t gu32HidDescMiscOffset = (offsetof(config_desc_t, HID_Misc));
const uint32_t gu32UsbHidIO4ReportLen = sizeof IO4_ReportDescriptor;
const uint32_t gu32UsbHidMiscReportLen = sizeof Keyboard_ReportDescriptor;
const uint8_t* gpu8UsbHidIO4Report = (uint8_t*)IO4_ReportDescriptor;
const uint8_t* gpu8UsbHidMiscReport = (uint8_t*)Keyboard_ReportDescriptor;
