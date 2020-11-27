#pragma once

#include <stdint.h>

void serial_init(void);
void serial_tx(char *data, uint8_t len);
