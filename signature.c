#include <uECC.h>
#include <alea.h>
#include <signature.h>

void signature_init(void){
  uECC_set_rng(alea_gen);
}

int signature_keygen(uint8_t pk[40], uint8_t sk[21]){
  return uECC_make_key(pk, sk, uECC_secp160r1());
}

int signature_sign(uint8_t sk[21], uint8_t msg[20], uint8_t signature[40]){
  return uECC_sign(sk, msg, 20, signature, uECC_secp160r1());
}
