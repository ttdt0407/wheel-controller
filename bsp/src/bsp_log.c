#include "bsp_log.h"
#include "Driver_USART.h"
#include "cmsis_os2.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define QUEUE_SIZE          10
#define LOG_MSG_LEN         64

typedef struct {
    char msg[LOG_MSG_LEN];
} log_packet_t;

static osMessageQueueId_t mid_log_queue;
static osSemaphoreId_t sid_log_sem;
static osThreadId_t tid_log_thread;
static uint8_t bsp_log_ready;

extern ARM_DRIVER_USART Driver_USART1;

void USART1_Callback(uint32_t event){
    if ((event & ARM_USART_EVENT_TX_COMPLETE) && (sid_log_sem != NULL)) {
        osSemaphoreRelease(sid_log_sem);
    }
}

static void BSP_LogSendDirect(const char *msg) {
    uint32_t len = (uint32_t)strlen(msg);
    uint32_t sent = 0;

    while (sent < len) {
        int32_t res = Driver_USART1.Send(&msg[sent], len - sent);
        if (res > 0) {
            sent += (uint32_t)res;
        }
    }
}

static void vLoggingTask(void *argument) {
    (void)argument;
    log_packet_t rx;

    while(1) {
        if (osMessageQueueGet(mid_log_queue, &rx, NULL, osWaitForever) == osOK)
        {
            uint8_t msg_len = strlen(rx.msg);
            uint32_t byte = 0;

            while (byte < msg_len) {
                int32_t res = Driver_USART1.Send(&rx.msg[byte], msg_len - byte);

                if (res > 0) {
                    byte += res;
                }

                if (byte < msg_len) {
                    osSemaphoreAcquire(sid_log_sem, osWaitForever);
                }
            }
        }
    }
}

void bsp_log_init(void) {

    mid_log_queue = osMessageQueueNew(QUEUE_SIZE, sizeof(log_packet_t), NULL);
    sid_log_sem = osSemaphoreNew(1, 0, NULL);
    const osThreadAttr_t task_attr = {
        .name = "Logginf_Task",
        .priority = osPriorityLow,
        .stack_size = 512
    };
    tid_log_thread = osThreadNew(vLoggingTask, NULL, &task_attr);

    if ((mid_log_queue == NULL) || (sid_log_sem == NULL) || (tid_log_thread == NULL)) {
        bsp_log_ready = 0;
        return;
    }

    if (Driver_USART1.Initialize(USART1_Callback) != ARM_DRIVER_OK) {
        bsp_log_ready = 0;
        return;
    }

    if (Driver_USART1.PowerControl(ARM_POWER_FULL) != ARM_DRIVER_OK) {
        bsp_log_ready = 0;
        return;
    }

    if (Driver_USART1.Control(ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_DATA_BITS_8 | ARM_USART_PARITY_NONE, 115200) != ARM_DRIVER_OK) {
        bsp_log_ready = 0;
        return;
    }

    if (Driver_USART1.Control(ARM_USART_CONTROL_TX, 1) != ARM_DRIVER_OK) {
        bsp_log_ready = 0;
        return;
    }

    bsp_log_ready = 1;
    BSP_LogSendDirect("[bsp_log] ready\r\n");
}

void bsp_log_printf(const char *format, ...) {
    log_packet_t temp;

    va_list args;
    va_start(args, format);
    vsnprintf(temp.msg, LOG_MSG_LEN, format, args);
    va_end(args);

    if (bsp_log_ready && (mid_log_queue != NULL) && (osMessageQueuePut(mid_log_queue, &temp, 0, 0) == osOK)) {
        return;
    }

    BSP_LogSendDirect(temp.msg);
}