#define PIN_SDA PA10
#define PIN_SCL PA11
#define PIN_USB_MUX_EN PB1
#define PIN_USB_MUX_SEL PB0
#define PIN_FN1 PB7
#define PIN_FN2 PB2
#define PIN_EC1 PB3
#define PIN_EC2 PC5
#define PIN_EC3 PC4
#define PIN_RX2 PD9
#define PIN_RX3 PD10
#define PIN_RX4 PD11
#define PIN_LED_WING_PWR PB13
#define PIN_LED_GROUND_PWR PC3

#define _PIN_SDA PA, BIT10
#define _PIN_SCL PA, BIT11
#define _PIN_USB_MUX_EN PB, BIT1
#define _PIN_USB_MUX_SEL PB, BIT0
#define _PIN_FN1 PB, BIT7
#define _PIN_FN2 PB, BIT2
#define _PIN_EC1 PB, BIT3
#define _PIN_EC2 PC, BIT5
#define _PIN_EC3 PC, BIT4
#define _PIN_RX2 PD, BIT9
#define _PIN_RX3 PD, BIT10
#define _PIN_RX4 PD, BIT11
#define _PIN_LED_WING_PWR PB, BIT13
#define _PIN_LED_GROUND_PWR PC, BIT3

#define PIN_AIR1 PIN_RX2
#define PIN_AIR2 PIN_EC1
#define PIN_AIR3 PIN_RX3
#define PIN_AIR4 PIN_EC2
#define PIN_AIR5 PIN_RX4
#define PIN_AIR6 PIN_EC3
#define _PIN_AIR1 _PIN_RX2
#define _PIN_AIR2 _PIN_EC1
#define _PIN_AIR3 _PIN_RX3
#define _PIN_AIR4 _PIN_EC2
#define _PIN_AIR5 _PIN_RX4
#define _PIN_AIR6 _PIN_EC3

#define USB_MUX_ENABLE 0
#define USB_MUX_DISABLE 1
#define USB_MUX_HOST 0
#define USB_MUX_LEDS 1

#define DIGITAL_FN2_Pos 0
#define DIGITAL_FN1_Pos 1
#define DIGITAL_AIR1_Pos 2
#define DIGITAL_AIR2_Pos 3
#define DIGITAL_AIR3_Pos 4
#define DIGITAL_AIR4_Pos 5
#define DIGITAL_AIR5_Pos 6
#define DIGITAL_AIR6_Pos 7
#define DIGITAL_FN2_Msk (1 << DIGITAL_FN2_Pos)
#define DIGITAL_FN1_Msk (1 << DIGITAL_FN1_Pos)
#define DIGITAL_AIR1_Msk (1 << DIGITAL_AIR1_Pos)
#define DIGITAL_AIR2_Msk (1 << DIGITAL_AIR2_Pos)
#define DIGITAL_AIR3_Msk (1 << DIGITAL_AIR3_Pos)
#define DIGITAL_AIR4_Msk (1 << DIGITAL_AIR4_Pos)
#define DIGITAL_AIR5_Msk (1 << DIGITAL_AIR5_Pos)
#define DIGITAL_AIR6_Msk (1 << DIGITAL_AIR6_Pos)
