#include "tasoller.h"

usb_setup_t g_usbd_SetupPacket;
volatile uint8_t g_usbd_RemoteWakeupEn = 0;

volatile uint8_t *g_usbd_CtrlInPointer = 0;
volatile uint32_t g_usbd_CtrlInSize = 0;
static volatile uint8_t *g_usbd_CtrlOutPointer = 0;
static volatile uint32_t g_usbd_CtrlOutSize = 0;
static volatile uint32_t g_usbd_CtrlOutSizeLimit = 0;
static volatile uint32_t g_usbd_UsbAddr = 0;
static volatile uint32_t g_usbd_UsbConfig = 0;
static volatile uint32_t g_usbd_CtrlMaxPktSize = 8;
static volatile uint32_t g_usbd_UsbAltInterface = 0;
static volatile uint32_t g_usbd_CtrlOutToggle = 0;
static volatile uint8_t g_usbd_CtrlInZeroFlag = 0;
static void (*g_usbd_CtrlOutCallback)(volatile uint8_t *, uint32_t);

uint32_t g_u32EpStallLock = 0;

static inline void Tas_USBD_GetDescriptor(void);
static inline void Tas_USBD_StandardRequest(void);

static uint8_t su8VendorCount = 0;

void Tas_USBD_Open(void) {
    g_usbd_CtrlMaxPktSize = gpDeviceDescriptor->bMaxPacketSize0;
    USBD->ATTR = 0x650;  // Disable D+ and USB controller
    CLK_SysTickLongDelay(1000 ms);
    USBD->ATTR = 0x7D0;
    USBD_SET_SE0();
}
void Tas_USBD_Init(void) {
    // Buffer range for setup packet -> [0~7]
    USBD->STBUFSEG = SETUP_BUF_BASE;

    // USB control IN/OUT on EP_CTRL_IN/EP_CTRL_OUT (addr 0)
    USBD_CONFIG_EP(EP_CTRL_IN, USBD_CFG_CSTALL | USBD_CFG_EPMODE_IN | 0);
    USBD_SET_EP_BUF_ADDR(EP_CTRL_IN, EP0_BUF_BASE);
    USBD_CONFIG_EP(EP_CTRL_OUT, USBD_CFG_CSTALL | USBD_CFG_EPMODE_OUT | 0);
    USBD_SET_EP_BUF_ADDR(EP_CTRL_OUT, EP1_BUF_BASE);

    // CDC data IN/OUT on EP2/EP3
    USBD_CONFIG_EP(EP_CDC_IN, USBD_CFG_EPMODE_IN | 1);
    USBD_SET_EP_BUF_ADDR(EP_CDC_IN, EP2_BUF_BASE);
    USBD_CONFIG_EP(EP_CDC_OUT, USBD_CFG_EPMODE_OUT | 2);
    USBD_SET_EP_BUF_ADDR(EP_CDC_OUT, EP3_BUF_BASE);
    USBD_SET_PAYLOAD_LEN(EP_CDC_OUT, USBD_CDC_OUT_MAX_SIZE);

    // CDC command IN on EP4
    USBD_CONFIG_EP(EP_CDC_CMD, USBD_CFG_EPMODE_IN | 3);
    USBD_SET_EP_BUF_ADDR(EP_CDC_CMD, EP4_BUF_BASE);

    // IO4 HID IN on EP5
    USBD_CONFIG_EP(EP_HID_IO4_IN, USBD_CFG_EPMODE_IN | 4);
    USBD_SET_EP_BUF_ADDR(EP_HID_IO4_IN, EP5_BUF_BASE);

    // Misc HID IN/OUT on EP6/EP6
    USBD_CONFIG_EP(EP_HID_MISC_IN, USBD_CFG_EPMODE_IN | 5);
    USBD_SET_EP_BUF_ADDR(EP_HID_MISC_IN, EP6_BUF_BASE);
    USBD_CONFIG_EP(EP_HID_MISC_OUT, USBD_CFG_EPMODE_OUT | 6);
    USBD_SET_EP_BUF_ADDR(EP_HID_MISC_OUT, EP7_BUF_BASE);
    USBD_SET_PAYLOAD_LEN(EP_HID_MISC_OUT, USBD_HID_BUF_LEN);
}
void Tas_USBD_Start(void) {
    // 100ms delay required as part of spec
    CLK_SysTickDelay(100 ms);

    USBD_CLR_SE0();  // Disable software-disconnect function

    // Clear USB-related interrupts before enable interrupt
    USBD_CLR_INT_FLAG(USBD_INT_BUS | USBD_INT_USB | USBD_INT_FLDET | USBD_INT_WAKEUP);
    // Enable USB-related interrupts.
    USBD_ENABLE_INT(USBD_INT_BUS | USBD_INT_USB | USBD_INT_FLDET | USBD_INT_WAKEUP);
}

