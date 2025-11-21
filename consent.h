#include <avr/io.h>
#include <inttypes.h>
#include <util/delay.h>

/*
  Fait clignoter la LED 10 secondes.
  Si bouton appuyé, on arrete le clignotement et on renvoie 1.
  Sinon on renvoie zero.
*/
uint8_t askConsent();

void delay_ms(uint16_t ms);
void init_io();
int button_is_pressed();
void toggle_led();
