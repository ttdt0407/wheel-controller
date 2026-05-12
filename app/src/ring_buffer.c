#include "ring_buffer.h"

void uart_tx_buffer_init(UART_TX_Buffer_t* tx, uint8_t* storage, uint16_t size) {
    if ((tx == 0) || (storage == 0) || (size == 0U)) {
        return;
    }

    tx->buffer = storage;
    tx->size = size;
    tx->head = 0U;
    tx->tail = 0U;
}

bool uart_tx_buffer_is_empty(const UART_TX_Buffer_t* tx) {
    if (tx == 0) {
        return true;
    }

    return (tx->head == tx->tail);
}

bool uart_tx_buffer_is_full(const UART_TX_Buffer_t* tx) {
    if (tx == 0) {
        return true;
    }

    if (tx->size == 0U) {
        return true;
    }

    return (((tx->tail + 1U) % tx->size) == tx->head);
}

bool uart_tx_buffer_put(UART_TX_Buffer_t* tx, uint8_t c) {
    uint16_t next_tail;

    if ((tx == 0) || (tx->buffer == 0) || (tx->size == 0U)) {
        return false;
    }

    next_tail = (uint16_t)(tx->tail + 1U);
    if (next_tail >= tx->size) {
        next_tail = 0U;
    }

    if (next_tail == tx->head) {
        return false;
    }

    tx->buffer[tx->tail] = c;
    tx->tail = next_tail;

    return true;
}

bool uart_tx_buffer_get(UART_TX_Buffer_t* tx, uint8_t* c) {
    if ((tx == 0) || (c == 0) || (tx->buffer == 0) || (tx->size == 0U)) {
        return false;
    }

    if (tx->head == tx->tail) {
        return false;
    }

    *c = tx->buffer[tx->head];
    tx->head = (uint16_t)(tx->head + 1U);
    if (tx->head >= tx->size) {
        tx->head = 0U;
    }

    return true;
}