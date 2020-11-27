#include <string.h>
#include <util/delay.h>

#include "serial.h"

int main(void)
{
    char *str = "Hello, World!\r\n";

    serial_init();

    while (1)
    {
        serial_tx(str, strlen(str));

        _delay_ms(1000);
    }
}
