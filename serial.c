#include "serial.h"

#include <avr/io.h>

void serial_init(void)
{
    // Set baud rate (see datasheet §20.10)
    UBRR0 = 51; // 51 = 9600 baud at 8MHz with U2Xn=0

    // Enable TX
    UCSR0B = _BV(TXEN0);

    // Set frame format 8N1
    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
}

void serial_tx(char *data, uint8_t len)
{
    // TODO make this asynchronous by using TX interrupt

    while (len-- > 0)
    {
        while (!(UCSR0A & _BV(UDRE0)))
            ;

        UDR0 = *(data++);
    }
}
