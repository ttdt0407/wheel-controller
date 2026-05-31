#include "test_config.h"
#include "test_gpio.h"
#include "test_log.h"
#include "test_can.h"
#include "test_servo.h"
#include "test_motor.h"

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

    /* Init servo, hbrige, encoder, can, log before testing each module */
    BSP_Init();

    #if (GPIO_TEST == ENABLE)
    test_gpio_run();
    #endif

    #if (TIMER_TEST == ENABLE)
    test_timer_run();
    #endif

    #if (LOG_TEST == ENABLE)
    test_log_run();
    #endif

    #if (CAN_TEST == ENABLE)
    test_can_run();
    #endif

    #if (SERVO_TEST == ENABLE)
    test_servo_run();
    #endif

    #if (MOTOR_TEST == ENABLE)
    test_motor_run();
    #endif

    return 0;
}
