#ifndef MEMOIRE_H
#define MEMOIRE_H

#include <stdint.h>

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

typedef uint8_t id_t[20];

typedef struct{
  uint8_t cred_id[16];
  uint8_t sk[21];
}eep_item_t;

#define EEP_MAXSIZE (1002/(sizeof(id_t)+sizeof(eep_item_t)))
#define eep_ids ((id_t *)22)
#define eep_items ((eep_item_t *)(22+EEP_MAXSIZE*sizeof(id_t)))

// on stocke en SRAM la bitmap, le compteur et les app_id
uint8_t sram_compteur[16];
uint8_t sram_bitmap[4];
id_t sram_ids[EEP_MAXSIZE];

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

// a appeler au debut du programme
void memoire_init(void);

void memoire_reset(void);

/*
  stocke la cle secrete en memoire.
  ecrit dans cred_id l'identifiant attribue.

  renvoie 0 si ok, sinon c'est qu'il n'y a plus de place

  quoi qu'il arrive, efface la cle secrete de la SRAM (mets des 0)
*/
int memoire_push(uint8_t sk[21], id_t, uint8_t cred_id[16]);

/*
  cherche l'entree qui correspond a l'id
  l'ecrit dans le pointeur

  ! il faut effacer la cle secrete de la SRAM quand plus besoin
  
  renvoie 0 si ok, sinon c'est que l'id n'a pas ete trouve.
*/
int memoire_get(id_t, eep_item_t *);

//////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////

// on definit des fonctions pour pouvoir lister les credentials efficacement

/*
  est egal a la postion du prochain item dans le parcours
  MEM_PARCOURS_FINI==parcours fini
*/
typedef uint8_t memoire_iterateur_t;
# define MEM_PARCOURS_FINI ((uint8_t)EEP_MAXSIZE)

void memoire_init_iterateur(memoire_iterateur_t *);

/*
  ecrit le prochain item dans le pointeur.
  ne fait rien si parcours fini
*/
void memoire_iterateur_next(memoire_iterateur_t *, eep_item_t *);

#endif //MEMOIRE_H
