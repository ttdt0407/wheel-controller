#ifndef RING_BUFFER_H_
#define RING_BUFFER_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdbool.h>
#include <stdint.h>

typedef struct {
	uint8_t* buffer;
	uint16_t size;
	volatile uint16_t head;
	volatile uint16_t tail;
} UART_TX_Buffer_t;

void uart_tx_buffer_init(UART_TX_Buffer_t* tx, uint8_t* storage, uint16_t size);
bool uart_tx_buffer_is_empty(const UART_TX_Buffer_t* tx);
bool uart_tx_buffer_is_full(const UART_TX_Buffer_t* tx);
bool uart_tx_buffer_put(UART_TX_Buffer_t* tx, uint8_t c);
bool uart_tx_buffer_get(UART_TX_Buffer_t* tx, uint8_t* c);

#ifdef __cplusplus
}
#endif

#endif /* RING_BUFFER_H_ */