void Tas_USBD_GetSetupPacket(usb_setup_t *buf) {
    memcpy(buf, &g_usbd_SetupPacket, sizeof g_usbd_SetupPacket);
}

// <static inline> as unused for now
static inline void Tas_USBD_VendorRequest(void) { return; }

void Tas_USBD_ProcessSetupPacket(void) {
    g_usbd_CtrlOutToggle = 0;
    // Get SETUP packet from USB buffer
    USBD_MemCopy((uint8_t *)&g_usbd_SetupPacket, (uint8_t *)USBD_BUF_BASE, 8);

    // Check the request type
    switch (g_usbd_SetupPacket.bmRequestType & 0x60) {
        case REQ_STANDARD:
            Tas_USBD_StandardRequest();
            break;
        case REQ_CLASS:
            Tas_USBD_ClassRequest();
            break;
        case REQ_VENDOR:
            Tas_USBD_VendorRequest();
            break;

        default:
            // Setup error, stall the device
            USBD_SET_EP_STALL(EP_CTRL_IN);
            USBD_SET_EP_STALL(EP_CTRL_OUT);
            break;
    }
}

static inline void Tas_USBD_GetDescriptor(void) {
    uint16_t u16Len;

    g_usbd_CtrlInZeroFlag = 0;
    u16Len = g_usbd_SetupPacket.wLength;

    switch (g_usbd_SetupPacket.getDescriptor.bType) {
        // Get Device Descriptor
        case DESC_DEVICE:
            su8VendorCount = 0;

            u16Len = Minimum(u16Len, LEN_DEVICE);
            Tas_USBD_PrepareCtrlIn((uint8_t *)gpDeviceDescriptor, u16Len);
            Tas_USBD_PrepareCtrlOut(NULL, 0, NULL);
            break;

        // Get Configuration Descriptor
        case DESC_CONFIG: {
            uint32_t u32TotalLen = gpConfigDescriptor->wTotalLength;
            if (u16Len > u32TotalLen) {
                u16Len = u32TotalLen;
                if ((u16Len % g_usbd_CtrlMaxPktSize) == 0) g_usbd_CtrlInZeroFlag = 1;
            }
            Tas_USBD_PrepareCtrlIn((uint8_t *)gpConfigDescriptor, u16Len);
            Tas_USBD_PrepareCtrlOut(NULL, 0, NULL);
            break;
        }
        // Get HID Descriptor
        case DESC_HID:
            /* CV3.0 HID Class Descriptor Test,
               Need to indicate index of the HID Descriptor within gConfigDescriptor, specifically
               HID Composite device. */
            uint32_t u32ConfigDescOffset = 0;  // u32ConfigDescOffset is configuration descriptor
                                               // offset (HID descriptor start index)
            u16Len = Minimum(u16Len, LEN_HID);

            if (g_usbd_SetupPacket.hidGetDescriptor.wInterfaceNum == USBD_ITF_HID_IO4)
                u32ConfigDescOffset = gu32HidDescIO4Offset;
            else if (g_usbd_SetupPacket.hidGetDescriptor.wInterfaceNum == USBD_ITF_HID_MISC)
                u32ConfigDescOffset = gu32HidDescMiscOffset;
            Tas_USBD_PrepareCtrlIn(((uint8_t *)gpConfigDescriptor) + u32ConfigDescOffset, u16Len);
            Tas_USBD_PrepareCtrlOut(NULL, 0, NULL);
            break;

        // Get Report Descriptor
        case DESC_HID_RPT:
            if (g_usbd_SetupPacket.hidGetDescriptor.wInterfaceNum == USBD_ITF_HID_IO4) {
                if (u16Len > gu32UsbHidIO4ReportLen) {
                    u16Len = gu32UsbHidIO4ReportLen;
                    if ((u16Len % g_usbd_CtrlMaxPktSize) == 0) g_usbd_CtrlInZeroFlag = 1;
                }
                Tas_USBD_PrepareCtrlIn((uint8_t *)gpu8UsbHidIO4Report, u16Len);
                Tas_USBD_PrepareCtrlOut(NULL, 0, NULL);
            } else if (g_usbd_SetupPacket.hidGetDescriptor.wInterfaceNum == USBD_ITF_HID_MISC) {
                if (u16Len > gu32UsbHidMiscReportLen) {
                    u16Len = gu32UsbHidMiscReportLen;
                    if ((u16Len % g_usbd_CtrlMaxPktSize) == 0) g_usbd_CtrlInZeroFlag = 1;
                }
                Tas_USBD_PrepareCtrlIn((uint8_t *)gpu8UsbHidMiscReport, u16Len);
                Tas_USBD_PrepareCtrlOut(NULL, 0, NULL);
            } else {
                Tas_USBD_PrepareCtrlIn(NULL, u16Len);
                Tas_USBD_PrepareCtrlOut(NULL, 0, NULL);
            }

            break;

        // Get String Descriptor
        case DESC_STRING: {
            uint8_t iString = g_usbd_SetupPacket.getDescriptor.bIndex;
            static uint8_t u8Str[256];  // IO4 needs at least 2+(2*96)=194

            memset(u8Str, 0, sizeof u8Str);
            u8Str[1] = DESC_STRING;

            switch (iString) {
                case USB_STRING_LANG:
                    u8Str[0] = 2 + (2 * 2);
                    u8Str[2] = 0x09;
                    u8Str[3] = 0x04;
                    break;

                case USB_STRING_VENDOR: {
                    uint8_t u8Len = strlen(gszVendor);
                    u8Str[0] = 2 + u8Len * 2;
                    for (uint8_t i = 0; i < u8Len; i++) {
                        u8Str[2 + i * 2] = gszVendor[i];
                        u8Str[2 + i * 2 + 1] = 0;
                    }
                    break;
                }
                case USB_STRING_PRODUCT: {
                    uint8_t u8Len = strlen(gszProduct);
                    u8Str[0] = 2 + u8Len * 2;
                    for (uint8_t i = 0; i < u8Len; i++) {
                        u8Str[2 + i * 2] = gszProduct[i];
                        u8Str[2 + i * 2 + 1] = 0;
                    }
                    break;
                }
                case USB_STRING_SERIAL: {
                    // The unique ID is technically 3 words, but I'm pretty sure only the last one
                    // really changes. The TRM has no details regarding this.
                    // There's no harm using all three as our serial, so to stay on the safe side
                    // that's what we do.
                    uint32_t u32serial;

                    FMC_Open();
                    u8Str[0] = 2 + (2 * 24);

                    u32serial = FMC_ReadUID(0);
                    for (int i = 0; i < 8; i++)
                        u8Str[2 + i * 2] = HEX_NIBBLE((u32serial >> (i * 4)));
                    u32serial = FMC_ReadUID(1);
                    for (int i = 0; i < 8; i++)
                        u8Str[2 + (8 * 2) + i * 2] = HEX_NIBBLE((u32serial >> (i * 4)));
                    u32serial = FMC_ReadUID(2);
                    for (int i = 0; i < 8; i++)
                        u8Str[2 + (16 * 2) + i * 2] = HEX_NIBBLE((u32serial >> (i * 4)));

                    FMC_Close();
                    break;
                }
                case USB_STRING_CDC: {
                    const char *szCdc = "TASOLLER Slider Serial (COM1)";
                    uint8_t u8Len = strlen(szCdc);
                    u8Str[0] = 2 + u8Len * 2;
                    for (uint8_t i = 0; i < u8Len; i++) {
                        u8Str[2 + i * 2] = szCdc[i];
                        u8Str[2 + i * 2 + 1] = 0;
                    }
                    break;
                }
                case USB_STRING_HID_IO4: {
                    const char *szCdc = IO4_PRODUCT;
                    uint8_t u8Len = strlen(szCdc);
                    u8Str[0] = 2 + u8Len * 2;
                    for (uint8_t i = 0; i < u8Len; i++) {
                        u8Str[2 + i * 2] = szCdc[i];
                        u8Str[2 + i * 2 + 1] = 0;
                    }
                    break;
                }
                case USB_STRING_HID_MISC: {
                    const char *szCdc = "TASOLLER HID";
                    uint8_t u8Len = strlen(szCdc);
                    u8Str[0] = 2 + u8Len * 2;
                    for (uint8_t i = 0; i < u8Len; i++) {
                        u8Str[2 + i * 2] = szCdc[i];
                        u8Str[2 + i * 2 + 1] = 0;
                    }
                    break;
                }

                default:
                    // Not support. Reply STALL.
                    USBD_SET_EP_STALL(EP_CTRL_IN);
                    USBD_SET_EP_STALL(EP_CTRL_OUT);
                    break;
            }

            if (u8Str[0] != 0) {
                if (u16Len > u8Str[0]) u16Len = u8Str[0];
                if ((u16Len % g_usbd_CtrlMaxPktSize) == 0) g_usbd_CtrlInZeroFlag = 1;
                Tas_USBD_PrepareCtrlIn(u8Str, u16Len);
                Tas_USBD_PrepareCtrlOut(NULL, 0, NULL);
            }
            break;
        }

        default:
            // Not support. Reply STALL.
            USBD_SET_EP_STALL(EP_CTRL_IN);
            USBD_SET_EP_STALL(EP_CTRL_OUT);
            break;
    }
}

