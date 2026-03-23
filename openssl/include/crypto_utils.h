#ifndef CRYPTO_UTILS_H
#define CRYPTO_UTILS_H

#include <stdint.h>

// Chiffre les données en AES-256-GCM.
// Génère automatiquement un IV (12 octets) et un Tag d'authentification (16 octets).
// Retourne la taille du message chiffré, ou -1 en cas d'erreur.
int encrypt_data(const uint8_t* plaintext, int plaintext_len, 
                 const uint8_t* key, 
                 uint8_t* out_iv, uint8_t* out_ciphertext, uint8_t* out_tag);

// Déchiffre les données en AES-256-GCM.
// Vérifie l'intégrité du paquet grâce au Tag d'authentification.
// Retourne la taille du message en clair, ou -1 si un tricheur a altéré le paquet.
int decrypt_data(const uint8_t* ciphertext, int ciphertext_len, 
                 const uint8_t* in_tag, const uint8_t* in_iv, 
                 const uint8_t* key, 
                 uint8_t* out_plaintext);

#endif // CRYPTO_UTILS_H