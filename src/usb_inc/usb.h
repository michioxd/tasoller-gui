#pragma once
#include <stdint.h>

// USB U16 helpers
#define U16(_high, _low) ((uint16_t)(((_high) << 8) | (_low)))
#define U16_HIGH(_u16) ((uint8_t)(((_u16) >> 8) & 0x00ff))
#define U16_LOW(_u16) ((uint8_t)((_u16)&0x00ff))
#define U16_TO_U8S_BE(_u16) U16_HIGH(_u16), U16_LOW(_u16)
#define U16_TO_U8S_LE(_u16) U16_LOW(_u16), U16_HIGH(_u16)
#define U16_TO_U8S_BE_A(_u16) \
    { U16_TO_U8S_BE(_u16) }
#define U16_TO_U8S_LE_A(_u16) \
    { U16_TO_U8S_LE(_u16) }

// USB String helpers
enum {
    USB_STRING_LANG = 0,
    USB_STRING_VENDOR,
    USB_STRING_PRODUCT,
    USB_STRING_SERIAL,
    USB_STRING_CDC,
    USB_STRING_HID_IO4,
    USB_STRING_HID_MISC,
};

// USB Spec definitions
#define DESC_IAD 0x0B
#define DESC_CS_DEVICE 0x21
#define DESC_CS_CONFIGURATION 0x22
#define DESC_CS_STRING 0x23
#define DESC_CS_INTERFACE 0x24
#define DESC_CS_ENDPOINT 0x25

#define USB_CLASS_UNSPECIFIED 0
#define USB_CLASS_AUDIO 1
#define USB_CLASS_CDC 2
#define USB_CLASS_HID 3
#define USB_CLASS_RESERVED_4 4
#define USB_CLASS_PHYSICAL 5
#define USB_CLASS_IMAGE 6
#define USB_CLASS_PRINTER 7
#define USB_CLASS_MSC 8
#define USB_CLASS_HUB 9
#define USB_CLASS_CDC_DATA 10
#define USB_CLASS_SMART_CARD 11
#define USB_CLASS_RESERVED_12 12
#define USB_CLASS_CONTENT_SECURITY 13
#define USB_CLASS_VIDEO 14
#define USB_CLASS_PERSONAL_HEALTHCARE 15
#define USB_CLASS_AUDIO_VIDEO 16

#define USB_CLASS_DIAGNOSTIC 0xDC
#define USB_CLASS_WIRELESS_CONTROLLER 0xE0
#define USB_CLASS_MISC 0xEF
#define USB_CLASS_APPLICATION_SPECIFIC 0xFE
#define USB_CLASS_VENDOR_SPECIFIC 0xFF

#define CDC_COMM_SUBCLASS_DIRECT_LINE_CONTROL_MODEL 0x01
#define CDC_COMM_SUBCLASS_ABSTRACT_CONTROL_MODEL 0x02
#define CDC_COMM_SUBCLASS_TELEPHONE_CONTROL_MODEL 0x03
#define CDC_COMM_SUBCLASS_MULTICHANNEL_CONTROL_MODEL 0x04
#define CDC_COMM_SUBCLASS_CAPI_CONTROL_MODEL 0x05
#define CDC_COMM_SUBCLASS_ETHERNET_CONTROL_MODEL 0x06
#define CDC_COMM_SUBCLASS_ATM_NETWORKING_CONTROL_MODEL 0x07
#define CDC_COMM_SUBCLASS_WIRELESS_HANDSET_CONTROL_MODEL0x08
#define CDC_COMM_SUBCLASS_DEVICE_MANAGEMENT 0x09
#define CDC_COMM_SUBCLASS_MOBILE_DIRECT_LINE_MODEL 0x0A
#define CDC_COMM_SUBCLASS_OBEX 0x0B
#define CDC_COMM_SUBCLASS_ETHERNET_EMULATION_MODEL 0x0C
#define CDC_COMM_SUBCLASS_NETWORK_CONTROL_MODEL 0x0D