static inline void Tas_USBD_StandardRequest(void) {
    // Clear global variables for new request
    g_usbd_CtrlInPointer = 0;
    g_usbd_CtrlInSize = 0;

    // Switch on request data transfer direction
    if (g_usbd_SetupPacket.bmRequestType & 0x80) {
        // Device to host
        switch (g_usbd_SetupPacket.bRequest) {
            case GET_CONFIGURATION:
                // Return current configuration setting
                /* Data stage */
                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP_CTRL_IN)) = g_usbd_UsbConfig;
                USBD_SET_DATA1(EP_CTRL_OUT);
                USBD_SET_PAYLOAD_LEN(EP_CTRL_OUT, 0);
                USBD_SET_DATA1(EP_CTRL_IN);
                USBD_SET_PAYLOAD_LEN(EP_CTRL_IN, 1);
                /* Status stage */
                Tas_USBD_PrepareCtrlOut(NULL, 0, NULL);
                break;

            case GET_DESCRIPTOR:
                Tas_USBD_GetDescriptor();
                /* Status stage */
                Tas_USBD_PrepareCtrlOut(NULL, 0, NULL);
                break;

            case GET_INTERFACE:
                // Return current interface setting
                // Data stage
                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP_CTRL_IN)) = g_usbd_UsbAltInterface;
                USBD_SET_DATA1(EP_CTRL_IN);
                USBD_SET_PAYLOAD_LEN(EP_CTRL_IN, 1);
                // Status stage
                Tas_USBD_PrepareCtrlOut(NULL, 0, NULL);
                break;

            case GET_STATUS:
                // Device
                if (g_usbd_SetupPacket.bmRequestType == 0x80) {
                    uint8_t u8Tmp;

                    u8Tmp = 0;
                    if (gpConfigDescriptor->bmAttributes & 0x40)
                        u8Tmp |= 1;  // Self-Powered/Bus-Powered.
                    if (gpConfigDescriptor->bmAttributes & 0x20)
                        u8Tmp |= (g_usbd_RemoteWakeupEn << 1);  // Remote wake up

                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP_CTRL_IN)) = u8Tmp;
                }
                // Interface
                else if (g_usbd_SetupPacket.bmRequestType == 0x81)
                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP_CTRL_IN)) = 0;
                // Endpoint
                else if (g_usbd_SetupPacket.bmRequestType == 0x82) {
                    uint8_t ep = g_usbd_SetupPacket.getStatus.wInterface & 0xF;
                    M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP_CTRL_IN)) =
                        USBD_GetStall(ep) ? 1 : 0;
                }

                M8(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP_CTRL_IN) + 1) = 0;
                // Data stage
                USBD_SET_DATA1(EP_CTRL_IN);
                USBD_SET_PAYLOAD_LEN(EP_CTRL_IN, 2);
                // Status stage
                Tas_USBD_PrepareCtrlOut(NULL, 0, NULL);
                break;

            default:
                // Setup error, stall the device
                USBD_SET_EP_STALL(EP_CTRL_IN);
                USBD_SET_EP_STALL(EP_CTRL_OUT);
                break;
        }
    } else {
        // Host to device
        switch (g_usbd_SetupPacket.bRequest) {
            case CLEAR_FEATURE:
                if (g_usbd_SetupPacket.clearFeature.wFeature == FEATURE_ENDPOINT_HALT) {
                    int32_t epNum, i;

                    /* EP number stall is not allow to be clear in MSC class "Error Recovery Test".
                       a flag: g_u32EpStallLock is added to support it */
                    epNum = g_usbd_SetupPacket.clearFeature.wEp & 0xF;
                    for (i = 0; i < USBD_MAX_EP; i++) {
                        if (((USBD->EP[i].CFG & 0xF) == epNum) &&
                            ((g_u32EpStallLock & (1 << i)) == 0)) {
                            USBD->EP[i].CFGP &= ~USBD_CFGP_SSTALL_Msk;
                            USBD->EP[i].CFG &= ~USBD_CFG_DSQ_SYNC_Msk;
                        }
                    }
                } else if (g_usbd_SetupPacket.clearFeature.wFeature == FEATURE_DEVICE_REMOTE_WAKEUP)
                    g_usbd_RemoteWakeupEn = 0;
                // Status stage
                USBD_SET_DATA1(EP_CTRL_IN);
                USBD_SET_PAYLOAD_LEN(EP_CTRL_IN, 0);
                break;

            case SET_ADDRESS:
                g_usbd_UsbAddr = g_usbd_SetupPacket.setAddress.wAddress;

                // DATA IN for end of setup
                // Status Stage
                USBD_SET_DATA1(EP_CTRL_IN);
                USBD_SET_PAYLOAD_LEN(EP_CTRL_IN, 0);
                break;

            case SET_CONFIGURATION:
                g_usbd_UsbConfig = g_usbd_SetupPacket.setConfiguration.wConfiguration;

                // Callback would be here if we wanted to have one :)

                // Status stage
                USBD_SET_DATA1(EP_CTRL_IN);
                USBD_SET_PAYLOAD_LEN(EP_CTRL_IN, 0);
                break;

            case SET_FEATURE:
                if (g_usbd_SetupPacket.setFeature.wFeature == FEATURE_ENDPOINT_HALT) {
                    USBD_SetStall(g_usbd_SetupPacket.setFeature.wEp & 0xF);
                } else if (g_usbd_SetupPacket.setFeature.wFeature == FEATURE_DEVICE_REMOTE_WAKEUP) {
                    g_usbd_RemoteWakeupEn = 1;
                }
                // Status stage
                USBD_SET_DATA1(EP_CTRL_IN);
                USBD_SET_PAYLOAD_LEN(EP_CTRL_IN, 0);
                break;

            case SET_INTERFACE:
                g_usbd_UsbAltInterface = g_usbd_SetupPacket.setInterface.wAlternate;
                // Callback would be here if wanted to have one :)
                // Status stage
                USBD_SET_DATA1(EP_CTRL_IN);
                USBD_SET_PAYLOAD_LEN(EP_CTRL_IN, 0);
                break;

            default:
                // Setup error, stall the device
                USBD_SET_EP_STALL(EP_CTRL_IN);
                USBD_SET_EP_STALL(EP_CTRL_OUT);
                break;
        }
    }
}

