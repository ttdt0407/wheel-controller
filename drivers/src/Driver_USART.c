/**
 * @file Driver_USART.c
 * @author Dinh Tien (tien.ta.eswe@gmail.com)
 * @brief Implementation of USART driver for stm32f103c8t6
 * @version 0.1
 * @date 2026-05-17
 * 
 * @copyright Copyright (c) 2026
 * 
 */

#include <string.h>

#include "Driver_USART.h"
#include "Driver_GPIO.h"
#include "Driver_RCC.h"

#include "bsp_config.h"
#include "ring_buffer.h"

#include "stm32f103xb.h"

/********************************************************************
 * Definitions
 ********************************************************************/

#define ARM_USART_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(1, 0)  /* driver version */
#define BUFFER_SIZE              256 /* must be power of 2 */

typedef struct {
    volatile uint8_t initialized;
    volatile uint8_t powered;
    volatile uint32_t tx_count;
    volatile uint32_t rx_count;
    volatile uint32_t controlled;
    ARM_USART_STATUS status;
    ARM_USART_MODEM_STATUS modem_status;
} USART_Context_t;

static USART_Context_t usart1_ctx;
static uint8_t usart1_tx_data[BUFFER_SIZE];
static ring_buffer_t usart1_tx_rb;
static ARM_USART_SignalEvent_t cb_ev;

extern ARM_DRIVER_GPIO Driver_GPIO0;

/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion = { 
    ARM_USART_API_VERSION,
    ARM_USART_DRV_VERSION
};

/* Driver Capabilities */
static const ARM_USART_CAPABILITIES DriverCapabilities = {
    1, /* supports UART (Asynchronous) mode */
    0, /* supports Synchronous Master mode */
    0, /* supports Synchronous Slave mode */
    0, /* supports UART Single-wire mode */
    0, /* supports UART IrDA mode */
    0, /* supports UART Smart Card mode */
    0, /* Smart Card Clock generator available */
    0, /* RTS Flow Control available */
    0, /* CTS Flow Control available */
    0, /* Transmit completed event: \ref ARM_USART_EVENT_TX_COMPLETE */
    0, /* Signal receive character timeout event: \ref ARM_USART_EVENT_RX_TIMEOUT */
    0, /* RTS Line: 0=not available, 1=available */
    0, /* CTS Line: 0=not available, 1=available */
    0, /* DTR Line: 0=not available, 1=available */
    0, /* DSR Line: 0=not available, 1=available */
    0, /* DCD Line: 0=not available, 1=available */
    0, /* RI Line: 0=not available, 1=available */
    0, /* Signal CTS change event: \ref ARM_USART_EVENT_CTS */
    0, /* Signal DSR change event: \ref ARM_USART_EVENT_DSR */
    0, /* Signal DCD change event: \ref ARM_USART_EVENT_DCD */
    0, /* Signal RI change event: \ref ARM_USART_EVENT_RI */
    0  /* Reserved (must be zero) */
};

/********************************************************************
 * Helper function
 ********************************************************************/

void USART1_ResetContext(USART_Context_t *ctx) {
    if (ctx != NULL) {
        memset(ctx, 0, sizeof(USART_Context_t));
    }
}

static ARM_DRIVER_VERSION ARM_USART_GetVersion(void)
{
  return DriverVersion;
}

static ARM_USART_CAPABILITIES ARM_USART_GetCapabilities(void)
{
  return DriverCapabilities;
}

static int32_t ARM_USART_Initialize(ARM_USART_SignalEvent_t cb_event)
{
    if (cb_event == NULL) {
        return ARM_DRIVER_ERROR;
    }

    USART1_ResetContext(&usart1_ctx);
    usart1_ctx.initialized = 1;

    if (!rb_init(&usart1_tx_rb, usart1_tx_data, BUFFER_SIZE)) {
        return ARM_DRIVER_ERROR;
    }
    cb_ev = cb_event;

    return ARM_DRIVER_OK;
}

