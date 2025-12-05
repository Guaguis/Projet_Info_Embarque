#include <alea.h>
#include <blake2s.h>
#include <salt.h>
#include <string.h>
#include <avr/io.h>

/*
  0-31 : inputs utilisateur
  32-62 : inputs client
  63 : chaleur
  reste : rien
*/
static uint16_t toHash[64];
/*
  nombre d'inputs utilisateur deja enregistrees
  pour le prochain reseed
*/
static uint8_t hash_user=0;
// pareil client
static uint8_t hash_client=0;

static uint8_t alea[32];
// nombre d'octets d'alea deja consommes
static uint8_t alea_curseur=32;

void alea_collect_user(uint16_t x){
  if(hash_user>=32)
    return;

  toHash[hash_user]=x;
  hash_user++;
}

void alea_collect_client(uint16_t x){
  if(hash_client>=31)
    return;

  toHash[32|hash_client]=x;
  hash_client++;
}

static void collect_temperature(void){
  uint8_t oldPRR=PRR, oldDDRC=DDRC, oldPORTC=PORTC, oldADMUX=ADMUX, oldADCSRA=ADCSRA;
  PRR &= ~(1 << PRADC);
  DDRC &= ~1;  

  PORTC &= ~1;

  ADMUX = 1 << REFS0;  


  ADCSRA |= (1 << ADEN) | (1 << ADSC);
  while (ADCSRA & (1 << ADSC));

 
  ADCSRA |= (1 << ADSC);
  while (ADCSRA & (1 << ADSC));

  toHash[63]=ADC;

  // race condition possible ?
  PRR=oldPRR; DDRC=oldDDRC; PORTC=oldPORTC; ADMUX=oldADMUX; ADCSRA=oldADCSRA;
}

// 0==OK, sinon pas de source d'alea disponible
static int reseed(void){
  if(!hash_user||!hash_client)
    return -1; // a priori impossible

  collect_temperature();
  for(int i=0; i<8; i++){ // ajout sel
    toHash[8|i]^=salt[i<<1]|(salt[(i<<1)|1]);
  }
  compute_blake2s((uint8_t *)toHash, alea);
  
  alea_curseur=0;
  hash_user=0;
  hash_client=0;

  return 0;
}
//#include <consent.h>
#include <uart.h>
int alea_gen(uint8_t * dest, unsigned length){
  if(length>32){
    UART__putc(length);
    return 0;
  }
  if(length>(unsigned)(32-alea_curseur)){
    if(alea_curseur<32){
      memcpy(dest, alea+alea_curseur, 32-alea_curseur);
      length-=32-alea_curseur;
      dest+=32-alea_curseur;
      alea_curseur=32;
    }
    if(reseed()){
      //echec generation alea, mais on continue
      //return 0;
       return 1;
    }
  }
  memcpy(dest, alea+alea_curseur, length);
  alea_curseur+=length;
  return 1;
}
