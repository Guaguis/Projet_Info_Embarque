#include <memoire.h>
#include <avr/eeprom.h>
#include <string.h>

static int const IDS_SIZE=EEP_MAXSIZE*sizeof(id_t);

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
  eeprom_read_block(sram_ids, eep_ids, IDS_SIZE);
}

void memoire_reset(void){
  memset(sram_bitmap, 0, 4);
  memset(sram_ids, 0xff, IDS_SIZE);
  eeprom_update_block(sram_bitmap, eep_bitmap, 4);

  // un memset improvise version eeprom
  uint8_t * i=(uint8_t *)eep_ids;
  for(; i<(uint8_t *)1024; i+=IDS_SIZE){
    eeprom_update_block(sram_bitmap, i, IDS_SIZE);
  }
  eeprom_update_block(sram_bitmap, i, ((uint8_t *)1024)-i);
}

static void incremente_compteurs(void){
  int i=-1;
  do{
    i++;
    sram_compteur[i]++;
  }while(i<16 && !sram_compteur[i]);
  
  eeprom_update_block(sram_compteur, eep_compteur, 16);
}

int memoire_push(uint8_t sk[21], id_t id, uint8_t cred_id[16]){
  // position ou on va ecrire dans l'eep_items
  int pos=-1;
  
  // recherche de l'id dans les ids deja enregistres
  for(unsigned i=0; i<EEP_MAXSIZE; i++){
    if( ((sram_bitmap[i>>3]>>(i&7))&1) && !memcmp(id, sram_ids[i], sizeof(id_t)) ){
      pos=i;
      break;
    }
  }

  if(pos==-1){
    // recherche d'une position libre
    for(unsigned i=0; i<EEP_MAXSIZE; i++){
      if(!((sram_bitmap[i>>3]>>(i&7))&1)){ // position libre
	pos=i;
	sram_bitmap[i>>3]|=1U<<(i&7); // mise a jour bitmap

	// on ecrit l'id et la bitmap, et on ecrira l'item apres
	memcpy(sram_ids, id, sizeof(id_t));
	eeprom_update_block(sram_bitmap, eep_bitmap, 4);
	id_t * dst=eep_ids+i;
	eeprom_update_block(id, dst, sizeof(id_t));
	break;
      }
    }
  }

  // pas de place
  if(pos==-1){
    memset(sk, 0, 21);
    return -1;
  }

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

int memoire_get(id_t id, eep_item_t * dst){
  // position de l'id dans le tableau des ids
  int pos=-1;
  
  // recherche de l'id
  for(unsigned i=0; i<EEP_MAXSIZE; i++){
    if( ((sram_bitmap[i>>3]>>(i&7))&1) && !memcmp(id, sram_ids[i], sizeof(id_t)) ){
      pos=i;
      break;
    }
  }

  if(pos==-1)
    return -1;

  eep_item_t * src=eep_items+pos;
  eeprom_read_block(&dst, src, sizeof(eep_item_t));

  return 0;
}

void memoire_init_iterateur(memoire_iterateur_t * i){
  for(*i=0; *i<EEP_MAXSIZE && !(sram_bitmap[*i>>3]>>(*i&7))&1; (*i)++){}
}

void memoire_iterateur_next(memoire_iterateur_t * i, eep_item_t * dst){
  eep_item_t * src=eep_items+*i;
  eeprom_read_block(dst, src, sizeof(eep_item_t));

  // incrementation de l'iterateur
  for(; *i<EEP_MAXSIZE && !(sram_bitmap[*i>>3]>>(*i&7))&1; (*i)++){}
}
