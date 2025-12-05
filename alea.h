#ifndef ALEA_H
#define ALEA_H

#include <stdint.h>

/*
  quand l'utilisateur donne son consentement,
  on engregistre ici la valeur de l'horloge pour notre alea.
*/
void alea_collect_user(uint16_t);

// pareil quand on reçoit une requete du client
void alea_collect_client(uint16_t);

/*
  comme l'impose micro ecc,
  renvoie 1 si ok, 0 sinon
*/
int alea_gen(uint8_t *, unsigned);

#endif // ALEA_H
