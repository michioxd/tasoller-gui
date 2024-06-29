#include "tasoller.h"

uint8_t volatile g_u8Suspend = 0;
uint8_t g_u8Idle = 0;
uint8_t g_u8Protocol = 0;

uint8_t gHidSetReport[64];

STR_VCOM_LINE_CODING gLineCoding = { 0, 0, 0, 0 };
uint16_t gCtrlSignal;

uint32_t volatile g_u32OutToggle = 0;

void USBD_IRQHandler(void) {
    uint32_t u32IntSts = USBD_GET_INT_FLAG();
    uint32_t u32State = USBD_GET_BUS_STATE();

    if (u32IntSts & USBD_INTSTS_FLDET) {
        // Floating detect
        USBD_CLR_INT_FLAG(USBD_INTSTS_FLDET);

        if (USBD_IS_ATTACHED()) {
            USBD_ENABLE_USB();
        } else {
            USBD_DISABLE_USB();
        }
    }

    if (u32IntSts & USBD_INTSTS_WAKEUP) {
        USBD_CLR_INT_FLAG(USBD_INTSTS_WAKEUP);
    }

    if (u32IntSts & USBD_INTSTS_BUS) {
        USBD_CLR_INT_FLAG(USBD_INTSTS_BUS);

        if (u32State & USBD_STATE_USBRST) {
            // Bus reset
            USBD_ENABLE_USB();
            Tas_USBD_SwReset();
            g_u32OutToggle = 0;
            g_u8Suspend = 0;
        }
        if (u32State & USBD_STATE_SUSPEND) {
            // Enter power down to wait USB attached; enable USB but disable PHY
            g_u8Suspend = 1;
            USBD_DISABLE_PHY();
        }
        if (u32State & USBD_STATE_RESUME) {
            // Enable USB and enable PHY
            USBD_ENABLE_USB();
            g_u8Suspend = 0;
        }
    }

    if (u32IntSts & USBD_INTSTS_USB) {
        // USB event
        if (u32IntSts & USBD_INTSTS_SETUP) {  // Setup packet
            USBD_CLR_INT_FLAG(USBD_INTSTS_SETUP);

            // Clear the data IN/OUT ready flag of control end-points
            USBD_STOP_TRANSACTION(EP_CTRL_IN);
            USBD_STOP_TRANSACTION(EP_CTRL_OUT);

            Tas_USBD_ProcessSetupPacket();
        }

        // Control endpoints
        if (u32IntSts & USBD_INTSTS_CTRL_IN) {
            USBD_CLR_INT_FLAG(USBD_INTSTS_CTRL_IN);
            Tas_USBD_CtrlIn();
        }
        if (u32IntSts & USBD_INTSTS_CTRL_OUT) {
            USBD_CLR_INT_FLAG(USBD_INTSTS_CTRL_OUT);
            Tas_USBD_CtrlOut();
        }

        // CDC endpoints
        if (u32IntSts & USBD_INTSTS_CDC_IN) {
            USBD_CLR_INT_FLAG(USBD_INTSTS_CDC_IN);
            EP_CDC_IN_Handler();
        }
        if (u32IntSts & USBD_INTSTS_CDC_OUT) {
            USBD_CLR_INT_FLAG(USBD_INTSTS_CDC_OUT);
            EP_CDC_OUT_Handler();
        }
        if (u32IntSts & USBD_INTSTS_CDC_CMD) {
            USBD_CLR_INT_FLAG(USBD_INTSTS_CDC_CMD);
            // TODO: ACM packets have connect/disconnect?
        }

        // IO4 HID endpoints
        if (u32IntSts & USBD_INTSTS_HID_IO4_IN) {
            USBD_CLR_INT_FLAG(USBD_INTSTS_HID_IO4_IN);
            EP_HID_IO4_IN_Handler();
        }

        // IO4 Misc endpoints
        if (u32IntSts & USBD_INTSTS_HID_MISC_IN) {
            USBD_CLR_INT_FLAG(USBD_INTSTS_HID_MISC_IN);
            EP_HID_MISC_IN_Handler();
        }
        if (u32IntSts & USBD_INTSTS_HID_MISC_OUT) {
            USBD_CLR_INT_FLAG(USBD_INTSTS_HID_MISC_OUT);
            EP_HID_MISC_OUT_Handler();
        }
    }
}

void EP_HID_IO4_IN_Handler(void) { gu8HIDIO4Ready = 1; }
void EP_HID_MISC_IN_Handler(void) { gu8HIDMiscReady = 1; }
void EP_HID_MISC_OUT_Handler(void) {
    // TODO: Handle anything we need to here
}
void EP_CDC_OUT_Handler(void) {
    // Bulk OUT
    if (g_u32OutToggle == (USBD->EPSTS & USBD_EPSTS_EPSTS3_Msk)) {
        USBD_SET_PAYLOAD_LEN(EP_CDC_OUT, USBD_CDC_OUT_MAX_SIZE);
    } else {
        gu32RxSize = USBD_GET_PAYLOAD_LEN(EP_CDC_OUT);
        gpu8RxBuf = (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP_CDC_OUT));

        g_u32OutToggle = USBD->EPSTS & USBD_EPSTS_EPSTS3_Msk;
        // Set a flag to indicate bulk out ready
        gi8BulkOutReady = 1;
    }
}
void EP_CDC_IN_Handler(void) { gu32TxSize = 0; }

