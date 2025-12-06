#include <memoire.h>
#include <avr/eeprom.h>
#include <string.h>
#include <salt.h>

uint8_t salt[16];

/*
  on manipulera directement les adresses numeriques
  on ne fait pas confiance a gcc
  pour nous attribuer les bonnes adresses,
  car on va remplir presque toute la memoire.
*/

/*
  ordre dans la memoire :
  eep_init, eep_compteur, eep_bitmap, eep_ids, eep_items
*/

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

/*
  l'octet eep_init dans l'EEPROM nous permet de verifier
  si la memoire a deja ete initialisee.
  Si oui, l'octet est egal a EEP_INIT.
  Sinon normalement le bootloader ecrit 0xff dedans.
*/
#define EEP_INIT (0x5a)
#define eep_init ((uint8_t *)1)

/*
  compteur sur 128 bits pour les cred_id
  little endian
*/
#define eep_compteur ((uint8_t *)2)

/*
  on stocke dans l'eeprom des triplets de la forme
  (app_id, cred_id, sk)
  on mets tous les app_id dans un seul tableau eep_ids
  (pour reduire le temps d'initialisation :
  un seul appel appel a eeprom_read_block)
  puis on mets les couples (cred_id, sk)
  dans un autre tableau eep_items
  une bitmap eep_bitmap nous indique
  quelles sont les positions allouees des tableaus
*/

/*
  bitmap sur quatre octets
  i-eme bit de la bitmap==(i%8)-eme bit du (i/8)-eme octet
  i eme bit a 1==i eme cle allouee
  bourrage de zeros a la fin
*/
#define eep_bitmap ((uint8_t *)18)

#define eep_ids ((id_t *)22)
#define eep_items ((eep_item_t *)(22+EEP_MAXSIZE*sizeof(id_t)))

// on stocke en SRAM la bitmap, le compteur et les app_id
#define sram_compteur (salt)
static uint8_t sram_bitmap[4];

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

void memoire_init(void){
  uint8_t init=eeprom_read_byte(eep_init);
  if(init!=EEP_INIT){ // premier allumage : tout initialiser
    memset(sram_compteur, 0, 16);
    memset(sram_bitmap, 0, 4);
    
    eeprom_update_byte(eep_init, EEP_INIT);
    eeprom_update_block(sram_compteur, eep_compteur, 16);
    eeprom_update_block(sram_bitmap, eep_bitmap, 4);
    return;
  }

  eeprom_read_block(sram_compteur, eep_compteur, 16);
  eeprom_read_block(sram_bitmap, eep_bitmap, 4);
}

void memoire_reset(void){
  eep_item_t zero;
  memset(zero.sk, 0, 21);
  memset(zero.cred_id, 0, 16);
  // un memset improvise version eeprom
  for(uint8_t i=0; i<EEP_MAXSIZE; i++){
    if((sram_bitmap[i>>3]>>(i&7))&1U)
      eeprom_update_block(&zero, eep_items+i, sizeof(eep_item_t));
  }

  memset(sram_bitmap, 0, 4);
  eeprom_update_block(sram_bitmap, eep_bitmap, 4);
  // pas besoin de nettoyer les ids si on nettoie la bitmap et les secrets
}

static void incremente_compteurs(void){
  int i=-1;
  do{
    i++;
    sram_compteur[i]++;
  }while(i<16 && !sram_compteur[i]);
  
  eeprom_update_block(sram_compteur, eep_compteur, 16);
}
#include <consent.h>
int memoire_push(uint8_t sk[21], id_t id, uint8_t cred_id[16]){
  // position ou on va ecrire dans l'eep_items
  int pos=-1;
  
  // recherche de l'id dans les ids deja enregistres
  for(uint8_t i=0; i<EEP_MAXSIZE; i++){
    if((sram_bitmap[i>>3]>>(i&7))&1){ // position occupee
      id_t cle;
      id_t * src=eep_ids+i;
      eeprom_read_block(cle, src, 20);
      if(!memcmp(id, cle, sizeof(id_t))){
	pos=i;
	break;
      }
    } else if(pos==-1)
      pos=i;
  }

  // pas de place
  if(pos==-1){
    memset(sk, 0, 21);
    return -1;
  }

  if(!((sram_bitmap[pos>>3]>>(pos&7))&1)){ // l'id n'etait pas stocke
     sram_bitmap[pos>>3]|=1U<<(pos&7); // mise a jour bitmap
     eeprom_update_block(sram_bitmap, eep_bitmap, 4);
     id_t * dst=eep_ids+pos;
     eeprom_update_block(id, dst, sizeof(id_t));
  }

  /*if(pos==-1){
    // recherche d'une position libre
    for(unsigned i=0; i<EEP_MAXSIZE; i++){
      if(!((sram_bitmap[i>>3]>>(i&7))&1)){ // position libre
	pos=i;
	sram_bitmap[i>>3]|=1U<<(i&7); // mise a jour bitmap
	
	// on ecrit l'id et la bitmap, et on ecrira l'item apres
	memcpy(sram_ids+i, id, sizeof(id_t));
	eeprom_update_block(sram_bitmap, eep_bitmap, 4);
	id_t * dst=eep_ids+i;
	eeprom_update_block(id, dst, sizeof(id_t));
	break;
      }
    }
    }*/

  memcpy(cred_id, sram_compteur, 16);
  incremente_compteurs();
  
  eep_item_t src, * dst=eep_items+pos;
  memcpy(src.cred_id, cred_id, 16);
  memcpy(src.sk, sk, 21);
  eeprom_update_block(&src, dst, sizeof(eep_item_t));
  
  memset(sk, 0, 21);
  memset(src.sk, 0, 21);
  
  return 0;
}
#include <util/delay.h>
#include <avr/io.h>
int memoire_get(id_t id, eep_item_t * dst){
  // position de l'id dans le tableau des ids
  int pos=-1;

  // recherche de l'id
  for(uint8_t i=0; i<EEP_MAXSIZE; i++){
    if((sram_bitmap[i>>3]>>(i&7))&1){ // position occupee
      id_t cle;
      id_t * src=eep_ids+i;
      eeprom_read_block(cle, src, 20);
      if(!memcmp(id, cle, sizeof(id_t))){
	pos=i;
	break;
      }
    }
  }

  if(pos==-1)
    return -1;

  eep_item_t * src=eep_items+pos;
  eeprom_read_block(&dst, src, sizeof(eep_item_t));

  PORTB|=_BV(PORTB5);
    _delay_ms(4000);
    PORTB^=_BV(PORTB5);
    _delay_ms(4000);
  return 0;
}

void memoire_init_iterateur(memoire_iterateur_t * i){
  for(*i=0; *i<EEP_MAXSIZE && !((sram_bitmap[*i>>3]>>(*i&7))&1); (*i)++){}
}

void memoire_iterateur_next(memoire_iterateur_t * i, eep_item_t * dst, id_t id_dst){
  if(*i==MEM_PARCOURS_FINI)
    return;

  id_t * id_src=eep_ids+*i;
  eep_item_t * src=eep_items+*i;

  eeprom_read_block(id_dst, id_src, 20);
  eeprom_read_block(dst, src, sizeof(eep_item_t));

  // incrementation de l'iterateur
  (*i)++;
  for(; *i<EEP_MAXSIZE && !((sram_bitmap[*i>>3]>>(*i&7))&1); (*i)++){}
}

uint8_t memoire_count(void){
  uint8_t ans=0;
  for(uint8_t i=0; i<EEP_MAXSIZE; i++)
    if((sram_bitmap[i>>3]>>(i&7))&1)
      ans++;
  return ans;
}
