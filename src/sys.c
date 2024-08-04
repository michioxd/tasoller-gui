#include "tasoller.h"

#ifdef stdout
// Unnecessary for compilation, but clangd can get a bit confused
#undef stdout
#endif
// For picolibc
FILE *const stdout = NULL;

void SYS_ResetModule(uint32_t u32ModuleIndex) {
    // Generate reset signal to the corresponding module
    *(volatile uint32_t *)((uint32_t)&SYS->IPRSTC1 + (u32ModuleIndex >> 24)) |=
        1 << (u32ModuleIndex & 0x00ffffff);

    // Release corresponding module from reset state
    *(volatile uint32_t *)((uint32_t)&SYS->IPRSTC1 + (u32ModuleIndex >> 24)) &=
        ~(1 << (u32ModuleIndex & 0x00ffffff));
}

// Even though we have an external 12MHz crystal, it's never actually used.
// This define toggles if we will even bother configuring it or not.
#define EXT_CLK

void SYS_Init(void) {
#ifdef EXT_CLK
    // Enable XT1_OUT (PF.0) and XT1_IN (PF.1)
    SYS->GPF_MFP &= ~(SYS_GPF_MFP_PF0_Msk | SYS_GPF_MFP_PF1_Msk);
    SYS->GPF_MFP |= SYS_GPF_MFP_PF0_XT1_OUT | SYS_GPF_MFP_PF1_XT1_IN;
#endif

    // Enable Internal RC 22.1184 MHz clock
    CLK_EnableXtalRC(CLK_PWRCON_OSC22M_EN_Msk);
    CLK_WaitClockReady(CLK_CLKSTATUS_OSC22M_STB_Msk);

    // Switch HCLK clock source to Internal RC and HCLK source divide 1
    CLK_SetHCLK(CLK_CLKSEL0_HCLK_S_HIRC, CLK_CLKDIV_HCLK(1));

#ifdef EXT_CLK
    // Enable external XTAL 12 MHz clock
    CLK_EnableXtalRC(CLK_PWRCON_XTL12M_EN_Msk);
    CLK_WaitClockReady(CLK_CLKSTATUS_XTL12M_STB_Msk);
#endif

    // Set core clock
    CLK_SetCoreClock(72 MHz);

    // Enable module clocks
    CLK_EnableModuleClock(UART1_MODULE);
    CLK_EnableModuleClock(USBD_MODULE);
    CLK_EnableModuleClock(TMR0_MODULE);
    CLK_EnableModuleClock(I2C1_MODULE);
    // Select module clock sources
    CLK_SetModuleClock(UART1_MODULE, CLK_CLKSEL1_UART_S_HXT, CLK_CLKDIV_UART(1));
    CLK_SetModuleClock(USBD_MODULE, 0, CLK_CLKDIV_USB(3));
    CLK_SetModuleClock(TMR0_MODULE, 0, 0);
    CLK_SetModuleClock(I2C1_MODULE, 0, 0);

    // Set GPB multi-function pins for UART1 RXD(PB4) and TXD(PB5)
    SYS->GPB_MFP &= ~(SYS_GPB_MFP_PB4_Msk | SYS_GPB_MFP_PB5_Msk);
    SYS->GPB_MFP |= SYS_GPB_MFP_PB4_UART1_RXD | SYS_GPB_MFP_PB5_UART1_TXD;

    GPIO_SetMode(_PIN_SDA, GPIO_PMD_OUTPUT);
    GPIO_SetMode(_PIN_SCL, GPIO_PMD_OUTPUT);
    PIN_SDA = 1;
    PIN_SCL = 1;

    // Configure our GPIO pins
    GPIO_SetMode(_PIN_FN1, GPIO_PMD_QUASI);
    GPIO_SetMode(_PIN_FN2, GPIO_PMD_QUASI);
    GPIO_SetMode(_PIN_EC1, GPIO_PMD_INPUT);
    GPIO_SetMode(_PIN_RX2, GPIO_PMD_INPUT);
    GPIO_SetMode(_PIN_RX3, GPIO_PMD_INPUT);
    GPIO_SetMode(_PIN_RX4, GPIO_PMD_INPUT);
    GPIO_SetMode(_PIN_EC2, GPIO_PMD_INPUT);
    GPIO_SetMode(_PIN_EC3, GPIO_PMD_INPUT);
    GPIO_SetMode(_PIN_USB_MUX_SEL, GPIO_PMD_OUTPUT);
    GPIO_SetMode(_PIN_USB_MUX_EN, GPIO_PMD_OUTPUT);
    GPIO_SetMode(_PIN_LED_WING_PWR, GPIO_PMD_OUTPUT);
    GPIO_SetMode(_PIN_LED_GROUND_PWR, GPIO_PMD_OUTPUT);

    PIN_LED_WING_PWR = 1;
    PIN_LED_GROUND_PWR = 1;

    // If FN2 is depressed, trigger the LED bootloader to enter bootloading mode by pulling both
    // PA10 and PA11 low rather than configuring them for I2C (pulling high).
    //
    // We have 145us to get to this point, because that's how long the LED bootloader is willing to
    // wait!
    if (PIN_FN2 == 0) {
        PIN_USB_MUX_SEL = USB_MUX_LEDS;
        PIN_USB_MUX_EN = USB_MUX_ENABLE;

        // Trigger the bootloader
        GPIO_SetMode(_PIN_SDA, GPIO_PMD_OUTPUT);
        GPIO_SetMode(_PIN_SCL, GPIO_PMD_OUTPUT);
        PIN_SDA = 0;
        PIN_SCL = 0;
    } else {
        // Set GPA multi-function pins for I2C1 SDA and SCL
        SYS->GPA_MFP &= ~(SYS_GPA_MFP_PA10_Msk | SYS_GPA_MFP_PA11_Msk);
        SYS->GPA_MFP |= (SYS_GPA_MFP_PA10_I2C1_SDA | SYS_GPA_MFP_PA11_I2C1_SCL);
        SYS->ALT_MFP &= ~(SYS_ALT_MFP_PA10_Msk | SYS_ALT_MFP_PA11_Msk);
        SYS->ALT_MFP |= (SYS_ALT_MFP_PA10_I2C1_SDA | SYS_ALT_MFP_PA11_I2C1_SCL);

        PIN_USB_MUX_SEL = USB_MUX_HOST;
        PIN_USB_MUX_EN = USB_MUX_ENABLE;
    }
}

