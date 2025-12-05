#include <avr/io.h>
#include <inttypes.h>
#include <util/delay.h>

/*
  Fait clignoter la LED 10 secondes.
  Si bouton appuyé, on arrete le clignotement et on renvoie 1.
  Sinon on renvoie zero.
*/
uint8_t wait_for_consent(void);

void consent_init(void);
