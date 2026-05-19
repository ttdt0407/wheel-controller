/**
 * @file bsp_console.h
 * @brief BSP Console API
 */

#ifndef BSP_CONSOLE_H_
#define BSP_CONSOLE_H_

#ifdef __cplusplus
extern "C" {
#endif

void bsp_log_init(void);
void bsp_log_printf(const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif /* BSP_CONSOLE_H_ */
