/**
 * @file Driver_CAN.c
 * @author dt (tien.ta.eswe@gmail.com)
 * @brief CMSIS CAN driver
 * @version 0.1
 * @date 2026-05-18
 * 
 * @copyright Copyright (c) 2026
 * 
 */
 
#include "Driver_CAN.h"
#include "Driver_RCC.h"
#include "Driver_GPIO.h"

#include "bsp_config.h"

#define ARM_CAN_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1,0) // CAN driver version

// Driver Version
static const ARM_DRIVER_VERSION can_driver_version = { ARM_CAN_API_VERSION, ARM_CAN_DRV_VERSION };

// Driver Capabilities
static const ARM_CAN_CAPABILITIES can_driver_capabilities = {
  32U,  // Number of CAN Objects available
  0U,   // Does not support reentrant calls to ARM_CAN_MessageSend, ARM_CAN_MessageRead, ARM_CAN_ObjectConfigure and abort message sending used by ARM_CAN_Control.
  0U,   // Does not support CAN with Flexible Data-rate mode (CAN_FD)
  0U,   // Does not support restricted operation mode
  0U,   // Does not support bus monitoring mode
  0U,   // Does not support internal loopback mode
  0U,   // Does not support external loopback mode
  0U    // Reserved (must be zero)
};

// Object Capabilities
static const ARM_CAN_OBJ_CAPABILITIES can_object_capabilities = {
  1U,   // Object supports transmission
  1U,   // Object supports reception
  0U,   // Object does not support RTR reception and automatic Data transmission
  0U,   // Object does not support RTR transmission and automatic Data reception
  0U,   // Object does not allow assignment of multiple filters to it
  0U,   // Object does not support exact identifier filtering
  0U,   // Object does not support range identifier filtering
  0U,   // Object does not support mask identifier filtering
  0U,   // Object can not buffer messages
  0U    // Reserved (must be zero)
};

static uint8_t                     can_driver_powered     = 0U;
static uint8_t                     can_driver_initialized = 0U;
static ARM_CAN_SignalUnitEvent_t   CAN_SignalUnitEvent    = NULL;
static ARM_CAN_SignalObjectEvent_t CAN_SignalObjectEvent  = NULL;

//
//   Functions
//

extern ARM_DRIVER_GPIO Driver_GPIO0;

static ARM_DRIVER_VERSION ARM_CAN_GetVersion (void) {
  // Return driver version
  return can_driver_version;
}

static ARM_CAN_CAPABILITIES ARM_CAN_GetCapabilities (void) {
  // Return driver capabilities
  return can_driver_capabilities;
}

static int32_t ARM_CAN_Initialize (ARM_CAN_SignalUnitEvent_t   cb_unit_event,
                                   ARM_CAN_SignalObjectEvent_t cb_object_event) {

  if (can_driver_initialized != 0U) { return ARM_DRIVER_OK; }

  CAN_SignalUnitEvent   = cb_unit_event;
  CAN_SignalObjectEvent = cb_object_event;

  /* Turn on clock */
  RCC_AFIO_CLK_EN();
  RCC_GPIOA_CLK_EN();

  /* Tx pin */
  Driver_GPIO0.Setup(CAN_TX_PIN, NULL);
  Driver_GPIO0.SetDirection(CAN_TX_PIN, ARM_GPIO_AF_OUTPUT);
  Driver_GPIO0.SetOutputMode(CAN_TX_PIN, ARM_AFIO_PUSH_PULL);

  /* Rx pin */
  Driver_GPIO0.Setup(CAN_RX_PIN, NULL);
  Driver_GPIO0.SetDirection(CAN_RX_PIN, ARM_GPIO_INPUT);
  can_driver_initialized = 1U;

  return ARM_DRIVER_OK;
}

static int32_t ARM_CAN_Uninitialize (void) {

  // Add code for pin, memory, RTX objects de-initialization
  // ..

  can_driver_initialized = 0U;

  return ARM_DRIVER_OK;
}

