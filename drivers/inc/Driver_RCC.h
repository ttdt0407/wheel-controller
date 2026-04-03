/**
 * @file Driver_RCC.h
 * @author Tien Ta (tien.ta.eswe@gmail.com)
 * @brief APIs for clock configuration
 * @version 0.1
 * @date 2026-04-01
 *
 * @copyright Copyright (c) 2026
 *
 */

#ifndef DRIVER_RCC_H_
#define DRIVER_RCC_H_

#include "Driver_Common.h"
#include "stm32f103xb.h"

#define RCC_AFIO_CLK_EN()           (RCC->APB2ENR |= (1U << RCC_APB2ENR_AFIOEN_Pos))
#define RCC_GPIOA_CLK_EN()          (RCC->APB2ENR |= (1U << RCC_APB2ENR_IOPAEN_Pos))
#define RCC_GPIOB_CLK_EN()          (RCC->APB2ENR |= (1U << RCC_APB2ENR_IOPBEN_Pos))
#define RCC_GPIOC_CLK_EN()          (RCC->APB2ENR |= (1U << RCC_APB2ENR_IOPCEN_Pos))

int32_t RCC_SystemClock_72Mhz(void);

#endif /* DRIVER_RCC_H_ */