static int32_t ARM_USART_Uninitialize(void)
{
    USART1_ResetContext(&usart1_ctx);

    return ARM_DRIVER_OK;
}

static int32_t ARM_USART_PowerControl(ARM_POWER_STATE state)
{
    int32_t result = ARM_DRIVER_OK;

    switch (state)
    {
    case ARM_POWER_OFF:
        USART1->CR1 &= ~((1 << USART_CR1_UE_Pos) | (1 << USART_CR1_RE_Pos) | (1 << USART_CR1_TE_Pos));
        usart1_ctx.powered = 0;
        NVIC_DisableIRQ(USART1_IRQn);
        break;

    case ARM_POWER_LOW:
        result = ARM_DRIVER_ERROR_UNSUPPORTED;
        break;

    case ARM_POWER_FULL:
        RCC_AFIO_CLK_EN();
        RCC_USART1_EN();
        /* TX pin */
        Driver_GPIO0.Setup(USART1_TX_PIN, NULL);
        Driver_GPIO0.SetDirection(USART1_TX_PIN, ARM_GPIO_AF_OUTPUT);
        Driver_GPIO0.SetOutputMode(USART1_TX_PIN, ARM_AFIO_PUSH_PULL);
        NVIC_SetPriority(USART1_IRQn, 5);
        NVIC_EnableIRQ(USART1_IRQn);
        usart1_ctx.powered = 1;
        break;
    }
    return result;
}

static int32_t ARM_USART_Send(const void *data, uint32_t num)
{
    if (data == NULL || num == 0) {
        return ARM_DRIVER_ERROR;
    }

    if (usart1_ctx.initialized == 0 || usart1_ctx.powered == 0 || usart1_ctx.controlled == 0) {
        return ARM_DRIVER_ERROR;
    }

    uint8_t *p_data = (uint8_t *)data;
    uint16_t byte_copied = 0;

    while (byte_copied < num) {
        if (!rb_put(&usart1_tx_rb, *p_data++)) {
            break;
        } else {
            byte_copied++;
        }
    }

    if (byte_copied > 0) {
        USART1->CR1 |= (1 << USART_CR1_TXEIE_Pos);
    }

    return byte_copied;
}

