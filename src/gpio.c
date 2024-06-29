#include "tasoller.h"

void GPIO_SetMode(GPIO_T *port, uint32_t u32PinMask, uint32_t u32Mode) {
    uint32_t i;

    for (i = 0; i < GPIO_PIN_MAX; i++) {
        if (u32PinMask & (1 << i)) {
            port->PMD = (port->PMD & ~(0x3 << (i << 1))) | (u32Mode << (i << 1));
        }
    }
}
