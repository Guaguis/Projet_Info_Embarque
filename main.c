#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdint.h>

#include "uart.h"
#include "ctap.h"
#include "memoire.h"
#include "signature.h"

#define COMMAND_LIST_CREDENTIALS   0
#define COMMAND_MAKE_CREDENTIAL    1
#define COMMAND_GET_ASSERTION      2
#define COMMAND_RESET              3

#define STATUS_ERR_COMMAND_UNKNOWN 1

int main(void) {
    UART__init();
    memoire_init();
    signature_init();
    TCCR1A = 0;
    TCCR1B |= _BV(CS11) | _BV(CS10);   // prescaler 64
    sei();

    //  (CPU idle)
    set_sleep_mode(SLEEP_MODE_IDLE);
    sleep_enable();

    for (;;) {
        
        sleep_cpu();

        uint8_t cmd;
        if (!uart_read_from_buffer(&cmd)) {
            switch (cmd) {
                case COMMAND_MAKE_CREDENTIAL:
                    makecredential();
                    break;

                case COMMAND_GET_ASSERTION:
                    getassertion();
                    break;

                case COMMAND_LIST_CREDENTIALS:
                    listcredentials();
                    break;

                case COMMAND_RESET:
                    reset();
                    break;

                default:
                    UART__putc(STATUS_ERR_COMMAND_UNKNOWN);
                    break;
            }
        }
        .
    }

    return 0;
}