static int32_t ARM_USART_Receive(void *data, uint32_t num)
{
    (void)data;
    (void)num;
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

static int32_t ARM_USART_Transfer(const void *data_out, void *data_in, uint32_t num)
{
    (void)data_out;
    (void)data_in;
    (void)num;

    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

static uint32_t ARM_USART_GetTxCount(void)
{
    return usart1_ctx.tx_count;
}

static uint32_t ARM_USART_GetRxCount(void)
{
    return usart1_ctx.rx_count;
}

static int32_t ARM_USART_Control(uint32_t control, uint32_t arg)
{
    if (usart1_ctx.initialized == 0 || usart1_ctx.powered == 0) {
        return ARM_DRIVER_ERROR;
    }
    switch (control & ARM_USART_CONTROL_Msk) {
        case ARM_USART_MODE_ASYNCHRONOUS: {
            if (arg <= 0) {
                return ARM_DRIVER_ERROR;
            }

            uint32_t pclk = 72000000;
            uint16_t brr_val;

            switch(control & ARM_USART_DATA_BITS_Msk) {
                case ARM_USART_DATA_BITS_8:
                    USART1->CR1 &= ~(1 << USART_CR1_M_Pos);
                    break;
                case ARM_USART_DATA_BITS_9:
                    USART1->CR1 |= (1 << USART_CR1_M_Pos);
                    break;
                default:
                    return ARM_DRIVER_ERROR;
            }

            switch (control & ARM_USART_PARITY_Msk) {
                case ARM_USART_PARITY_NONE:
                    USART1->CR1 &= ~(1 << USART_CR1_PCE_Pos);
                    break;
                case ARM_USART_PARITY_EVEN:
                    USART1->CR1 &= ~(1 << USART_CR1_PS_Pos);
                    USART1->CR1 |= (1 << USART_CR1_PCE_Pos);
                    break;
                case ARM_USART_PARITY_ODD:
                    USART1->CR1 |= (1 << USART_CR1_PS_Pos);
                    USART1->CR1 |= (1 << USART_CR1_PCE_Pos);
                    break;
                default:
                    return ARM_DRIVER_ERROR;
            }

            switch (control & ARM_USART_STOP_BITS_Msk) {
                case ARM_USART_STOP_BITS_1:
                    USART1->CR2 &= ~(0b11 << USART_CR2_STOP_Pos);
                    break;
                case ARM_USART_STOP_BITS_2:
                    USART1->CR2 &= ~(0b11 << USART_CR2_STOP_Pos);
                    USART1->CR2 |= (0b10 << USART_CR2_STOP_Pos);
                    break;
                case ARM_USART_STOP_BITS_1_5:
                    USART1->CR2 &= ~(0b11 << USART_CR2_STOP_Pos);
                    USART1->CR2 |= (0b11 << USART_CR2_STOP_Pos);
                    break;
                case ARM_USART_STOP_BITS_0_5:
                    USART1->CR2 &= ~(0b11 << USART_CR2_STOP_Pos);
                    USART1->CR2 |= (0b1 << USART_CR2_STOP_Pos);
                    break;
                default:
                    return ARM_DRIVER_ERROR;
            }

            brr_val = (pclk + (arg / 2)) / arg;
            USART1->BRR = brr_val;
            break;
        }
        case ARM_USART_CONTROL_TX:
            if (arg == 0) {
                USART1->CR1 &= ~(1 << USART_CR1_TE_Pos);
            } else {
                USART1->CR1 |= (1 << USART_CR1_TE_Pos);
            }
            break;

        case ARM_USART_CONTROL_RX:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }

    USART1->CR1 |= (1 << USART_CR1_UE_Pos);
    usart1_ctx.controlled = 1;

    return ARM_DRIVER_OK;
}

static ARM_USART_STATUS ARM_USART_GetStatus(void)
{
    return usart1_ctx.status;
}

static int32_t ARM_USART_SetModemControl(ARM_USART_MODEM_CONTROL control)
{
    (void) control;
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

static ARM_USART_MODEM_STATUS ARM_USART_GetModemStatus(void)
{
    return usart1_ctx.modem_status;
}

static void ARM_USART_SignalEvent(uint32_t event)
{
    // function body
}

void USART1_IRQHandler(void) {
    if (((USART1->SR & (1 << USART_SR_TXE_Pos)) >> USART_SR_TXE_Pos) && ((USART1->CR1 & (1 << USART_CR1_TXEIE_Pos)) >> USART_CR1_TXEIE_Pos)) {
        uint8_t data;

        if (rb_get(&usart1_tx_rb, &data)) {
            USART1->DR = data;
        }
        else
        {
            USART1->CR1 &= ~(1 << USART_CR1_TXEIE_Pos);
            if (cb_ev != NULL) {
                cb_ev(ARM_USART_EVENT_TX_COMPLETE);
            }
        }
    }
}
// End USART Interface

extern \
ARM_DRIVER_USART Driver_USART1;
ARM_DRIVER_USART Driver_USART1 = {
    ARM_USART_GetVersion,
    ARM_USART_GetCapabilities,
    ARM_USART_Initialize,
    ARM_USART_Uninitialize,
    ARM_USART_PowerControl,
    ARM_USART_Send,
    ARM_USART_Receive,
    ARM_USART_Transfer,
    ARM_USART_GetTxCount,
    ARM_USART_GetRxCount,
    ARM_USART_Control,
    ARM_USART_GetStatus,
    ARM_USART_SetModemControl,
    ARM_USART_GetModemStatus
};