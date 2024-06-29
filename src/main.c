#include "tasoller.h"

uint8_t volatile gu8HIDIO4Ready = 0;
uint8_t volatile gu8HIDMiscReady = 0;

#define DEBOUNCE_COUNT 8
uint8_t TickDebouncer(uint8_t u8Buttons) {
    static uint8_t u8Counters[8] = { 0 };
    uint8_t debounced = 0;
    for (uint8_t i = 0; i < 8; i++) {
        if (u8Buttons & (1 << i))
            u8Counters[i] = 0;
        else if (u8Counters[i] < DEBOUNCE_COUNT)
            u8Counters[i]++;

        if (u8Counters[i] < DEBOUNCE_COUNT) debounced |= (1 << i);
    }
    return debounced;
}

uint8_t gu8DigitalButtons;
#define DELAY_FN 2
#define DELAY_AIR 2
void Digital_TickInputs() {
    uint8_t u8Buttons = 0;

    GPIO_SetMode(_PIN_FN1, GPIO_PMD_INPUT);
    DelayCycles_Small(DELAY_FN);
    GPIO_SetMode(_PIN_FN1, GPIO_PMD_QUASI);
    u8Buttons |= PIN_FN1 ? 0 : DIGITAL_FN1_Msk;

    GPIO_SetMode(_PIN_FN2, GPIO_PMD_INPUT);
    DelayCycles_Small(DELAY_FN);
    GPIO_SetMode(_PIN_FN2, GPIO_PMD_QUASI);
    u8Buttons |= PIN_FN2 ? 0 : DIGITAL_FN2_Msk;

    GPIO_SetMode(_PIN_AIR1, GPIO_PMD_INPUT);
    DelayCycles_Small(DELAY_AIR);
    GPIO_SetMode(_PIN_AIR1, GPIO_PMD_QUASI);
    u8Buttons |= PIN_AIR1 ? DIGITAL_AIR1_Msk : 0;

    GPIO_SetMode(_PIN_AIR2, GPIO_PMD_INPUT);
    DelayCycles_Small(DELAY_AIR);
    GPIO_SetMode(_PIN_AIR2, GPIO_PMD_QUASI);
    u8Buttons |= PIN_AIR2 ? DIGITAL_AIR2_Msk : 0;

    GPIO_SetMode(_PIN_AIR3, GPIO_PMD_INPUT);
    DelayCycles_Small(DELAY_AIR);
    GPIO_SetMode(_PIN_AIR3, GPIO_PMD_QUASI);
    u8Buttons |= PIN_AIR3 ? DIGITAL_AIR3_Msk : 0;

    GPIO_SetMode(_PIN_AIR4, GPIO_PMD_INPUT);
    DelayCycles_Small(DELAY_AIR);
    GPIO_SetMode(_PIN_AIR4, GPIO_PMD_QUASI);
    u8Buttons |= PIN_AIR4 ? DIGITAL_AIR4_Msk : 0;

    GPIO_SetMode(_PIN_AIR5, GPIO_PMD_INPUT);
    DelayCycles_Small(DELAY_AIR);
    GPIO_SetMode(_PIN_AIR5, GPIO_PMD_QUASI);
    u8Buttons |= PIN_AIR5 ? DIGITAL_AIR5_Msk : 0;

    GPIO_SetMode(_PIN_AIR6, GPIO_PMD_INPUT);
    DelayCycles_Small(DELAY_AIR);
    GPIO_SetMode(_PIN_AIR6, GPIO_PMD_QUASI);
    u8Buttons |= PIN_AIR6 ? DIGITAL_AIR6_Msk : 0;

    gu8DigitalButtons = TickDebouncer(u8Buttons);
}

int _entry(void) {
    SYS_UnlockReg();
    SYS_Init();
#ifdef ENABLE_BOOTLOADER_CHECK
    SYS_Bootloader_Check();
#endif
    SYS_ModuleInit();

    FMC_EEPROM_Load();

    gu8VcomReady = 1;
    gu8HIDIO4Ready = 1;
    gu8HIDMiscReady = 1;

    /**
     * Unfortunately, everything related to this seems to be a bit broken at the moment
     * For some reason, this code gets into infinite loops on cold boots, even with 5 seconds of
     * delay added first.
     * To make matters worse, if we ever actually call PSoCSetFingerCapacitance it ends up with
     * huge asymmetry between the two PSoCs, which is just unworkable.
     */
    // uint16_t u16InitialFingerCap = PSoCGetFingerCapacitance();
    // if (u16InitialFingerCap != gu16PSoCFingerCap) {
    //     PSoCSetFingerCapacitance(gu16PSoCFingerCap);
    // }

    static uint32_t su32NowMs = 0;
    su32NowMs++;
    while (1) {
        USB_VCOM_Tick();

        if (bPSoCDirty) {
            bPSoCDirty = 0;
            PSoC_PostProcessing();
        }

        Slider_TickSerial();
        USBD_HID_PrepareReport();

        // Limit this to 250us so our debouncer has a chance to function
        // 250us is the resolution of our timer currently, though we could increase it if needed
        if (gu8Do250usTick) {
            Digital_TickInputs();
        }

        if (gu8Do1msTick) {
            su32NowMs++;
            gu8Do1msTick = 0;

            Slider_Tick1ms();

            PSoC_DigitalCalc();
            UI_Tick();

            // LED_WriteBasicGrounds();
            LED_WriteRGB();
        } /* else {
             DelayCycles(45);
         }*/

        // Wait until we know both PSoC are awake and talking before we attempt configure them
        if (gu8PSoCSeenData == 0b011) {
            PSoC_SetFingerCapacitanceFromConfig();
            gu8PSoCSeenData |= 0b100;
        }

        FMC_EEPROM_Store();
    }
}
