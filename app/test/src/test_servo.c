/**
 * @file test_servo.c
 * @author dt (tien.ta.eswe@gmail.com)
 * @brief testing servo module with FreeRTOS
 * @version 0.4
 * @date 2026-05-31
 * @copyright Copyright (c) 2026
 * */

#include "test_servo.h"
#include "bsp_servo.h"
#include "bsp_log.h"
#include "test_config.h"

#include "system_stm32f1xx.h"
#include "Driver_Timer.h"
#include "Driver_GPIO.h"
#include "Driver_RCC.h"

#include <stdio.h>
#include "cmsis_os2.h"

static void vTestServoTask(void *argument) {
    (void)argument;

    bsp_log_printf("[Test] Servo RTOS Task Started!\r\n");

    while (1) {
        bsp_log_printf("[Servo] Angle = 0\r\n");
        BSP_Servo_SetAngle(0);
        osDelay(1000);

        bsp_log_printf("[Servo] Angle = 90\r\n");
        BSP_Servo_SetAngle(90);
        osDelay(1000);

        bsp_log_printf("[Servo] Angle = 180\r\n");
        BSP_Servo_SetAngle(180);
        osDelay(1000);
    }
}

void test_servo_run(void) {

    BSP_Servo_Init();
    bsp_log_init();


    setvbuf(stdout, NULL, _IONBF, 0);

    osKernelInitialize();

    const osThreadAttr_t task_attr = {
        .name = "TestServo_Task",
        .priority = osPriorityNormal,
        .stack_size = 2048
    };
    osThreadNew(vTestServoTask, NULL, &task_attr);

    osKernelStart();

    while (1) {}
}