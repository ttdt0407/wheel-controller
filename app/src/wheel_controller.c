#include "cmsis_os2.h"
#include "bsp_can.h"
#include "bsp_encoder.h"
#include "bsp_hbridge.h"
#include "bsp_servo.h"
#include "bsp_log.h"


#define CAN_ID_MASTER_CMD   0x100
#define CAN_ID_THIS_WHEEL   0x111

typedef struct {
    uint16_t target_speed;
    uint8_t target_angle;
} CommandState_t;

typedef struct {
    uint16_t current_speed;
    uint8_t current_angle;
    uint8_t error_code;
} ActualState_t;

static CommandState_t g_CmdState = {0, 90};
static ActualState_t  g_ActState = {0, 90};
static osMutexId_t    g_StateMutex = NULL;

typedef struct {
    float Kp, Ki, Kd;
    float integral;
    float prev_error;
    float out_min, out_max;
} PID_Controller_t;

static PID_Controller_t motor_pid = {
    .Kp = 4.0f, .Ki = 3.0f, .Kd = 0.001f,
    .integral = 0.0f, .prev_error = 0.0f,
    .out_min = -100.0f, .out_max = 100.0f // Giới hạn duty cycle PWM
};

static int16_t PID_Compute(PID_Controller_t *pid, int16_t setpoint, int16_t measured, float dt) {
    float error = (float)(setpoint - measured);
    
    pid->integral += error * dt;
    float derivative = (error - pid->prev_error) / dt;
    
    float output = (pid->Kp * error) + (pid->Ki * pid->integral) + (pid->Kd * derivative);
    
    pid->prev_error = error;
    
    // Anti-windup & Clamp output
    if (output > pid->out_max) output = pid->out_max;
    else if (output < pid->out_min) output = pid->out_min;
    
    return (int16_t)output;
}

static void vDriveCtrlTask(void *argument) {
    (void)argument;

    uint32_t tick = osKernelGetTickCount();
    const uint32_t TICK_PERIOD_MS = 20U;
    const float DT_SEC = 0.02f;

    int16_t local_target_speed = 0;
    uint8_t local_target_angle = 90;
    int16_t current_speed = 0;

    bsp_log_printf("[DRIVE] Task started. Period: %d ms\r\n", TICK_PERIOD_MS);

    while (1) {
        tick += TICK_PERIOD_MS;
        osDelayUntil(tick);

        int16_t encoder_delta = BSP_Encoder_GetDelta();

        current_speed = encoder_delta;

        if (osMutexAcquire(g_StateMutex, osWaitForever) == osOK) {
            local_target_speed = g_CmdState.target_speed;
            local_target_angle = g_CmdState.target_angle;
            g_ActState.current_speed = current_speed;
            g_ActState.current_angle = local_target_angle;
            osMutexRelease(g_StateMutex);
        }

        BSP_Servo_SetAngle(local_target_angle);

        int16_t pwm_output = 0;
        if (local_target_speed == 0) {
            pwm_output = 0;
            motor_pid.integral = 0;
        } else {
            pwm_output = PID_Compute(&motor_pid, local_target_speed, current_speed, DT_SEC);
        }

        BSP_Motor_SetSpeed((int8_t)pwm_output);
    }
}

static void vCAN_RxTask(void *argument) {
    (void)argument;
    CAN_Message_t rx_msg;

    bsp_log_printf("[CAN_RX] Task started. Listening for ID 0x%03X\r\n", CAN_ID_MASTER_CMD);

    while (1) {
        if (BSP_CAN_Read(&rx_msg, osWaitForever) == true) {
            if (rx_msg.id == CAN_ID_MASTER_CMD && rx_msg.dlc >= 3) {

                int16_t t_speed = (int16_t)((rx_msg.data[1] << 8) | rx_msg.data[0]);
                uint8_t t_angle = rx_msg.data[2];
                if (osMutexAcquire(g_StateMutex, osWaitForever) == osOK) {
                    g_CmdState.target_speed = t_speed;
                    g_CmdState.target_angle = t_angle;
                    osMutexRelease(g_StateMutex);
                }
            }
        }
    }
}

static void vCAN_TxTask(void *argument) {
    (void)argument;
    CAN_Message_t tx_msg;

    tx_msg.id = CAN_ID_THIS_WHEEL;
    tx_msg.dlc = 4;
    tx_msg.isExt = 0;
    tx_msg.isRTR = 0;

    bsp_log_printf("[CAN_TX] Task started. Period: 100ms\r\n");

    while (1) {
        int16_t c_speed = 0;
        uint8_t c_angle = 0;
        uint8_t err_code = 0;

        if (osMutexAcquire(g_StateMutex, osWaitForever) == osOK) {
            c_speed = g_ActState.current_speed;
            c_angle = g_ActState.current_angle;
            err_code = g_ActState.error_code;
            osMutexRelease(g_StateMutex);
        }

        tx_msg.data[0] = (uint8_t)(c_speed & 0xFF);
        tx_msg.data[1] = (uint8_t)((c_speed >> 8) & 0xFF);
        tx_msg.data[2] = c_angle;
        tx_msg.data[3] = err_code;

        if (!BSP_CAN_Write(&tx_msg, 10)) {
        }

        osDelay(100);
    }
}

void WheelController_Init(void) {

    const osMutexAttr_t mutex_attr = {
        .name = "StateMutex",
        .attr_bits = osMutexRecursive | osMutexPrioInherit
    };
    g_StateMutex = osMutexNew(&mutex_attr);

    const osThreadAttr_t drive_attr = {
        .name = "Drive_Ctrl",
        .priority = osPriorityNormal,
        .stack_size = 512
    };
    osThreadNew(vDriveCtrlTask, NULL, &drive_attr);

    const osThreadAttr_t can_rx_attr = {
        .name = "CAN_RX",
        .priority = osPriorityHigh, 
        .stack_size = 512
    };
    osThreadNew(vCAN_RxTask, NULL, &can_rx_attr);

    const osThreadAttr_t can_tx_attr = {
        .name = "CAN_TX",
        .priority = osPriorityBelowNormal,
        .stack_size = 512
    };
    osThreadNew(vCAN_TxTask, NULL, &can_tx_attr);
}
