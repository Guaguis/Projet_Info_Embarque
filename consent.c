//on utilise watch dog timer et power-down mode  pour moins de consommation 
#include<avr/wdt.h>
#include<avr/intterupt.h>
#include <avr/sleep.h>

void setup_watchdog_for_16ms(){
   cli(); // disable global interrupts 
   WDTCSR|=0x18; // enable wdt change
   WDTCSR=0x40; // 16ms

  // power-down mode
  SMCR=4;
   sei(); //reactiver les interruptions 
 }

ISR(WDT_vect){}


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