static int32_t ARM_CAN_PowerControl (ARM_POWER_STATE state) {
  switch (state) {
    case ARM_POWER_OFF:
      can_driver_powered = 0U;

      NVIC_DisableIRQ(USB_HP_CAN1_TX_IRQn);
      NVIC_DisableIRQ(USB_LP_CAN1_RX0_IRQn);
      RCC->APB1RSTR |= RCC_APB1RSTR_CAN1RST;
      RCC->APB1RSTR &= ~RCC_APB1RSTR_CAN1RST;
      RCC->APB1ENR &= ~RCC_APB1ENR_CAN1EN;
      break;

    case ARM_POWER_FULL:
      if (can_driver_initialized == 0U) { return ARM_DRIVER_ERROR; }
      if (can_driver_powered     != 0U) { return ARM_DRIVER_OK;    }

	    RCC->APB1ENR |= RCC_APB1ENR_CAN1EN;
	    CAN1->MCR |= CAN_MCR_INRQ;
      while ((CAN1->MSR & CAN_MSR_INAK) == 0);
      CAN1->MCR &= ~CAN_MCR_SLEEP;
      NVIC_ClearPendingIRQ(USB_HP_CAN1_TX_IRQn);
      CAN1->IER |= CAN_IER_FMPIE0 | CAN_IER_TMEIE;
      // Priority phải >= 5 (để an toàn với FreeRTOS)
      NVIC_SetPriority(USB_HP_CAN1_TX_IRQn, 5);
      NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 5);
      NVIC_EnableIRQ(USB_HP_CAN1_TX_IRQn);
      NVIC_ClearPendingIRQ(USB_LP_CAN1_RX0_IRQn);
      NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
      can_driver_powered = 1U;
      break;

    case ARM_POWER_LOW:
      return ARM_DRIVER_ERROR_UNSUPPORTED;
  }

  return ARM_DRIVER_OK;
}

static uint32_t ARM_CAN_GetClock (void) {

  // Add code to return peripheral clock frequency
  // ..
  return ARM_DRIVER_OK;
}

static int32_t ARM_CAN_SetBitrate (ARM_CAN_BITRATE_SELECT select, uint32_t bitrate, uint32_t bit_segments) {

  if (can_driver_powered == 0U) { return ARM_DRIVER_ERROR; }

  if (select != ARM_CAN_BITRATE_NOMINAL) {
      return ARM_DRIVER_ERROR_UNSUPPORTED;
  }

  if (bitrate != 500000U) {
      return ARM_DRIVER_ERROR_UNSUPPORTED; // Báo lỗi nếu tầng OS đòi tốc độ khác
  }

  if ((CAN1->MSR & CAN_MSR_INAK) == 0) {
      CAN1->MCR |= CAN_MCR_INRQ;
      while ((CAN1->MSR & CAN_MSR_INAK) == 0); // Đợi phần cứng xác nhận
  }

  CAN1->BTR = (0U << 24) |    // SJW: 1 tq
              (2U << 20) |    // TS2: 3 tq
              (13U << 16) |   // TS1: 14 tq
              (3U << 0);      // BRP: Prescaler = 4

  return ARM_DRIVER_OK;
}

static int32_t ARM_CAN_SetMode (ARM_CAN_MODE mode) {

  if (can_driver_powered == 0U) { return ARM_DRIVER_ERROR; }

  switch (mode) {
    case ARM_CAN_MODE_INITIALIZATION:
      CAN1->MCR |= CAN_MCR_INRQ;
      while ((CAN1->MSR & CAN_MSR_INAK) == 0);
      break;
    case ARM_CAN_MODE_NORMAL:
      CAN1->MCR |= CAN_MCR_INRQ;
      while ((CAN1->MSR & CAN_MSR_INAK) == 0);
      CAN1->BTR &= ~(CAN_BTR_LBKM | CAN_BTR_SILM);
      /* Enter normal mode */
      CAN1->MCR &= ~CAN_MCR_INRQ;
      while ((CAN1->MSR & CAN_MSR_INAK) != 0);
      break;
    case ARM_CAN_MODE_RESTRICTED:
      // Add code to put peripheral into restricted operation mode
      // ..
      break;
    case ARM_CAN_MODE_MONITOR:
      CAN1->MCR |= CAN_MCR_INRQ;
      while ((CAN1->MSR & CAN_MSR_INAK) == 0);

      CAN1->BTR &= ~CAN_BTR_LBKM;
      CAN1->BTR |= CAN_BTR_SILM;
      CAN1->MCR &= ~CAN_MCR_INRQ;
      while ((CAN1->MSR & CAN_MSR_INAK) != 0);
      break;
    case ARM_CAN_MODE_LOOPBACK_INTERNAL:
      CAN1->MCR |= CAN_MCR_INRQ;
      while ((CAN1->MSR & CAN_MSR_INAK) == 0);

      CAN1->BTR &= ~CAN_BTR_SILM;
      CAN1->BTR |= CAN_BTR_LBKM; // Bật bit Loopback

      CAN1->MCR &= ~CAN_MCR_INRQ;
      while ((CAN1->MSR & CAN_MSR_INAK) != 0);
      break;
    case ARM_CAN_MODE_LOOPBACK_EXTERNAL:
      // Add code to put peripheral into external loopback mode
      // ..
      break;
  }

  return ARM_DRIVER_OK;
}

static ARM_CAN_OBJ_CAPABILITIES ARM_CAN_ObjectGetCapabilities (uint32_t obj_idx) {
  // Return object capabilities
  return can_object_capabilities;
}

