#ifndef SALT_H
#define SALT_H
#include <stdint.h>

// pour la generation d'alea et les cred_id
extern uint8_t salt[16];

void incremente_compteurs(void);

#endif // SALT_H
