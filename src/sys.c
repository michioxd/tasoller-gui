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

void SYS_Init(void) {
    // Enable XT1_OUT (PF.0) and XT1_IN (PF.1)
    SYS->GPF_MFP &= ~(SYS_GPF_MFP_PF0_Msk | SYS_GPF_MFP_PF1_Msk);
    SYS->GPF_MFP |= SYS_GPF_MFP_PF0_XT1_OUT | SYS_GPF_MFP_PF1_XT1_IN;

    // Enable Internal RC 22.1184 MHz clock
    CLK_EnableXtalRC(CLK_PWRCON_OSC22M_EN_Msk);
    CLK_WaitClockReady(CLK_CLKSTATUS_OSC22M_STB_Msk);

    // Switch HCLK clock source to Internal RC and HCLK source divide 1
    CLK_SetHCLK(CLK_CLKSEL0_HCLK_S_HIRC, CLK_CLKDIV_HCLK(1));

    // Enable external XTAL 12 MHz clock
    CLK_EnableXtalRC(CLK_PWRCON_XTL12M_EN_Msk);
    CLK_WaitClockReady(CLK_CLKSTATUS_XTL12M_STB_Msk);

    // Set core clock
    CLK_SetCoreClock(72000000);

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

    GPIO_SetMode(_PIN_FN2, GPIO_PMD_INPUT);
    // TODO:
    if (PIN_FN2 == 0) {
        PIN_USB_MUX_SEL = USB_MUX_LEDS;
        PIN_USB_MUX_EN = USB_MUX_ENABLE;
        GPIO_SetMode(_PIN_SDA, GPIO_PMD_OUTPUT);
        GPIO_SetMode(_PIN_SCL, GPIO_PMD_OUTPUT);
        PIN_SDA = 0;
        PIN_SCL = 0;
        return;
    }

    // Set GPA multi-function pins for I2C1 SDA and SCL
    SYS->GPA_MFP &= ~(SYS_GPA_MFP_PA10_Msk | SYS_GPA_MFP_PA11_Msk);
    SYS->GPA_MFP |= (SYS_GPA_MFP_PA10_I2C1_SDA | SYS_GPA_MFP_PA11_I2C1_SCL);
    SYS->ALT_MFP &= ~(SYS_ALT_MFP_PA10_Msk | SYS_ALT_MFP_PA11_Msk);
    SYS->ALT_MFP |= (SYS_ALT_MFP_PA10_I2C1_SDA | SYS_ALT_MFP_PA11_I2C1_SCL);

    PIN_USB_MUX_EN = USB_MUX_ENABLE;
    PIN_USB_MUX_SEL = USB_MUX_HOST;
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

// Define a dedicated symbol for this, so we can easily trap it with a debugger!
void HardFault_Handler() {
    while (1)
        ;
}

void SYS_Bootloader_Check() {
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
