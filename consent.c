//on utilise watch dog timer pour moins de consommation 
#include<avr/wdt.h>
#include<avr/intterupt.h>
#include <avr/sleep.h>



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