void Tas_USBD_PrepareCtrlIn(void *pu8Buf, uint32_t u32Size) {
    if (u32Size > g_usbd_CtrlMaxPktSize) {
        // Data size > MXPLD
        g_usbd_CtrlInPointer = (uint8_t *)pu8Buf + g_usbd_CtrlMaxPktSize;
        g_usbd_CtrlInSize = u32Size - g_usbd_CtrlMaxPktSize;
        USBD_SET_DATA1(EP_CTRL_IN);
        USBD_MemCopy((uint8_t *)USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP_CTRL_IN), (uint8_t *)pu8Buf,
                     g_usbd_CtrlMaxPktSize);
        USBD_SET_PAYLOAD_LEN(EP_CTRL_IN, g_usbd_CtrlMaxPktSize);
    } else {
        // Data size <= MXPLD
        g_usbd_CtrlInPointer = 0;
        g_usbd_CtrlInSize = 0;
        USBD_SET_DATA1(EP_CTRL_IN);
        USBD_MemCopy((uint8_t *)USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP_CTRL_IN), (uint8_t *)pu8Buf,
                     u32Size);
        USBD_SET_PAYLOAD_LEN(EP_CTRL_IN, u32Size);
    }
}

/**
 * @brief    Repeat Control IN pipe
 * @details  This function processes the remained data of Control IN transfer.
 */
