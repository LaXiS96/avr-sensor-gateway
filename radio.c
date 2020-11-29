#include "radio.h"

#include <avr/io.h>
#include <avr/interrupt.h>

#include "serial.h"

#define RADIO_SYNC_COUNT_MIN (4 * 2) // Minimum number of SYNC conditions to start receiving
#define MAX_DATA_LENGTH 16

// Timer counts at 1MHz
#define RX_1T 28
#define RX_TOLERANCE 6

#define RX_1T_MIN (RX_1T - RX_TOLERANCE)
#define RX_1T_MAX (RX_1T + RX_TOLERANCE)
#define RX_2T (RX_1T * 2)
#define RX_2T_MIN (RX_2T - RX_TOLERANCE * 2)
#define RX_2T_MAX (RX_2T + RX_TOLERANCE * 2)
#define RX_3T (RX_1T * 3)
#define RX_3T_MIN (RX_3T - RX_TOLERANCE * 3)
#define RX_3T_MAX (RX_3T + RX_TOLERANCE * 3)

#define DEBUG_PIN1_HIGH() (PORTD |= _BV(PD7))
#define DEBUG_PIN1_LOW() (PORTD &= ~_BV(PD7))
#define DEBUG_PIN2_HIGH() (PORTD |= _BV(PD6))
#define DEBUG_PIN2_LOW() (PORTD &= ~_BV(PD6))

typedef enum
{
    RX_SYNC,
    RX_DATA,
    RX_STOP,
} radio_rx_state_t;
static volatile radio_rx_state_t rx_state = RX_SYNC;

static volatile uint8_t data[MAX_DATA_LENGTH];

void radio_init(void)
{
    DDRD |= _BV(PD6) | _BV(PD7); // TODO DEBUG

    // Set up Timer in normal mode
    TCCR0B = 0b00000010; // start, prescaler 8

    // Set up Pin Change interrupt on PCINT0/PB0
    PCICR = _BV(PCIE0);
    PCMSK0 = _BV(PCINT0);
}

static void sample_data(uint8_t *byte, uint8_t *bit)
{
    uint8_t value = PINB & _BV(PCINT0);

    if (value) // TODO DEBUG
    {
        DEBUG_PIN1_HIGH();
        DEBUG_PIN1_LOW();
    }
    else
    {
        DEBUG_PIN2_HIGH();
        DEBUG_PIN2_LOW();
    }

    data[*byte] |= value << (7 - *bit);
    if (++(*bit) == 8)
    {
        *bit = 0;
        if (++(*byte) == MAX_DATA_LENGTH)
            *byte = 0;
    }
}

ISR(PCINT0_vect)
{
    static uint8_t sync_count = 0;
    static uint8_t data_byte = 0;
    static uint8_t data_bit = 0;
    static uint8_t ignore_1t = 0;

    uint8_t elapsed = TCNT0;
    TCNT0 = 0;

    uint8_t t_count = 0; // 0=invalid, 1=T, 2=2T, 3=3T
    if (elapsed >= RX_1T_MIN && elapsed <= RX_1T_MAX)
        t_count = 1;
    else if (elapsed >= RX_2T_MIN && elapsed <= RX_2T_MAX)
        t_count = 2;
    else if (elapsed >= RX_3T_MIN && elapsed <= RX_3T_MAX)
        t_count = 3;

    uint8_t need_resync = 0;
    switch (rx_state)
    {
    case RX_SYNC:
        // Stay in SYNC state until at least RADIO_SYNC_COUNT_MIN conditions were received
        if (t_count == 3)
        {
            sync_count++;
            break;
        }
        else if (t_count == 0 || sync_count < RADIO_SYNC_COUNT_MIN)
        {
            need_resync = 1;
            break;
        }

        rx_state = RX_DATA;
        // Fall-through to RX_DATA
    case RX_DATA:
        if (t_count == 1)
        {
            // 1T received, sample first edge and ignore next 1T
            if (ignore_1t)
                ignore_1t = 0;
            else
            {
                sample_data(&data_byte, &data_bit);
                ignore_1t = 1;
            }
        }
        else if (t_count == 2)
        {
            // 2T received, always sample
            sample_data(&data_byte, &data_bit);
        }
        else
            need_resync = 1;
        break;
    case RX_STOP:

        break;
    default:
        need_resync = 1;
    }

    if (need_resync)
    {
        if (data_byte != 0)
            serial_tx_buffer((uint8_t *)data, MAX_DATA_LENGTH);

        sync_count = 0;
        data_byte = 0;
        data_bit = 0;
        ignore_1t = 0;
        rx_state = RX_SYNC;
    }
}
