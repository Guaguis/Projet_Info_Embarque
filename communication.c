#include <avr/io.h>
#include <stdint.h>
#include <uart.h>

uint8_t usart_read_byte() {
    while (!(UCSR0A & _BV(RXC0))) {
        ; // attendre un octet
    }
    return UDR0;
}
uint8_t cmd = uart_get_byte();

switch(cmd) {
    case COMMAND_MAKE_CREDENTIAL:
        make_credential();
        break;

    case COMMAND_GET_ASSERTION:
        get_assertion();
        break;

    case COMMAND_LIST_CREDENTIALS:
        list_credentials();
        break;

    case COMMAND_RESET:
        reset();
        break;
}