void SYS_ModuleInit(void) {
    // Setup UART1
    SYS_ResetModule(UART1_RST);
    UART_Open(UART1, 460800);
    /* Enable interrupts for:
     * - Receive data available
     * - Receive line status
     * - RX time-out
     */
    UART1->IER = (UART_IER_RDA_IEN_Msk | UART_IER_RLS_IEN_Msk | UART_IER_RTO_IEN_Msk);
    NVIC_EnableIRQ(UART1_IRQn);

    // Set NVIC priorities
    NVIC_SetPriority(USBD_IRQn, 1);
    NVIC_SetPriority(TMR0_IRQn, 2);
    NVIC_SetPriority(I2S_IRQn, 3);
    NVIC_SetPriority(I2C0_IRQn, 4);

    CLK_SysTickDelay(10 ms);

    // For communication with the LED MCU
    LED_I2C1_Init();

    // Timer 0: Used for HID, PSoC processing and slider outbound
    // This timer runs at 4kHz, but only sets the 1ms flag every 4 calls
    TIMER_Open(TIMER0, TIMER_PERIODIC_MODE, 4 kHz);
    TIMER_EnableInt(TIMER0);
    NVIC_EnableIRQ(TMR0_IRQn);
    TIMER_Start(TIMER0);

    // Setup our USB stack
    Tas_USBD_Open();
    Tas_USBD_Init();
    Tas_USBD_Start();
    NVIC_EnableIRQ(USBD_IRQn);
}

void HardFault_Handler(uint32_t *stack) {
    // Extract r0-3, r12, lr, pc, psr, and place them into "stack"
    asm("MOVS    r0, #4                        \n"
        "MOV     r1, lr                        \n"
        "TST     r0, r1                        \n"  // check LR bit 2
        "BEQ     1f                            \n"  // stack use MSP
        "MRS     r0, psp                       \n"  // stack use PSP, read PSP
        "MOV     r1, lr                        \n"  // LR current value
        "B       2f                            \n"
        "1:                                    \n"
        "MRS     r0, msp                       \n"  // LR current value
        "2:                                    \n");

    // Extract out into locals for easy debugging
    uint32_t r0 = stack[0];
    uint32_t r1 = stack[1];
    uint32_t r2 = stack[2];
    uint32_t r3 = stack[3];
    uint32_t r12 = stack[4];
    uint32_t lr = stack[5];
    uint32_t pc = stack[6];
    uint32_t psr = stack[7];

    // Silence unused warnings
    (void)r0;
    (void)r1;
    (void)r2;
    (void)r3;
    (void)r12;
    (void)lr;
    (void)pc;
    (void)psr;

    // Force a chip reset
    SYS->IPRSTC1 = SYS_IPRSTC1_CHIP_RST_Msk;
    while (1)
        ;
}

void SYS_Bootloader_Check(void) {
    FMC_Open();
    while (FMC_Read(BOOTLOADER_MAGIC_ADDR) != BOOTLOADER_MAGIC)
        ;
    FMC_Close();
}

