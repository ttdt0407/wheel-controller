#ifndef TEST_CONFIG_H
#define TEST_CONFIG_H

#include "test_gpio.h"
#include "test_log.h"

#include "bsp_config.h"

#include "Driver_RCC.h"
#include "Driver_GPIO.h"
#include "Driver_Timer.h"

#include "system_stm32f1xx.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief GPIO constant
 */
#define GPIO_TEST           DISABLE
#define BLINK_LED_TEST      DISABLE
#define BUTTON_INT_TEST     DISABLE


/**
 * @brief Log test
 */
#define LOG_TEST            DISABLE

/**
 * @brief CAN test
 */
#define CAN_TEST            DISABLE

/**
 * @brief Timer test
 */
#define SERVO_TEST          DISABLE

/**
 * @brief MOTOR test
 * 
 */
#define MOTOR_TEST          ENABLE

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* TEST_CONFIG_H */