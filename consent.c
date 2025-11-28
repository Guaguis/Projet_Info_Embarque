//on utilise watch dog timer et power-down mode  pour moins de consommation 
#include<avr/wdt.h>
#include<avr/intterupt.h>
#include <avr/sleep.h>
#include <stdint.h>
#include <avr/io.h>

void setup_watchdog_for_16ms(){
   cli(); // disable global interrupts 
   WDTCSR|=0x18; // enable wdt change
   WDTCSR=0x40; // 16ms

  // power-down mode
  SMCR=4;
   sei(); //reactiver les interruptions 
 }

ISR(WDT_vect){}
DDRD &= ~_BV(PD2);  // PD2 en entrée
PORTD |= _BV(PD2);

volatile uint8_t button_state = 1;     // bouton sur pull-up
volatile uint8_t counter = 0;
volatile uint8_t button_pushed_flag = 0;

void debounce() {
    uint8_t current = (PIND & _BV(PD2)) ? 1 : 0;

    if (current != button_state) {
        counter++;
        if (counter >= 4) {
            button_state = current;
            counter = 0;

            if (button_state == 0) {    // appui détecté (active LOW)
                button_pushed_flag = 1;
            }
        }
    } else {
        counter = 0;
    }
}


uint8_t() wait_for_consent(){
setup_watchdog_for_16ms();
sleep_enable();
button_flag=0;
uint16_t tick=0;
//16ms*256=10s
  while(tick<625){
    sleep_cpu();
    debounce();
    if (button_pushed==1){
        sleep_disable();
        return 1;
    
    }
    cligonter_led_500ms(); //cligoneter la LED toute les 500 ms 
    tick++;   
  } 
sleep_disable();
return 0; //temps écoulé 
  
}