static int32_t ARM_CAN_ObjectSetFilter (uint32_t obj_idx, ARM_CAN_FILTER_OPERATION operation, uint32_t id, uint32_t arg) {

  if (can_driver_powered == 0U) { return ARM_DRIVER_ERROR; }

  if (obj_idx != 3U) { return ARM_DRIVER_ERROR_PARAMETER; }

  switch (operation) {
    case ARM_CAN_FILTER_ID_EXACT_ADD:
      CAN1->FMR |= CAN_FMR_FINIT;
      CAN1->FA1R &= ~(1U << 0);

      CAN1->FS1R  |=  (1U << 0);
      CAN1->FM1R  &= ~(1U << 0);
      CAN1->FFA1R &= ~(1U << 0);

      CAN1->sFilterRegister[0].FR1 = 0x00000000;
      CAN1->sFilterRegister[0].FR2 = 0x00000000;

      CAN1->FA1R |= (1U << 0);
      CAN1->FMR &= ~CAN_FMR_FINIT;
      break;
    case ARM_CAN_FILTER_ID_MASKABLE_ADD:
      // Add code to setup peripheral to receive messages with specified maskable ID
      break;
    case ARM_CAN_FILTER_ID_RANGE_ADD:
      return ARM_DRIVER_ERROR_UNSUPPORTED;
      break;
    case ARM_CAN_FILTER_ID_EXACT_REMOVE:
      return ARM_DRIVER_ERROR_UNSUPPORTED;
      break;
    case ARM_CAN_FILTER_ID_MASKABLE_REMOVE:
      return ARM_DRIVER_ERROR_UNSUPPORTED;
      break;
    case ARM_CAN_FILTER_ID_RANGE_REMOVE:
      return ARM_DRIVER_ERROR_UNSUPPORTED;
      break;
  }

  return ARM_DRIVER_OK;
}

static int32_t ARM_CAN_ObjectConfigure (uint32_t obj_idx, ARM_CAN_OBJ_CONFIG obj_cfg) {

  if (can_driver_powered == 0U) { return ARM_DRIVER_ERROR; }

  switch (obj_cfg) {
    case ARM_CAN_OBJ_INACTIVE:
      // Deactivate object
      // ..
      break;
    case ARM_CAN_OBJ_RX_RTR_TX_DATA:
      // Setup object to automatically return data when RTR with it's ID is received
      // ..
      break;
    case ARM_CAN_OBJ_TX_RTR_RX_DATA:
      // Setup object to send RTR and receive data response
      // ..
      break;
    case ARM_CAN_OBJ_TX:
      // Setup object to be used for sending messages
      // ..
      break;
    case ARM_CAN_OBJ_RX:
      // Setup object to be used for receiving messages
      // ..
      break;
  }

  return ARM_DRIVER_OK;
}

static int32_t ARM_CAN_MessageSend (uint32_t obj_idx, ARM_CAN_MSG_INFO *msg_info, const uint8_t *data, uint8_t size) {

  if (can_driver_powered == 0U) { return ARM_DRIVER_ERROR; }

  if (obj_idx > 2U) { return ARM_DRIVER_ERROR_PARAMETER; }
  if ((CAN1->TSR & (CAN_TSR_TME0 << obj_idx)) == 0) {
      return ARM_DRIVER_ERROR_BUSY;
  }
  CAN1->sTxMailBox[obj_idx].TIR = 0;
  if (msg_info->id & ARM_CAN_ID_IDE_Msk) {
      CAN1->sTxMailBox[obj_idx].TIR = ((msg_info->id & 0x1FFFFFFF) << 3) | CAN_TI0R_IDE;
  } else {
      CAN1->sTxMailBox[obj_idx].TIR = ((msg_info->id & 0x7FF) << 21);
  }

  CAN1->sTxMailBox[obj_idx].TDTR = (size & 0x0F);
  CAN1->sTxMailBox[obj_idx].TDLR = ((uint32_t)data[3] << 24) |
                                   ((uint32_t)data[2] << 16) |
                                   ((uint32_t)data[1] << 8)  |
                                   ((uint32_t)data[0]);

  CAN1->sTxMailBox[obj_idx].TDHR = ((uint32_t)data[7] << 24) |
                                   ((uint32_t)data[6] << 16) |
                                   ((uint32_t)data[5] << 8)  |
                                   ((uint32_t)data[4]);

  CAN1->sTxMailBox[obj_idx].TIR |= CAN_TI0R_TXRQ;

  return ((int32_t)size);
}

