#ifndef BSP_ENCODER_H
#define BSP_ENCODER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void BSP_Encoder_Init(void);
int16_t BSP_Encoder_GetDelta(void);
int32_t BSP_Encoder_GetTotalPosition(void);

#ifdef __cplusplus
}
#endif

#endif /* BSP_ENCODER_H */