#ifndef BSP_CAN_H
#define BSP_CAN_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t id;
    uint8_t  data[8];
    uint8_t  dlc;
    uint8_t  isExt;
    uint8_t  isRTR;
    uint8_t  reserved;
} CAN_Message_t;

void BSP_CAN_Init(void);
bool BSP_CAN_Write(CAN_Message_t *msg, uint32_t timeout_ms);
bool BSP_CAN_Read(CAN_Message_t *msg, uint32_t timeout_ms);

#endif /* BSP_CAN_H */