void Tas_USBD_CtrlIn(void) {
    if (g_usbd_CtrlInSize) {
        // Process remained data
        if (g_usbd_CtrlInSize > g_usbd_CtrlMaxPktSize) {
            // Data size > MXPLD
            USBD_MemCopy((uint8_t *)USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP_CTRL_IN),
                         (uint8_t *)g_usbd_CtrlInPointer, g_usbd_CtrlMaxPktSize);
            USBD_SET_PAYLOAD_LEN(EP_CTRL_IN, g_usbd_CtrlMaxPktSize);
            g_usbd_CtrlInPointer += g_usbd_CtrlMaxPktSize;
            g_usbd_CtrlInSize -= g_usbd_CtrlMaxPktSize;
        } else {
            // Data size <= MXPLD
            USBD_MemCopy((uint8_t *)USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP_CTRL_IN),
                         (uint8_t *)g_usbd_CtrlInPointer, g_usbd_CtrlInSize);
            USBD_SET_PAYLOAD_LEN(EP_CTRL_IN, g_usbd_CtrlInSize);
            g_usbd_CtrlInPointer = 0;
            g_usbd_CtrlInSize = 0;
        }
    } else {  // No more data for IN token
        // In ACK for Set address
        if ((g_usbd_SetupPacket.bmRequestType == REQ_STANDARD) &&
            (g_usbd_SetupPacket.bRequest == SET_ADDRESS)) {
            if ((USBD_GET_ADDR() != g_usbd_UsbAddr) && (USBD_GET_ADDR() == 0)) {
                USBD_SET_ADDR(g_usbd_UsbAddr);
            }
        }

        /* For the case of data size is integral times maximum packet size */
        if (g_usbd_CtrlInZeroFlag) {
            USBD_SET_PAYLOAD_LEN(EP_CTRL_IN, 0);
            g_usbd_CtrlInZeroFlag = 0;
        }
    }
}

