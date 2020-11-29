#pragma once

#include <stdint.h>
#include <stdio.h>

/**
 * Initialize Serial peripheral.
 * Remember to enable global interrupts afterwards.
 */
void serial_init(void);

/** Transmit byte, busy blocking */
void serial_tx(uint8_t data);

/** Transmit raw buffer using interrupts */
void serial_tx_buffer(uint8_t *data, uint8_t len);

/** putc implementation for printf */
int serial_putc(char c, FILE *stream);
