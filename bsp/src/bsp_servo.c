/**
 * @file bsp_servo.c
 * @author dt (tien.ta.eswe@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2026-05-30
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include "bsp_servo.h"
#include "bsp_config.h"
#include "Driver_Timer.h"
#include "Driver_GPIO.h"
#include <stddef.h>

extern ARM_DRIVER_TIM_PWM Driver_TIM0;
extern ARM_DRIVER_GPIO    Driver_GPIO0;

void BSP_Servo_Init(void) {
    Driver_GPIO0.Setup(SERVO_PWM_PIN, NULL);
    Driver_GPIO0.SetDirection(SERVO_PWM_PIN, ARM_GPIO_AF_OUTPUT);
    Driver_GPIO0.SetOutputMode(SERVO_PWM_PIN, ARM_AFIO_PUSH_PULL);

    Driver_TIM0.Setup(ARM_TIM_4, SERVO_FREQ);
    Driver_TIM0.SetMode(ARM_TIM_4, ARM_CHANNEL_1);
    Driver_TIM0.Trigger(ARM_TIM_4, ARM_CHANNEL_1);

    BSP_Servo_SetAngle(90U);
}

void BSP_Servo_SetAngle(uint8_t angle_degrees) {
    uint16_t duty_scaled;

    if (angle_degrees > 180U) {
        angle_degrees = 180U;
    }

    duty_scaled = 500U + (uint16_t)(((uint32_t)angle_degrees * 500U) / 180U);
    Driver_TIM0.SetDuty(ARM_TIM_4, ARM_CHANNEL_1, duty_scaled);
}