void Tas_USBD_PrepareCtrlOut(void *pu8Buf, uint32_t u32Size,
                             void (*pCallback)(volatile uint8_t *, uint32_t)) {
    g_usbd_CtrlOutPointer = pu8Buf;
    g_usbd_CtrlOutSize = 0;
    g_usbd_CtrlOutSizeLimit = u32Size;
    g_usbd_CtrlOutCallback = pCallback;
    USBD_SET_PAYLOAD_LEN(EP_CTRL_OUT, g_usbd_CtrlMaxPktSize);
}

/**
 * @brief    Repeat Control OUT pipe
 * @details  This function processes the successive Control OUT transfer.
 */
void Tas_USBD_CtrlOut(void) {
    uint32_t u32Size;

    if (g_usbd_CtrlOutToggle != (USBD->EPSTS & USBD_EPSTS_EPSTS1_Msk)) {
        g_usbd_CtrlOutToggle = USBD->EPSTS & USBD_EPSTS_EPSTS1_Msk;
        if (g_usbd_CtrlOutSize < g_usbd_CtrlOutSizeLimit) {
            u32Size = USBD_GET_PAYLOAD_LEN(EP_CTRL_OUT);
            USBD_MemCopy((uint8_t *)g_usbd_CtrlOutPointer,
                         (uint8_t *)USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP_CTRL_OUT), u32Size);
            g_usbd_CtrlOutPointer += u32Size;
            g_usbd_CtrlOutSize += u32Size;

            if (g_usbd_CtrlOutSize < g_usbd_CtrlOutSizeLimit)
                USBD_SET_PAYLOAD_LEN(EP_CTRL_OUT, g_usbd_CtrlMaxPktSize);
        }

        if (g_usbd_CtrlOutSize >= g_usbd_CtrlOutSizeLimit && g_usbd_CtrlOutCallback) {
            g_usbd_CtrlOutCallback(g_usbd_CtrlOutPointer - g_usbd_CtrlOutSize, g_usbd_CtrlOutSize);
        }
    } else {
        USBD_SET_PAYLOAD_LEN(EP_CTRL_OUT, g_usbd_CtrlMaxPktSize);
    }
}

void Tas_USBD_SwReset(void) {
    // Reset all variables for protocol
    g_usbd_CtrlInPointer = 0;
    g_usbd_CtrlInSize = 0;
    g_usbd_CtrlOutPointer = 0;
    g_usbd_CtrlOutSize = 0;
    g_usbd_CtrlOutSizeLimit = 0;
    g_u32EpStallLock = 0;
    memset(&g_usbd_SetupPacket, 0, 8);

    // Reset PID DATA0
    for (int i = 0; i < USBD_MAX_EP; i++) USBD->EP[i].CFG &= ~USBD_CFG_DSQ_SYNC_Msk;

    // Reset USB device address
    USBD_SET_ADDR(0);
}
