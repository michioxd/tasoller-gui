#include "tasoller.h"

uint8_t volatile gu8HIDIO4Ready = 0;
uint8_t volatile gu8HIDMiscReady = 0;
uint32_t gu32NowMs = 0;

#define NUM_BUTTONS 8
#define DEBOUNCE_ON (2 * 4)   // We need a digital high for 2ms to latch on
#define DEBOUNCE_OFF (8 * 4)  // Then a digital low for 8ms to latch off
uint8_t TickDebouncer(uint8_t u8Buttons) {
    static uint8_t u8CountUp[NUM_BUTTONS] = { 0 };
    static uint8_t u8CountDown[NUM_BUTTONS] = { 0 };

    uint8_t debounced = 0;
    for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
        if (u8Buttons & (1 << i)) {
            if (u8CountUp[i] < DEBOUNCE_ON)
                u8CountUp[i]++;
            else
                u8CountDown[i] = DEBOUNCE_OFF;
        } else {
            if (u8CountDown[i])
                u8CountDown[i]--;
            else
                u8CountUp[i] = 0;
        }

        if (u8CountDown[i]) debounced |= (1 << i);
    }
    return debounced;
}

/**
 * @brief Perform a sensor read using Quasi-bidirectional Mode
 *
 * See TRM section 6.5.5.4 for details
 */
static inline uint8_t DigitalRead(volatile uint32_t* pData, GPIO_T* pGpio, uint32_t u32Pin) {
    GPIO_SetMode(pGpio, u32Pin, GPIO_PMD_INPUT);
    *pData = 1;
    DELAY_CYCLES_2;
    DELAY_CYCLES_2;
    DELAY_CYCLES_2;
    GPIO_SetMode(pGpio, u32Pin, GPIO_PMD_QUASI);
    DELAY_CYCLES_2;
    DELAY_CYCLES_2;
    DELAY_CYCLES_2;
    return *pData;
}

uint8_t gu8DigitalButtons;
void Digital_TickInputs() {
    uint8_t u8Buttons = 0;

    // The two FN buttons need a quasi read cycle
    u8Buttons |= DigitalRead(&PIN_FN1, _PIN_FN1) ? 0 : DIGITAL_FN2_Msk;
    u8Buttons |= DigitalRead(&PIN_FN2, _PIN_FN1) ? 0 : DIGITAL_FN1_Msk;

    // The 6 AIR sensors drive a ~4.5V signal on the pin, so are just in INPUT mode
    u8Buttons |= DigitalRead(&PIN_AIR1, _PIN_AIR1) ? DIGITAL_AIR1_Msk : 0;
    u8Buttons |= DigitalRead(&PIN_AIR2, _PIN_AIR2) ? DIGITAL_AIR2_Msk : 0;
    u8Buttons |= DigitalRead(&PIN_AIR3, _PIN_AIR3) ? DIGITAL_AIR3_Msk : 0;
    u8Buttons |= DigitalRead(&PIN_AIR4, _PIN_AIR4) ? DIGITAL_AIR4_Msk : 0;
    u8Buttons |= DigitalRead(&PIN_AIR5, _PIN_AIR5) ? DIGITAL_AIR5_Msk : 0;
    u8Buttons |= DigitalRead(&PIN_AIR6, _PIN_AIR6) ? DIGITAL_AIR6_Msk : 0;

    u8Buttons = TickDebouncer(u8Buttons);
    gu8DigitalButtons = u8Buttons;
}

int _entry(void) {
    SYS_UnlockReg();

    // Load our configuration, so we know if the LED processor should be rebooted
    FMC_EEPROM_Load();

    SYS_Init();
#ifdef ENABLE_BOOTLOADER_CHECK
    SYS_Bootloader_Check();
#endif
    SYS_ModuleInit();
    // TODO: Re-lock registers, ideally. Need to check which registers we use where

    gu8VComReady = 1;
    gu8HIDIO4Ready = 1;
    gu8HIDMiscReady = 1;

    /**
     * Unfortunately, everything related to this seems to be a bit broken at the moment
     * For some reason, this code gets into infinite loops on cold boots, even with 5 seconds of
     * delay added first.
     * To make matters worse, if we ever actually call PSoCSetFingerCapacitance it ends up with
     * huge asymmetry between the two PSoCs, which is just unworkable.
     *
     * Instead, see the non-blocking logic in the 1ms tick block.
     */
    // uint16_t u16InitialFingerCap = PSoCGetFingerCapacitance();
    // if (u16InitialFingerCap != gu16PSoCFingerCap) {
    //     PSoCSetFingerCapacitance(gu16PSoCFingerCap, 1);
    // }

    gbLedIsCustom = LED_FMC_Read(0, NULL);

    gu32NowMs = 0;
    uint8_t bPSoCHasTalked = 0;

    while (1) {
        USB_VCOM_Tick();

        if (bPSoCDirtyVolatile) {
            bPSoCDirtyVolatile = 0;
            PSoC_PostProcessing();
        }
        if (bPSoCAliveVolatile && !bPSoCHasTalked) {
            PSoC_SetFingerCapacitanceFromConfig(1);
            bPSoCHasTalked = 1;
        }

        Slider_TickSerial();
        USBD_HID_PrepareReport();

        // Limit this to 250us so our debouncer has a chance to function
        // 250us is the resolution of our timer currently, though we could increase it if needed
        if (gu8Do250usTick) {
            Digital_TickInputs();
        }

        if (gu8Do1msTick) {
            // Update timer
            gu32NowMs++;
            gu8Do1msTick = 0;

            Slider_Tick1ms();
            USB_VCOM_Tick();

            PSoC_DigitalCalc();
            UI_Tick();

            LED_Write();
        }

        FMC_EEPROM_Store();
    }
}
