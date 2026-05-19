#ifndef BSP_HBRIDGE_H
#define BSP_HBRIDGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void BSP_Motor_Init(void);
void BSP_Motor_SetSpeed(int8_t speed_percent);

#ifdef __cplusplus
}
#endif

#endif /* BSP_HBRIDGE_H */