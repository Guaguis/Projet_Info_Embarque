#include <stdint.h>
#include <string.h>
#include "memoire.h"
#include "uECC.h"   
#include "consent.h"
#include "uart.h"
#include "ctap.h"



// Constantes de statut 
#define STATUS_OK                 0
#define STATUS_ERR_COMMAND_UNKNOWN 1
#define STATUS_ERR_CRYPTO_FAILED  2
#define STATUS_ERR_BAD_PARAMETER  3
#define STATUS_ERR_NOT_FOUND      4
#define STATUS_ERR_STORAGE_FULL   5
#define STATUS_ERR_APPROVAL       6

// ================= UART =================

// on lit  exactement len octets depuis l'UART 
static void serial_read(uint8_t *buf, uint8_t len) {
    for (uint8_t i = 0; i < len; i++) {
        buf[i] = usart_read_byte();   // 
    }
}

uint8_t serial_read_exact(uint8_t *buf, uint8_t len) {
    for (uint8_t i = 0; i < len; i++) {
        if (!uart_available()) {  // 
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



void makecredential(void) {
    id_t hashed_app_id;          // 20 octets
    uint8_t priv_key20[20];      // clé privée micro-ecc (20 octets)
    uint8_t sk[21];              // ce qu'on stocke en EEPROM
    uint8_t pub_key[40];         // clé publique (X||Y)
    uint8_t cred_id[16];         // credential_id généré
    const struct uECC_Curve_t *curve = uECC_secp160r1();
    int ret;
    alea_collect_client(TCNT1);

    // Vérifier que nous avons reçu les 20 octets du hash
        if (!serial_read(hashed_app_id, 20)) {
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

    //  Succès : envoyer STATUS_OK + le couple  (credential_id + clé publique) : Makecredentialresponse
    serial_write_byte(STATUS_OK);
    serial_write(cred_id, 16);
    serial_write(pub_key, 40);

    //  Effacer la clé privée en RAM
    memset(priv_key20, 0, sizeof(priv_key20));
}




void getassertion(void) {
    id_t hashed_app_id;           // 20 octets
    uint8_t client_data_hash[20]; // 20 octets (SHA1(...))
    eep_item_t item;              // contient cred_id[16] + sk[21] depuis EEPROM
    uint8_t priv_key20[20];       // clé privée micro-ecc (20 octets)
    uint8_t signature[40];        // r||s
    const struct uECC_Curve_t *curve = uECC_secp160r1();

    //  Lire hashed_app_id et clientDataHash depuis l’UART
     if (!serial_read(hashed_app_id, 20)) {
        serial_write_byte(STATUS_ERR_BAD_PARAMETER);
        return;
    }

    // Lire les 20 octets du clientDataHash
    if (!serial_read(client_data_hash, 20)) {
        serial_write_byte(STATUS_ERR_BAD_PARAMETER);
        return;
    }

    //  Récupérer (cred_id, sk) pour cet app_id
    if (memoire_get(hashed_app_id, &item) != 0) {
        // Pas trouvé -> STATUS_ERR_NOT_FOUND
        serial_write_byte(STATUS_ERR_NOT_FOUND);
        return;
    }

    //  Demande de consentement utilisateur 
    if (!wait_for_consent()) {
        serial_write_byte(STATUS_ERR_APPROVAL);
        // On efface la clé de la SRAM 
        memset(item.sk, 0, sizeof(item.sk));
        return;
    }

    //  Extraire la vraie clé privée 20 octets depuis sk[21]
    //    On suppose qu'on a stocké 0 || priv_key20 dans sk
    memcpy(priv_key20, &item.sk[1], 20);

    //  Signer clientDataHash avec ECDSA
    if (!uECC_sign(priv_key20, client_data_hash, 20, signature, curve)) {
        serial_write_byte(STATUS_ERR_CRYPTO_FAILED);
        memset(priv_key20, 0, sizeof(priv_key20));
        memset(item.sk, 0, sizeof(item.sk));
        memset(client_data_hash, 0, sizeof(client_data_hash));
        return;
    }

    // Succès : envoyer STATUS_OK + credential_id + signature
    serial_write_byte(STATUS_OK);
    serial_write(item.cred_id, 16);
    serial_write(signature, 40);

    //  Nettoyage des secrets en RAM
    memset(priv_key20, 0, sizeof(priv_key20));
    memset(item.sk, 0, sizeof(item.sk));
    memset(client_data_hash, 0, sizeof(client_data_hash));
}



void listcredentials(void) {
    memoire_iterateur_t it;
    eep_item_t item;
    uint8_t count = 0;

    // on compte les credentials
    memoire_init_iterateur(&it);
    while (it != MEM_PARCOURS_FINI) {
        memoire_iterateur_next(&it, &item);
        count++;
    }

    // on envoit la réponse
    serial_write_byte(STATUS_OK);  // statut
    serial_write_byte(count);      // nombre de credentials

    //  ensuite on fait un deuxième tour et on  envoit  tous les credential_id
    memoire_init_iterateur(&it);
    while (it != MEM_PARCOURS_FINI) {
        memoire_iterateur_next(&it, &item);
        // On n'envoie que les 16 octets de cred_id
        serial_write(item.cred_id, 16);
    }
}

void reset(void) {
    // Demande de consentement utilisateur
    if (!wiat_for_consent()) {
        serial_write_byte(STATUS_ERR_APPROVAL);
        return;
    }
    // Effacer tous les credentials de l'Authenticator
    memoire_reset();

    // Réponse OK
    serial_write_byte(STATUS_OK);
}


