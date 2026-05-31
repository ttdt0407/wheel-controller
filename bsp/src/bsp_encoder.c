/**
 * @file bsp_encoder.c
 * @author dt (tien.ta.eswe@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2026-05-30
 * 
 * @copyright Copyright (c) 2026
 * 
 */
#include "bsp_encoder.h"

#include "bsp_encoder.h"
#include "bsp_config.h"
#include "Driver_Timer.h"
#include "Driver_GPIO.h"
#include <stddef.h>

extern ARM_DRIVER_TIM_ENCODER Driver_TIM1;
extern ARM_DRIVER_GPIO        Driver_GPIO0;

static uint16_t prev_count = 0U;
static int32_t  total_position = 0;

void BSP_Encoder_Init(void) {
    Driver_GPIO0.Setup(MOTOR_ENCODER_A, NULL);
    Driver_GPIO0.SetDirection(MOTOR_ENCODER_A, ARM_GPIO_INPUT);
    Driver_GPIO0.SetPullResistor(MOTOR_ENCODER_A, ARM_GPIO_PULL_NONE);

    Driver_GPIO0.Setup(MOTOR_ENCODER_B, NULL);
    Driver_GPIO0.SetDirection(MOTOR_ENCODER_B, ARM_GPIO_INPUT);
    Driver_GPIO0.SetPullResistor(MOTOR_ENCODER_B, ARM_GPIO_PULL_NONE);

    Driver_TIM1.Setup(ARM_TIM_3);
    Driver_TIM1.SetMode(ARM_TIM_3, ARM_EITHER_EDGE);

    prev_count = 0U;
    total_position = 0;
}

int16_t BSP_Encoder_GetDelta(void) {
    uint16_t current_count;
    int16_t delta;

    current_count = (uint16_t)Driver_TIM1.GetCount(ARM_TIM_3);
    delta = (int16_t)(current_count - prev_count);
    prev_count = current_count;
    total_position += delta;

    return delta;
}

int32_t BSP_Encoder_GetTotalPosition(void) {
    return total_position;
}