void Tas_USBD_ClassRequest(void) {
    usb_setup_t setup;
    Tas_USBD_GetSetupPacket(&setup);

    if (setup.bmRequestType & 0x80) {
        // Device to host
        switch (setup.bRequest) {
            case GET_LINE_CODING:
                if (setup.getLineCoding.wInterface == USBD_ITF_CDC_CMD)
                    USBD_MemCopy((uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP_CTRL_IN)),
                                 (uint8_t *)&gLineCoding, setup.getLineCoding.wLength);

                // Data stage
                USBD_SET_DATA1(EP_CTRL_IN);
                USBD_SET_PAYLOAD_LEN(EP_CTRL_IN, setup.getLineCoding.wLength);
                // Status stage
                Tas_USBD_PrepareCtrlOut(NULL, 0, NULL);
                break;

            case GET_REPORT: {
                uint32_t u32Size = 0;
                uint8_t *pu8Data = NULL;
                pu8Data = USBD_HID_GetReport(setup.hidGetReport.bReportId, &u32Size);

                Tas_USBD_PrepareCtrlIn(pu8Data, u32Size);
                // Status stage
                Tas_USBD_PrepareCtrlOut(NULL, 0, NULL);
                break;
            }

            case GET_IDLE:
                USBD_SET_PAYLOAD_LEN(EP_CTRL_OUT, setup.hidGetIdle.wLength);
                // Data stage
                Tas_USBD_PrepareCtrlIn(&g_u8Idle, setup.hidGetIdle.wLength);
                // Status stage
                Tas_USBD_PrepareCtrlOut(NULL, 0, NULL);
                break;

            case GET_PROTOCOL:
                USBD_SET_PAYLOAD_LEN(EP_CTRL_OUT, setup.hidGetProtocol.wLength);
                // Data stage
                Tas_USBD_PrepareCtrlIn(&g_u8Protocol, setup.hidGetProtocol.wLength);
                // Status stage
                Tas_USBD_PrepareCtrlOut(NULL, 0, NULL);
                break;

            default:
                // Setup error, stall the device
                USBD_SetStall(EP_CTRL_IN);
                USBD_SetStall(EP_CTRL_OUT);
                break;
        }
    } else {
        // Host to device
        switch (setup.bRequest) {
            case SET_CONTROL_LINE_STATE:
                // TODO: Use bit[0] (DTR) to identify connection state
                // Is RTS worth using?
                if (setup.wIndex == USBD_ITF_CDC_CMD) gCtrlSignal = setup.wValue;

                // Status stage
                USBD_SET_DATA1(EP_CTRL_IN);
                USBD_SET_PAYLOAD_LEN(EP_CTRL_IN, 0);
                break;

            case SET_LINE_CODING:
                if (setup.setLineCoding.wInterface == USBD_ITF_CDC_CMD)
                    Tas_USBD_PrepareCtrlOut(&gLineCoding, sizeof gLineCoding, NULL);

                // Status stage
                USBD_SET_DATA1(EP_CTRL_IN);
                USBD_SET_PAYLOAD_LEN(EP_CTRL_IN, 0);
                break;

            case SET_REPORT:
                // Report Type = Output
                if (setup.hidSetReport.bReportType == 2) {
                    Tas_USBD_PrepareCtrlOut(&gHidSetReport, sizeof gHidSetReport,
                                            USBD_HID_SetReport);

                    // USBD_SET_DATA1(EP_CTRL_OUT);
                    // USBD_SET_PAYLOAD_LEN(EP_CTRL_OUT, setup.wLength);

                    // USBD_HID_RecvData(
                    //     setup.hidSetReport.bReportId,
                    //     (uint8_t *)(USBD_BUF_BASE + USBD_GET_EP_BUF_ADDR(EP_CTRL_OUT)),
                    //     setup.hidSetReport.wLength);

                    // // Status stage
                    // Tas_USBD_PrepareCtrlIn(NULL, 0);
                }
                USBD_SET_DATA1(EP_CTRL_IN);
                USBD_SET_PAYLOAD_LEN(EP_CTRL_IN, 0);
                break;

            case SET_IDLE:
                g_u8Idle = setup.hidSetIdle.bDuration;
                // Status stage
                USBD_SET_DATA1(EP_CTRL_IN);
                USBD_SET_PAYLOAD_LEN(EP_CTRL_IN, 0);
                break;

            case SET_PROTOCOL:
                g_u8Protocol = setup.hidSetProtocol.wProtocol;
                // Status stage
                USBD_SET_DATA1(EP_CTRL_IN);
                USBD_SET_PAYLOAD_LEN(EP_CTRL_IN, 0);
                break;

            default:
                // Setup error, stall the device
                USBD_SetStall(EP_CTRL_IN);
                USBD_SetStall(EP_CTRL_OUT);
                break;
        }
    }
}
