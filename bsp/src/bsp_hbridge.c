/**
 * @file bsp_hbridge.c
 * @author dt (tien.ta.eswe@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2026-05-30
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "bsp_hbridge.h"
#include "bsp_config.h"
#include "Driver_Timer.h"
#include "Driver_GPIO.h"
#include <stddef.h>

extern ARM_DRIVER_TIM_PWM Driver_TIM0;
extern ARM_DRIVER_GPIO    Driver_GPIO0;

void BSP_Motor_Init(void) {

    Driver_GPIO0.Setup(MOTOR_PWM_PIN, NULL);
    Driver_GPIO0.SetDirection(MOTOR_PWM_PIN, ARM_GPIO_AF_OUTPUT);
    Driver_GPIO0.SetOutputMode(MOTOR_PWM_PIN, ARM_AFIO_PUSH_PULL);

    Driver_GPIO0.Setup(MOTOR_IN1, NULL);
    Driver_GPIO0.SetDirection(MOTOR_IN1, ARM_GPIO_OUTPUT);
    Driver_GPIO0.SetOutputMode(MOTOR_IN1, ARM_GPIO_PUSH_PULL);

    Driver_GPIO0.Setup(MOTOR_IN2, NULL);
    Driver_GPIO0.SetDirection(MOTOR_IN2, ARM_GPIO_OUTPUT);
    Driver_GPIO0.SetOutputMode(MOTOR_IN2, ARM_GPIO_PUSH_PULL);

    Driver_GPIO0.SetOutput(MOTOR_IN1, LOW);
    Driver_GPIO0.SetOutput(MOTOR_IN2, LOW);

    Driver_TIM0.Setup(ARM_TIM_1, PWM_FREQ);
    Driver_TIM0.SetMode(ARM_TIM_1, ARM_CHANNEL_1);
    Driver_TIM0.SetDuty(ARM_TIM_1, ARM_CHANNEL_1, 0U);
    Driver_TIM0.Trigger(ARM_TIM_1, ARM_CHANNEL_1);
}

void BSP_Motor_SetSpeed(int8_t speed_percent) {
    uint16_t duty_scaled = 0U;

    if (speed_percent > 100)  speed_percent = 100;
    if (speed_percent < -100) speed_percent = -100;

    if (speed_percent >= 0) {
        Driver_GPIO0.SetOutput(MOTOR_IN1, HIGH);
        Driver_GPIO0.SetOutput(MOTOR_IN2, LOW);
        duty_scaled = (uint16_t)speed_percent * 100U;
    } else {
        Driver_GPIO0.SetOutput(MOTOR_IN1, LOW);
        Driver_GPIO0.SetOutput(MOTOR_IN2, HIGH);
        duty_scaled = (uint16_t)(-speed_percent) * 100U;
    }

    Driver_TIM0.SetDuty(ARM_TIM_1, ARM_CHANNEL_1, duty_scaled);
}