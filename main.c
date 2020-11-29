#include <string.h>
#include <stdio.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "serial.h"
#include "radio.h"

static FILE serial_stdout = FDEV_SETUP_STREAM(serial_putc, NULL, _FDEV_SETUP_WRITE);

int main(void)
{
    // char *str = "Hello, World!\r\n";

    serial_init();
    stdout = &serial_stdout;

    radio_init();

    sei();

    while (1)
    {
        // serial_tx((uint8_t *)str, strlen(str));
        // printf("printf %d %X\n", -45, 0x45af);

        // _delay_ms(1000);
    }
}
