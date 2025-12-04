#include <stdint.h>
#include <string.h>
#include "memoire.h"
#include "uECC.h"   // bibliothèque micro-ecc
#include "consent.h"
// Constantes de statut (cf sujet)
#define STATUS_OK                 0
#define STATUS_ERR_COMMAND_UNKNOWN 1
#define STATUS_ERR_CRYPTO_FAILED  2
#define STATUS_ERR_BAD_PARAMETER  3
#define STATUS_ERR_NOT_FOUND      4
#define STATUS_ERR_STORAGE_FULL   5
#define STATUS_ERR_APPROVAL       6

// ================= UART =================

// Lis exactement len octets depuis l'UART (bloquant)
static void serial_read(uint8_t *buf, uint8_t len) {
    for (uint8_t i = 0; i < len; i++) {
        buf[i] = usart_read_byte();   
    }
}

uint8_t serial_read_exact(uint8_t *buf, uint8_t len) {
    for (uint8_t i = 0; i < len; i++) {
        if (!uart_available()) {  
            return 0;
        }
        buf[i] = usart_read_byte();
    }
    return 1;
}

// Envoie exactement len octets sur l'UART
static void serial_write(const uint8_t *buf, uint8_t len) {
    for (uint8_t i = 0; i < len; i++) {
        UART__putc(buf[i]);           //
    }
}

static void serial_write_byte(uint8_t b) {
    UART__putc(b);                    // 
}



void ctap_makecredential(void) {
    id_t hashed_app_id;          // 20 octets
    uint8_t priv_key20[20];      // clé privée micro-ecc (20 octets)
    uint8_t sk[21];              // ce qu'on stocke en EEPROM
    uint8_t pub_key[40];         // clé publique (X||Y)
    uint8_t cred_id[16];         // credential_id généré
    const struct uECC_Curve_t *curve = uECC_secp160r1();
    int ret;
    alea_collect_client(TCNT1);

    // Vérifier que nous avons reçu les 20 octets du hash
        if (!serial_read_exact(hashed_app_id, 20)) {
            serial_write_byte(STATUS_ERR_BAD_PARAMETER);
         return;
        }

    //  Demande de consentement utilisateur (LED qui clignote + bouton)
    if (!wait_for_consent()) {
        // Timeout / refus : renvoyer STATUS_ERR_APPROVAL
        serial_write_byte(STATUS_ERR_APPROVAL);
        return;
    }
    alea_collect_user(TCNT1);
    //  Générer une nouvelle paire de clés ECDSA (secp160r1)
    if (!uECC_make_key(pub_key, priv_key20, curve)) {
        // Erreur crypto
        serial_write_byte(STATUS_ERR_CRYPTO_FAILED);
        return;
    }

    // Préparer sk[21] pour la mémoire :
    //    on choisit de mettre un octet de padding devant (0) pour respecter le format du projet . 
    sk[0] = 0;
    memcpy(&sk[1], priv_key20, 20);

    //  Stocker (hashed_app_id, cred_id, sk) en EEPROM
    ret = memoire_push(sk, hashed_app_id, cred_id);
    // memoire_push efface déjà sk en SRAM quoi qu’il arrive :contentReference[oaicite:2]{index=2}

    if (ret != 0) {
        // Pas de place (pos==-1) -> STATUS_ERR_STORAGE_FULL
        serial_write_byte(STATUS_ERR_STORAGE_FULL);
        // On nettoie la clé privée temporaire
        memset(priv_key20, 0, sizeof(priv_key20));
        return;
    }

    //  Succès : envoyer STATUS_OK + le couple  (credential_id ,clé publique) 
    serial_write_byte(STATUS_OK);
    serial_write(cred_id, 16);
    serial_write(pub_key, 40);

    //  Effacer la clé privée en RAM
    memset(priv_key20, 0, sizeof(priv_key20));
}




