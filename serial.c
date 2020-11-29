#include "serial.h"

#include <stddef.h>
#include <avr/io.h>
#include <avr/interrupt.h>

static volatile uint8_t *tx_data = NULL;
static volatile uint8_t tx_data_len = 0;
static volatile uint8_t tx_cur_byte = 0;

void serial_init(void)
{
    // Set baud rate (see datasheet §20.10)
    UBRR0 = 51; // 51 = 9600 baud at 8MHz with U2Xn=0

    // Enable TX
    UCSR0B = _BV(TXEN0);

    // Set frame format 8N1
    UCSR0C = _BV(UCSZ01) | _BV(UCSZ00);
}

void serial_tx(uint8_t data)
{
    while (!(UCSR0A & _BV(UDRE0)))
        ;

    UDR0 = data;
}

void serial_tx_buffer(uint8_t *data, uint8_t len)
{
    tx_data = data;
    tx_data_len = len;

    // Enable Data Register Empty interrupt (starts transmission)
    UCSR0B |= _BV(UDRIE0);
}

int serial_putc(char c, FILE *stream)
{
    serial_tx((uint8_t)c);
    return 0;
}

ISR(USART_UDRE_vect)
{
    UDR0 = *(tx_data++);

    // Disable Data Register Empty interrupt if last byte was sent
    if (--tx_data_len == 0)
        UCSR0B &= ~_BV(UDRIE0);
}