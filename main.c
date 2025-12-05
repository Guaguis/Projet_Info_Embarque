#include <avr/io.h>
#include <util/delay.h>
#include <stdint.h>

#include "ctap.h"
#include "memoire.h"
#include "alea.h"
#include "signature.h"
#include "consent.h"

#define STATUS_ERR_COMMAND_UNKNOWN 1

int main(void ){
    UART__init();
    memoire_init();
    signature_init();
    consent_init();
    
for (;;) {
    uint8_t cmd = UART__getc();
    switch (cmd) {
        case COMMAND_MAKE_CREDENTIAL: makecredential(); break;
        case COMMAND_GET_ASSERTION:   getassertion();   break;
        case COMMAND_LIST_CREDENTIALS: listcredentials(); break;
        case COMMAND_RESET: reset(); break;
        default: serial_write_byte(STATUS_ERR_COMMAND_UNKNOWN); break;
    }
}
return 0 ;
}
