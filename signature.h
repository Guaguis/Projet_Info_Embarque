#ifndef SIGNATURE_H
#define SIGNATURE_H
#include <stdint.h>

// a appeler au debut du programme
void signature_init(void);

// 1==ok, 0==erreur
int signature_keygen(uint8_t pk[40], uint8_t sk[21]);
int signature_sign(uint8_t sk[21], uint8_t msg[20], uint8_t signature[40]);

#endif // SIGNATURE_H
