/**
 * @file bsp_config.c
 * @author dt (tien.ta.eswe@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2026-05-30
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "bsp_config.h"
#include "bsp_hbridge.h"
#include "bsp_encoder.h"
#include "bsp_servo.h"
#include "bsp_log.h"
#include "bsp_can.h"

#include "Driver_GPIO.h"
#include "Driver_RCC.h"

#include <stddef.h>

extern ARM_DRIVER_GPIO Driver_GPIO0;
extern ARM_DRIVER_RCC Driver_RCC0;

void BSP_Init(void) {
    Driver_RCC0.SetSystemClock();
    SystemCoreClockUpdate();

    /* Turn on LED */
    Driver_GPIO0.Setup(LED, NULL);
    Driver_GPIO0.SetDirection(LED, ARM_GPIO_OUTPUT);
    Driver_GPIO0.SetOutputMode(LED, ARM_GPIO_PUSH_PULL);
    Driver_GPIO0.SetOutput(LED, ON);
}