static int32_t ARM_CAN_MessageRead (uint32_t obj_idx, ARM_CAN_MSG_INFO *msg_info, uint8_t *data, uint8_t size) {

  if (can_driver_powered == 0U) { return ARM_DRIVER_ERROR;  }

  if (obj_idx != 3U) { return ARM_DRIVER_ERROR_PARAMETER; }

  if ((CAN1->RF0R & CAN_RF0R_FMP0) == 0) {
      return 0;
  }

  if (CAN1->sFIFOMailBox[0].RIR & CAN_RI0R_IDE) {
      msg_info->id = (0x1FFFFFFF & (CAN1->sFIFOMailBox[0].RIR >> 3)) | ARM_CAN_ID_IDE_Msk;
  } else {
      msg_info->id = (0x000007FF & (CAN1->sFIFOMailBox[0].RIR >> 21));
  }

  uint8_t dlc = CAN1->sFIFOMailBox[0].RDTR & CAN_RDT0R_DLC;
  uint8_t read_size = (dlc < size) ? dlc : size;

  uint32_t tdlr = CAN1->sFIFOMailBox[0].RDLR;
  uint32_t tdhr = CAN1->sFIFOMailBox[0].RDHR;

  if (read_size > 0) data[0] = (tdlr >> 0)  & 0xFF;
  if (read_size > 1) data[1] = (tdlr >> 8)  & 0xFF;
  if (read_size > 2) data[2] = (tdlr >> 16) & 0xFF;
  if (read_size > 3) data[3] = (tdlr >> 24) & 0xFF;
  if (read_size > 4) data[4] = (tdhr >> 0)  & 0xFF;
  if (read_size > 5) data[5] = (tdhr >> 8)  & 0xFF;
  if (read_size > 6) data[6] = (tdhr >> 16) & 0xFF;
  if (read_size > 7) data[7] = (tdhr >> 24) & 0xFF;

  CAN1->RF0R |= CAN_RF0R_RFOM0;

  return ((int32_t)read_size);
}

static int32_t ARM_CAN_Control (uint32_t control, uint32_t arg) {

  if (can_driver_powered == 0U) { return ARM_DRIVER_ERROR; }

  switch (control & ARM_CAN_CONTROL_Msk) {
    case ARM_CAN_ABORT_MESSAGE_SEND:
      // Add code to abort message pending to be sent
      // ..
      break;
    case ARM_CAN_SET_FD_MODE:
      // Add code to enable Flexible Data-rate mode
      // ..
      break;
    case ARM_CAN_SET_TRANSCEIVER_DELAY:
      // Add code to set transceiver delay
      // ..
      break;
    default:
      // Handle unknown control code
      return ARM_DRIVER_ERROR_UNSUPPORTED;
  }

  return ARM_DRIVER_OK;
}

static ARM_CAN_STATUS ARM_CAN_GetStatus (void) {

  // Add code to return device bus and error status
  // ..
  ARM_CAN_STATUS ret = {0};
  return ret;
}


void USB_HP_CAN1_TX_IRQHandler(void) {
    uint32_t tsr = CAN1->TSR;

    if (tsr & CAN_TSR_RQCP0) {
        CAN1->TSR |= CAN_TSR_RQCP0;
        if (CAN_SignalObjectEvent) {
          CAN_SignalObjectEvent(0, ARM_CAN_EVENT_SEND_COMPLETE);
        }
    }

    if (tsr & CAN_TSR_RQCP1) {
        CAN1->TSR |= CAN_TSR_RQCP1;
        if (CAN_SignalObjectEvent) {
            CAN_SignalObjectEvent(1, ARM_CAN_EVENT_SEND_COMPLETE);
        }
    }

    if (tsr & CAN_TSR_RQCP2) {
        CAN1->TSR |= CAN_TSR_RQCP2;
        if (CAN_SignalObjectEvent) {
            CAN_SignalObjectEvent(2, ARM_CAN_EVENT_SEND_COMPLETE);
        }
    }
}

void USB_LP_CAN1_RX0_IRQHandler(void) {
    if ((CAN1->RF0R & CAN_RF0R_FMP0) != 0) {

        if (CAN_SignalObjectEvent) {
            CAN_SignalObjectEvent(3, ARM_CAN_EVENT_RECEIVE);
        }
    }
}

// CAN driver functions structure

extern \
ARM_DRIVER_CAN Driver_CAN0;
ARM_DRIVER_CAN Driver_CAN0 = {
  ARM_CAN_GetVersion,
  ARM_CAN_GetCapabilities,
  ARM_CAN_Initialize,
  ARM_CAN_Uninitialize,
  ARM_CAN_PowerControl,
  ARM_CAN_GetClock,
  ARM_CAN_SetBitrate,
  ARM_CAN_SetMode,
  ARM_CAN_ObjectGetCapabilities,
  ARM_CAN_ObjectSetFilter,
  ARM_CAN_ObjectConfigure,
  ARM_CAN_MessageSend,
  ARM_CAN_MessageRead,
  ARM_CAN_Control,
  ARM_CAN_GetStatus
};