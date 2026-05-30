#include "test_console.h"
#include "bsp_log.h"
#include "test_config.h"

#include "test_gpio.h"

#include "system_stm32f1xx.h"

#include "Driver_GPIO.h"
#include "Driver_RCC.h"

#include <stdio.h>
#include "cmsis_os2.h"

extern ARM_DRIVER_GPIO Driver_GPIO0;
extern ARM_DRIVER_RCC Driver_RCC0;

static void vTestConsoleTask(void *argument) {
    (void)argument;

    while (1) {
        bsp_log_printf("Hello heo Peppa <3\r\n");
        osDelay(500);
    }
}

void test_console_run(void) {
    Driver_RCC0.SetSystemClock();
    SystemCoreClockUpdate();

    RCC_GPIOC_CLK_EN();
    Driver_GPIO0.Setup(LED, NULL);
    Driver_GPIO0.SetDirection(LED, ARM_GPIO_OUTPUT);
    Driver_GPIO0.SetOutputMode(LED, ARM_GPIO_PUSH_PULL);

    // Disable stdout buffering for embedded systems
    setvbuf(stdout, NULL, _IONBF, 0);

    osKernelInitialize();

    bsp_log_init();

    const osThreadAttr_t task_attr = {
        .name = "TestConsole_Task",
        .priority = osPriorityNormal,
        .stack_size = 2048
    };
    osThreadNew(vTestConsoleTask, NULL, &task_attr);

    // Start RTOS scheduler
    osKernelStart();

    // Should never reach here
    while (1) {}
}
