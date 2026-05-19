#include "bsp_config.h"
#include "bsp_hbridge.h"
#include "bsp_encoder.h"
#include "bsp_servo.h"

#include "Driver_GPIO.h"
#include "Driver_RCC.h"

#include <stddef.h>

extern ARM_DRIVER_GPIO Driver_GPIO0;
extern ARM_DRIVER_RCC Driver_RCC0;

void BSP_Init(void) {
    Driver_RCC0.SetSystemClock();
    SystemCoreClockUpdate();

    Driver_GPIO0.Setup(LED, NULL);
    Driver_GPIO0.SetDirection(LED, ARM_GPIO_OUTPUT);
    Driver_GPIO0.SetOutputMode(LED, ARM_GPIO_PUSH_PULL);
    Driver_GPIO0.SetOutput(LED, OFF);

    Driver_GPIO0.Setup(BUTTON, NULL);
    Driver_GPIO0.SetDirection(BUTTON, ARM_GPIO_INPUT);
    Driver_GPIO0.SetPullResistor(BUTTON, ARM_GPIO_PULL_UP);

    BSP_Motor_Init();
    BSP_Encoder_Init();
    BSP_Servo_Init();

    Driver_GPIO0.SetOutput(LED, ON);
}