#include "test_config.h"
#include "test_gpio.h"
#include "test_timer.h"
#include "test_console.h"
#include "test_can.h"

#include "cmsis_os2.h"

#include "wheel_controller.h"

#include "cmsis_os2.h"
#include "bsp_can.h"
#include "bsp_encoder.h"
#include "bsp_hbridge.h"
#include "bsp_servo.h"
#include "bsp_log.h"

int main(void) {
    __enable_irq();
    #if (GPIO_TEST == ENABLE)
    test_gpio_run();
    #endif

    #if (TIMER_TEST == ENABLE)
    test_timer_run();
    #endif

    #if (CONSOLE_TEST == ENABLE)
    test_console_run();
    #endif

    #if (CAN_TEST == ENABLE)
    test_can_run();
    #endif


    return 0;
}
