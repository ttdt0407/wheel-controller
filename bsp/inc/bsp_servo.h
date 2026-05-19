#ifndef BSP_SERVO_H
#define BSP_SERVO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void BSP_Servo_Init(void);
void BSP_Servo_SetAngle(uint8_t angle_degrees);

#ifdef __cplusplus
}
#endif

#endif /* BSP_SERVO_H */