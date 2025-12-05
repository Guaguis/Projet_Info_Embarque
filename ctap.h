#ifndef CTAP_H
#define CTAP_H
#include <stdint.h>
/*
 * Implémentation des 4 commandes pour la communication suivant  CTAP  :
 * - MakeCredential
 * - GetAssertion
 * - ListCredentials
 * - Reset
 * Ces fonctions doivent être appelées après lecture du byte de commande dans le main
 */

void makecredential(void);
void getassertion(void); 
void listcredentials(void);
void reset(void); 
#endif // CTAP_H
