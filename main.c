#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#include "ctap.h"
#include "memoire.h"
#include "alea.h"
#include "signature.h"
#include "consent.h"
#include <uart.h>

#define COMMAND_LIST_CREDENTIALS (0U)
#define COMMAND_MAKE_CREDENTIAL (1U)
#define COMMAND_GET_ASSERTION (2U)
#define COMMAND_RESET (3U)

#define STATUS_ERR_COMMAND_UNKNOWN (1U)

int main(void ){
    UART__init();
    memoire_init();
    signature_init();
    consent_init();
for (;;) {
  uint8_t cmd = usart_read_byte();
    switch (cmd) {
        case COMMAND_MAKE_CREDENTIAL: makecredential(); break;
        case COMMAND_GET_ASSERTION:   getassertion();   break;
        case COMMAND_LIST_CREDENTIALS: listcredentials(); break;
        case COMMAND_RESET: reset(); break;
        default: UART__putc(STATUS_ERR_COMMAND_UNKNOWN); break;
    }
}
return 0 ;
}
