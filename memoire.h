#ifndef MEMOIRE_H
#define MEMOIRE_H

#include <stdint.h>

typedef uint8_t id_t[20];

typedef struct{
  uint8_t cred_id[16];
  uint8_t sk[21];
}eep_item_t;

// nombre de credentials qu'on peut stocker
#define EEP_MAXSIZE (1002/(sizeof(id_t)+sizeof(eep_item_t)))

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

#ifdef DEBUG
/*
  cherche l'entree qui correspond a l'id
  l'ecrit dans le pointeur

  ! il faut effacer la cle secrete de la SRAM quand plus besoin
  
  renvoie 0 si ok, sinon c'est que l'id n'a pas ete trouve.
*/
int memoire_get(id_t, eep_item_t *);
#endif // DEBUG

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
  ecrit le prochain item et id dans les pointeurs.
  ne fait rien si parcours fini
*/
void memoire_iterateur_next(memoire_iterateur_t *, eep_item_t *, id_t);

uint8_t memoire_count(void);

#endif //MEMOIRE_H
