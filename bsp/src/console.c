#include "console.h"
#include "Driver_USART.h"

extern ARM_DRIVER_USART Driver_USART1;

void console_init(void) {

    Driver_USART1.Initialize(NULL);
    Driver_USART1.PowerControl(ARM_POWER_FULL);
    Driver_USART1.Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_DATA_BITS_8 |
                              ARM_USART_PARITY_NONE | ARM_USART_STOP_BITS_1,
                          115200U);
    Driver_USART1.Control(ARM_USART_CONTROL_TX, 1);
    Driver_USART1.Control(ARM_USART_CONTROL_RX, 1);
}

int _write(int file, char* ptr, int len) {

    if ((ptr == 0) || (len <= 0)) {
        return -1;
    }

    if (Driver_USART1.Send(ptr, (uint32_t)len) != ARM_DRIVER_OK) {
        return -1;
    }

    return len;
}