volatile uint8_t gu8Do250usTick = 0;
volatile uint8_t gu8Do1msTick = 0;
void TMR0_IRQHandler(void) {
    static uint32_t su32Counter;
    if (TIMER_GetIntFlag(TIMER0)) {
        TIMER_ClearIntFlag(TIMER0);
        gu8Do250usTick = 1;
        if (++su32Counter == 4) {
            su32Counter = 0;
            gu8Do1msTick = 1;
        }
    }
}

#define V6M_AIRCR_VECTKEY_DATA 0x05FA0000UL
void __attribute__((noreturn)) SYS_EnterLDROM(void) {
    SYS_UnlockReg();

    // If we use a CPU reset, I2C is left setup and so the LED board will be timing out rather than
    // early-NACKS.
    // If we use a CHIP reset we aren't guaranteed to land in LDROM because it's based on the CONFIG
    // flags, though realistically we are.
    // An MCU reset guarantees we end up in LDROM regardless of configuration.

    // Reset to LDROM using a CPU reset
    // FMC->ISPCON |= FMC_ISPCON_BS_Msk | FMC_ISPCON_ISPEN_Msk;
    // SYS->IPRSTC1 |= SYS_IPRSTC1_CPU_RST_Msk;

    // Reset to LDROM using a CHIP reset
    // SYS->IPRSTC1 |= SYS_IPRSTC1_CHIP_RST_Msk;

    // Reset to LDROM using a MCU reset
    SYS->RSTSRC = SYS_RSTSRC_RSTS_POR_Msk | SYS_RSTSRC_RSTS_RESET_Msk;
    FMC->ISPCON |= FMC_ISPCON_BS_Msk | FMC_ISPCON_ISPEN_Msk;
    NVIC_SystemReset();

    // Trap the processor
    while (1)
        ;
    __builtin_unreachable();
}

void SYS_WaitBootloaderLED(void) {
    SYS_UnlockReg();

    // Switch PA10 and PA11 to GPIO so we can pull them low
    SYS->GPA_MFP &= ~(SYS_GPA_MFP_PA10_Msk | SYS_GPA_MFP_PA11_Msk);
    SYS->GPA_MFP |= (SYS_GPA_MFP_PA10_GPIO | SYS_GPA_MFP_PA11_GPIO);
    SYS->ALT_MFP &= ~(SYS_ALT_MFP_PA10_Msk | SYS_ALT_MFP_PA11_Msk);
    SYS->ALT_MFP |= (SYS_ALT_MFP_PA10_GPIO | SYS_ALT_MFP_PA11_GPIO);

    // Trigger the bootloader
    GPIO_SetMode(_PIN_SDA, GPIO_PMD_OUTPUT);
    GPIO_SetMode(_PIN_SCL, GPIO_PMD_OUTPUT);
    PIN_SDA = 0;
    PIN_SCL = 0;

    // Shutdown I2C
    I2C_Close(I2C1);

    // Turn off our USB PHY
    USBD->ATTR = 0x650;
    NVIC_DisableIRQ(USBD_IRQn);
    SYS_ResetModule(USBD_RST);
    // Give Windows a moment to notice the disconnection
    CLK_SysTickLongDelay(1000 ms);
    // Switch the USB connection over the LED microcontroller
    PIN_USB_MUX_SEL = USB_MUX_LEDS;
    PIN_USB_MUX_EN = USB_MUX_ENABLE;

    u16I2CRxIndex = 0;
    // Wait 5 seconds, which is long enough for the LED bootloader to take over
    CLK_SysTickLongDelay(5000 ms);

    // Switch PA10 and PA11 back to I2C
    SYS->GPA_MFP &= ~(SYS_GPA_MFP_PA10_Msk | SYS_GPA_MFP_PA11_Msk);
    SYS->GPA_MFP |= (SYS_GPA_MFP_PA10_I2C1_SDA | SYS_GPA_MFP_PA11_I2C1_SCL);
    SYS->ALT_MFP &= ~(SYS_ALT_MFP_PA10_Msk | SYS_ALT_MFP_PA11_Msk);
    SYS->ALT_MFP |= (SYS_ALT_MFP_PA10_I2C1_SDA | SYS_ALT_MFP_PA11_I2C1_SCL);

    // Restart the I2C controller
    LED_I2C1_Init();

    // Wait until we start getting data from the LED firmware
    while (u16I2CRxIndex == 0)
        ;

    // Take back control of USB
    PIN_USB_MUX_SEL = USB_MUX_HOST;
    PIN_USB_MUX_EN = USB_MUX_ENABLE;

    // Bring our USB PHY back online, along with a delay sufficient for a device reconnect to be
    // detected
    Tas_USBD_Open();
    Tas_USBD_Init();
    Tas_USBD_Start();
    NVIC_EnableIRQ(USBD_IRQn);
}