#define CDC_COMM_PROTOCOL_NONE 0x00
#define CDC_COMM_PROTOCOL_ATCOMMAND 0x01
#define CDC_COMM_PROTOCOL_ATCOMMAND_PCCA_101 0x02
#define CDC_COMM_PROTOCOL_ATCOMMAND_PCCA_101_AND_ANNEXO0x03
#define CDC_COMM_PROTOCOL_ATCOMMAND_GSM_707 0x04
#define CDC_COMM_PROTOCOL_ATCOMMAND_3GPP_27007 0x05
#define CDC_COMM_PROTOCOL_ATCOMMAND_CDMA 0x06
#define CDC_COMM_PROTOCOL_ETHERNET_EMULATION_MODEL 0x07

#define CDC_FUNC_DESC_HEADER 0x00
#define CDC_FUNC_DESC_CALL_MANAGEMENT 0x01
#define CDC_FUNC_DESC_ABSTRACT_CONTROL_MANAGEMENT 0x02
#define CDC_FUNC_DESC_DIRECT_LINE_MANAGEMENT 0x03
#define CDC_FUNC_DESC_TELEPHONE_RINGER 0x04
#define CDC_FUNC_DESC_TELEPHONE_CALL_AND_LINE_STATE_REPORTING_CAPACITY0x05
#define CDC_FUNC_DESC_UNION 0x06
#define CDC_FUNC_DESC_COUNTRY_SELECTION 0x07
#define CDC_FUNC_DESC_TELEPHONE_OPERATIONAL_MODES 0x08
#define CDC_FUNC_DESC_USB_TERMINAL 0x09
#define CDC_FUNC_DESC_NETWORK_CHANNEL_TERMINAL 0x0A
#define CDC_FUNC_DESC_PROTOCOL_UNIT 0x0B
#define CDC_FUNC_DESC_EXTENSION_UNIT 0x0C
#define CDC_FUNC_DESC_MULTICHANEL_MANAGEMENT 0x0D
#define CDC_FUNC_DESC_CAPI_CONTROL_MANAGEMENT 0x0E
#define CDC_FUNC_DESC_ETHERNET_NETWORKING 0x0F
#define CDC_FUNC_DESC_ATM_NETWORKING 0x10
#define CDC_FUNC_DESC_WIRELESS_HANDSET_CONTROL_MODEL 0x11
#define CDC_FUNC_DESC_MOBILE_DIRECT_LINE_MODEL 0x12
#define CDC_FUNC_DESC_MOBILE_DIRECT_LINE_MODEL_DETAIL 0x13
#define CDC_FUNC_DESC_DEVICE_MANAGEMENT_MODEL 0x14
#define CDC_FUNC_DESC_OBEX 0x15
#define CDC_FUNC_DESC_COMMAND_SET 0x16
#define CDC_FUNC_DESC_COMMAND_SET_DETAIL 0x17
#define CDC_FUNC_DESC_TELEPHONE_CONTROL_MODEL 0x18
#define CDC_FUNC_DESC_OBEX_SERVICE_IDENTIFIER 0x19
#define CDC_FUNC_DESC_NCM 0x1A

#define SET_LINE_CODING 0x20
#define GET_LINE_CODING 0x21
#define SET_CONTROL_LINE_STATE 0x22

