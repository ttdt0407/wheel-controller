#include "bsp_can.h"
#include "Driver_CAN.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include <string.h>

extern ARM_DRIVER_CAN Driver_CAN0;

static QueueHandle_t     RX_Queue     = NULL;
static QueueHandle_t     TX_Queue     = NULL;
static SemaphoreHandle_t TX_Semaphore = NULL;
static TaskHandle_t      TX_TaskHandle = NULL;

static void CAN_TX_Task(void *argument);

void BSP_CAN_SignalObjectEvent(uint32_t obj_idx, uint32_t event);

void BSP_CAN_Init(void) {

    RX_Queue = xQueueCreate(10, sizeof(CAN_Message_t));
    TX_Queue = xQueueCreate(10, sizeof(CAN_Message_t));

    TX_Semaphore = xSemaphoreCreateBinary();

    xSemaphoreGive(TX_Semaphore);

    Driver_CAN0.Initialize(NULL, BSP_CAN_SignalObjectEvent);
    Driver_CAN0.PowerControl(ARM_POWER_FULL);
    Driver_CAN0.SetBitrate(ARM_CAN_BITRATE_NOMINAL, 500000, 0);
    Driver_CAN0.ObjectSetFilter(3, ARM_CAN_FILTER_ID_EXACT_ADD, 0, 0);
    Driver_CAN0.ObjectConfigure(3, ARM_CAN_OBJ_RX);
    Driver_CAN0.SetMode(ARM_CAN_MODE_NORMAL);

    xTaskCreate(CAN_TX_Task, "CAN_TX", configMINIMAL_STACK_SIZE + 64, NULL, 1, &TX_TaskHandle);
}

bool BSP_CAN_Write(CAN_Message_t *msg, uint32_t timeout_ms) {
    if (TX_Queue == NULL) return false;

    BaseType_t status = xQueueSend(TX_Queue, msg, pdMS_TO_TICKS(timeout_ms));
    return (status == pdPASS);
}

bool BSP_CAN_Read(CAN_Message_t *msg, uint32_t timeout_ms) {
    if (RX_Queue == NULL) return false;

    BaseType_t status = xQueueReceive(RX_Queue, msg, pdMS_TO_TICKS(timeout_ms));
    return (status == pdPASS);
}

void BSP_CAN_SignalObjectEvent(uint32_t obj_idx, uint32_t event) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if (event == ARM_CAN_EVENT_RECEIVE) {
        ARM_CAN_MSG_INFO msg_info;
    memset(&msg_info, 0, sizeof(msg_info));
        CAN_Message_t rx_msg;

        int32_t bytes_read = Driver_CAN0.MessageRead(obj_idx, &msg_info, rx_msg.data, 8);

        if (bytes_read >= 0) {
            rx_msg.id = msg_info.id & 0x1FFFFFFF;
            rx_msg.dlc = (uint8_t)bytes_read;
            rx_msg.isExt = (msg_info.id & ARM_CAN_ID_IDE_Msk) ? 1 : 0;
            rx_msg.isRTR = (msg_info.rtr) ? 1 : 0;

            xQueueSendFromISR(RX_Queue, &rx_msg, &xHigherPriorityTaskWoken);
        }
    }

    if (event == ARM_CAN_EVENT_SEND_COMPLETE) {
        xSemaphoreGiveFromISR(TX_Semaphore, &xHigherPriorityTaskWoken);
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

static void CAN_TX_Task(void *argument) {
    CAN_Message_t tx_msg;
    ARM_CAN_MSG_INFO msg_info;
    memset(&msg_info, 0, sizeof(msg_info));
    uint8_t target_mailbox = 0;
    while(1) {
        if (xQueueReceive(TX_Queue, &tx_msg, portMAX_DELAY) == pdPASS) {

            msg_info.id = tx_msg.id;
            if (tx_msg.isExt) msg_info.id |= ARM_CAN_ID_IDE_Msk;
            if (tx_msg.isRTR) msg_info.rtr = 1;
            xSemaphoreTake(TX_Semaphore, portMAX_DELAY);
            int32_t result = Driver_CAN0.MessageSend(target_mailbox, &msg_info, tx_msg.data, tx_msg.dlc);

            if (result < 0) {
                xSemaphoreGive(TX_Semaphore);
            } else {
                target_mailbox = (target_mailbox + 1) % 3;
            }
        }
    }
}