#include "test_can.h"
#include "test_config.h"

#include "bsp_can.h"
#include "bsp_log.h"

#include "Driver_CAN.h"
#include "Driver_GPIO.h"
#include "Driver_RCC.h"

#include "cmsis_os2.h"
#include <string.h>

#define NODE_ROLE_MASTER 1
#define NODE_ROLE_SLAVE  2

#define CURRENT_NODE_ROLE NODE_ROLE_MASTER

#define MASTER_TX_ID 0x111
#define SLAVE_TX_ID  0x222

extern ARM_DRIVER_CAN Driver_CAN0;
extern ARM_DRIVER_GPIO Driver_GPIO0;
extern ARM_DRIVER_RCC Driver_RCC0;

#if (CURRENT_NODE_ROLE == NODE_ROLE_MASTER)
static void vCANMasterTask(void *argument) {
    (void)argument;

    CAN_Message_t tx_msg;
    CAN_Message_t rx_msg;

    tx_msg.id = MASTER_TX_ID;
    tx_msg.dlc = 1;
    tx_msg.isExt = 0;
    tx_msg.isRTR = 0;
    tx_msg.data[0] = 1;

    static int led_state = ON;

    bsp_log_printf("[INFO] Mode: NORMAL. TX_ID: 0x%X\r\n", MASTER_TX_ID);

    while (1) {
        bsp_log_printf("[MASTER] Sending Command: %d ... ", tx_msg.data[0]);

        if (BSP_CAN_Write(&tx_msg, 100)) {
            bsp_log_printf("OK!\r\n");
        } else {
            bsp_log_printf("FAILED\r\n");
        }

        osDelay(200);

        if (BSP_CAN_Read(&rx_msg, 500)) {
            if (rx_msg.id == SLAVE_TX_ID) {
                bsp_log_printf("[MASTER] Received ACK from 0x%03X: ", (unsigned int)rx_msg.id);
                for (int i = 0; i < rx_msg.dlc; i++) {
                    bsp_log_printf("%02X ", rx_msg.data[i]);
                }
                bsp_log_printf("\r\n");
            }
        }

        tx_msg.data[0] = (tx_msg.data[0] == 1) ? 0 : 1;

        Driver_GPIO0.SetOutput(LED, led_state);
        led_state = (led_state == ON) ? OFF : ON;

        bsp_log_printf("------------------------------------------\r\n");
        osDelay(800);
    }
}
#endif

#if (CURRENT_NODE_ROLE == NODE_ROLE_SLAVE)
static void vCANSlaveTask(void *argument) {
    (void)argument;
    
    CAN_Message_t rx_msg;
    CAN_Message_t tx_msg;
    
    tx_msg.id = SLAVE_TX_ID;
    tx_msg.dlc = 3;
    tx_msg.isExt = 0;
    tx_msg.isRTR = 0;
    tx_msg.data[0] = 0xAA;
    tx_msg.data[1] = 0xBB;

    while (1) {
        if (BSP_CAN_Read(&rx_msg, osWaitForever)) {
            if (rx_msg.id == MASTER_TX_ID && rx_msg.dlc == 1) {

                if (rx_msg.data[0] == 1) {
                    Driver_GPIO0.SetOutput(LED, ON);
                    tx_msg.data[2] = 1;
                } else {
                    Driver_GPIO0.SetOutput(LED, OFF);
                    tx_msg.data[2] = 0;
                }

                BSP_CAN_Write(&tx_msg, 100);
            }
        }
    }
}
#endif

void test_can_run(void) {
    BSP_Init();
    BSP_CAN_Init();
    Driver_CAN0.SetMode(ARM_CAN_MODE_NORMAL);

    osKernelInitialize();

#if (CURRENT_NODE_ROLE == NODE_ROLE_MASTER)
    bsp_log_init();

    const osThreadAttr_t master_attr = {
        .name = "CAN_Master",
        .priority = osPriorityNormal,
        .stack_size = 1024
    };
    osThreadNew(vCANMasterTask, NULL, &master_attr);
#endif

#if (CURRENT_NODE_ROLE == NODE_ROLE_SLAVE)
    const osThreadAttr_t slave_attr = {
        .name = "CAN_Slave",
        .priority = osPriorityNormal1,
        .stack_size = 512
    };
    osThreadNew(vCANSlaveTask, NULL, &slave_attr);
#endif

    osKernelStart();

    while (1) {}
}