typedef struct __packed {
    uint8_t bmRequestType;
    uint8_t bRequest;
    union {
        uint8_t wBytes[6];
        struct __packed {
            uint16_t wValue;
            uint16_t wIndex;
            uint16_t wLength;
        };

        // Core setup packet types
        struct __packed {
            uint8_t bIndex;
            uint8_t bType;
            uint16_t wLanguageId;
            uint16_t wDescriptorLength;
        } getDescriptor;
        struct __packed {
            uint16_t wFeature;
            uint16_t wEp;
        } clearFeature;
        struct __packed {
            uint16_t wAddress;
        } setAddress;
        struct __packed {
            uint16_t wConfiguration;
        } setConfiguration;
        struct __packed {
            uint16_t wFeature;
            uint16_t wEp;
        } setFeature;
        struct __packed {
            uint16_t wAlternate;
            uint16_t wInterface;
        } setInterface;
        struct __packed {
            uint16_t wValue;
            uint16_t wInterface;
        } getStatus;

        // USB HID
        struct __packed {
            uint8_t bIndex;
            uint8_t bType;
            uint16_t wInterfaceNum;
            uint16_t wDescriptorLength;
        } hidGetDescriptor;
        struct __packed {
            uint8_t bReportId;
            uint8_t bReportType;
            uint16_t wInterface;
            uint16_t wLength;
        } hidGetReport;
        struct __packed {
            uint8_t bReportId;
            uint8_t bReportType;
            uint16_t wInterface;
            uint16_t wLength;
        } hidSetReport;
        struct __packed {
            uint8_t bReportId;
            uint8_t bPad;
            uint16_t wInterface;
            uint16_t wLength;
        } hidGetIdle;
        struct __packed {
            uint8_t bReportId;
            uint8_t bDuration;
            uint16_t wInterface;
            uint16_t wLength;
        } hidSetIdle;
        struct __packed {
            uint16_t wPad;
            uint16_t wInterface;
            uint16_t wLength;
        } hidGetProtocol;
        struct __packed {
            uint16_t wProtocol;
            uint16_t wInterface;
            uint16_t wLength;
        } hidSetProtocol;

        // USB CDC
        struct __packed {
            uint16_t wValue;
            uint16_t wInterface;
            uint16_t wLength;
        } getLineCoding;
        struct __packed {
            uint16_t wValue;
            uint16_t wInterface;
            uint16_t wLength;
        } setLineCoding;
    };
} usb_setup_t;

typedef struct __packed {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdUSB;
    uint8_t bDeviceClass;
    uint8_t bDeviceSubClass;
    uint8_t bDeviceProtocol;
    uint8_t bMaxPacketSize0;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t iManufacture;
    uint8_t iProduct;
    uint8_t iSerialNumber;
    uint8_t bNumConfigurations;
} usb_device_descr_t;
typedef struct __packed {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bFirstInterface;
    uint8_t bInterfaceCount;
    uint8_t bFunctionClass;
    uint8_t bFunctionSubClass;
    uint8_t bFunctionProtocol;
    uint8_t iFunction;
} usb_desc_iad_t;
typedef struct __packed {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t wTotalLength;
    uint8_t bNumInterfaces;
    uint8_t bConfigurationValue;
    uint8_t iConfiguration;
    uint8_t bmAttributes;
    uint8_t MaxPower;
} usb_desc_config_t;
typedef struct __packed {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bInterfaceNumber;
    uint8_t bAlternateSetting;
    uint8_t bNumEndpoints;
    uint8_t bInterfaceClass;
    uint8_t bInterfaceSubClass;
    uint8_t bInterfaceProtocol;
    uint8_t iInterface;
} usb_desc_interface_t;
typedef struct __packed {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdHID;
    uint8_t bCountryCode;
    uint8_t bNumDescriptors;
    uint8_t bReportDescriptorType;
    uint16_t wDescriptorLength;
} usb_desc_hid_t;
typedef struct __packed {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bEndpointAddress;
    uint8_t bmAttributes;
    uint16_t wMaxPacketSize;
    uint8_t bInterval;
} usb_desc_endpoint_t;

typedef struct __packed {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint16_t bcdCDC;
} usb_desc_cdc_header_t;
typedef struct __packed {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bControlInterface;
    uint8_t bSubordinateInterface0;
} usb_desc_cdc_union_t;
typedef struct __packed {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bmCapabilities;
    uint8_t bDataInterface;
} usb_desc_cdc_call_t;
typedef struct __packed {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint8_t bDescriptorSubtype;
    uint8_t bmCapabilities;
} usb_desc_cdc_acm_t;

// HID definitions
#define GET_REPORT 0x01
#define GET_IDLE 0x02
#define GET_PROTOCOL 0x03
#define SET_REPORT 0x09
#define SET_IDLE 0x0A
#define SET_PROTOCOL 0x0B

#define HID_NONE 0x00
#define HID_KEYBOARD 0x01
#define HID_MOUSE 0x02

#define HID_RPT_TYPE_INPUT 0x01
#define HID_RPT_TYPE_OUTPUT 0x02
#define HID_RPT_TYPE_FEATURE 0x03
