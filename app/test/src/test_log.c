/**
 * @file test_log.c
 * @author dt (tien.ta.eswe@gmail.com)
 * @brief testing console module
 * @version 0.1
 * @date 2026-05-31
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "test_log.h"
#include "bsp_log.h"
#include "test_config.h"

#include "test_gpio.h"

#include "system_stm32f1xx.h"

#include "Driver_GPIO.h"
#include "Driver_RCC.h"

#include <stdio.h>
#include "cmsis_os2.h"

extern ARM_DRIVER_GPIO Driver_GPIO0;
extern ARM_DRIVER_RCC Driver_RCC0;

static void vTestLogTask(void *argument) {
    (void)argument;

    while (1) {
        bsp_log_printf("This is non-blocking print !\r\n");
        osDelay(500);
    }
}

void test_log_run(void) {

    bsp_log_init();
    /* Disbale stdout buffer */
    setvbuf(stdout, NULL, _IONBF, 0);

    osKernelInitialize();

    const osThreadAttr_t task_attr = {
        .name = "TestLog_Task",
        .priority = osPriorityNormal,
        .stack_size = 2048
    };
    osThreadNew(vTestLogTask, NULL, &task_attr);

    osKernelStart();

    while (1) {}
}
