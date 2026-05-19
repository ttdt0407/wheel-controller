#include "test_timer.h"
#include "test_config.h"

#include "bsp_config.h"
#include "bsp_hbridge.h"
#include "bsp_encoder.h"
#include "bsp_servo.h"
#include "bsp_log.h"

#include "Driver_GPIO.h"
#include "cmsis_os2.h"

extern ARM_DRIVER_GPIO Driver_GPIO0;

#if (CONTROL_MOTOR_TEST == ENABLE || SERVO_MOTOR_TEST == ENABLE)

static void vMotorServoTask(void *argument) {
    (void)argument;
    int8_t speed = 0;
    int8_t speed_step = 10;
    uint8_t angle = 90;
    uint8_t angle_step = 10;

    bsp_log_printf("[INFO] Motor & Servo Test Task Started.\r\n");

    while (1) {
#if (CONTROL_MOTOR_TEST == ENABLE)
        bsp_log_printf("[TEST] Motor accelerating FORWARD | Servo turning RIGHT...\r\n");
        for (speed = 0, angle = 90; speed <= 100; speed += speed_step, angle += angle_step) {
            if (angle > 180) angle = 180;

            BSP_Motor_SetSpeed(speed);
            BSP_Servo_SetAngle(angle);
            osDelay(150);
        }

        osDelay(1000);

        bsp_log_printf("[TEST] Motor accelerating BACKWARD | Servo turning LEFT...\r\n");
        for (speed = 100, angle = 180; speed >= -100; speed -= speed_step, angle -= angle_step) {
            if ((int16_t)angle < 0) angle = 0;

            BSP_Motor_SetSpeed(speed);
            BSP_Servo_SetAngle(angle);
            osDelay(150);
        }

        osDelay(1000);

        bsp_log_printf("[TEST] Resetting Actuators to Neutral (Speed: 0, Angle: 90)\r\n");
        BSP_Motor_SetSpeed(0);
        BSP_Servo_SetAngle(90);
#endif
        osDelay(2000);
    }
}
#endif

#if (ENCODER_MOTOR_TEST == ENABLE)

static void vEncoderReadTask(void *argument) {
    (void)argument;
    int16_t delta_ticks = 0;
    int32_t total_ticks = 0;

    bsp_log_printf("[INFO] Encoder Sampling Task Started (Period: 50ms).\r\n");

    while (1) {
        delta_ticks = BSP_Encoder_GetDelta();
        total_ticks = BSP_Encoder_GetTotalPosition();

        bsp_log_printf("[ENCODER] Delta(50ms): %d | Total Position: %d\r\n", (int)delta_ticks, (int)total_ticks);
        osDelay(50); 
    }
}
#endif


void test_timer_run(void) {
    BSP_Init();

    osKernelInitialize();

    bsp_log_init();

    bsp_log_printf("\r\n==========================================\r\n");
    bsp_log_printf("      STM32F103 VEHICLE TIMER TESTBENCH   \r\n");
    bsp_log_printf("==========================================\r\n");

#if (CONTROL_MOTOR_TEST == ENABLE || SERVO_MOTOR_TEST == ENABLE)
    const osThreadAttr_t motor_servo_attr = {
        .name = "Motor_Servo_Ctrl",
        .priority = osPriorityNormal,
        .stack_size = 512
    };
    osThreadNew(vMotorServoTask, NULL, &motor_servo_attr);
#endif

#if (ENCODER_MOTOR_TEST == ENABLE)
    const osThreadAttr_t encoder_attr = {
        .name = "Encoder_Sampling",
        .priority = osPriorityNormal1,
        .stack_size = 512
    };
    osThreadNew(vEncoderReadTask, NULL, &encoder_attr);
#endif

    osKernelStart();

    while (1) {

    }
}