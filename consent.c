//on utilise watch dog timer et power-down mode  pour moins de consommation 
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdint.h>
#include <avr/io.h>
#include <consent.h>
#include <alea.h>

static void setup_watchdog_for_16ms(void){
   cli(); // disable global interrupts 
   WDTCSR|=0x18; // enable wdt change
   WDTCSR=0x40; // 16ms

   // power-down mode
   SMCR=4;
   sei(); //reactiver les interruptions 
 }

ISR(WDT_vect){}

void consent_init(void){
  DDRD &= ~_BV(DDD2);  // PD2 en entrée
  PORTD |= _BV(PORTD2); // pour le bouton
  
  DDRB = _BV(DDB5); // pour la LED
  PORTB = 0;

  TCCR1B=_BV(CS10); // horloge clock1 pour generation alea
}

volatile uint8_t button_state = 1;     // bouton sur pull-up
volatile uint8_t counter = 0;
volatile uint8_t counter_LED = 0;
volatile uint8_t button_pushed_flag = 0;

static void debounce(void) {
    uint8_t current = (PIND & _BV(PIND2)) ? 1 : 0;

    counter_LED++;
    if(counter_LED>=31){ // 496ms, approximatif
      counter_LED=0;
      PORTB^=_BV(PORTB5);
    }

    if (current != button_state) {
        counter++;
        if (counter >= 4) {
            button_state = current;
            counter = 0;
	    counter_LED=0;

            if (button_state == 0) {    // appui détecté (active LOW)
                button_pushed_flag = 1;
            }
        }
    } else {
      counter = 0;
    }
}

uint8_t wait_for_consent(){
setup_watchdog_for_16ms();
sleep_enable();
button_pushed_flag=0;
uint16_t tick=0;
//16ms*625=10s
  while(tick<625){
    sleep_cpu();
    debounce();
    if (button_pushed_flag==1){
        sleep_disable();
	alea_collect_user(TCNT1);
	PORTB&=~_BV(PORTB5);
        return 1;
    
    }
    tick++;   
  }
sleep_disable();
return 0; //temps écoulé 
  
}
