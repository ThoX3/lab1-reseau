#include "../include/crypto_utils.h"
#include <openssl/evp.h>
#include <openssl/rand.h>

int encrypt_data(const uint8_t* plaintext, int plaintext_len, 
                 const uint8_t* key, 
                 uint8_t* out_iv, uint8_t* out_ciphertext, uint8_t* out_tag) 
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    int len = 0, ciphertext_len = 0;

    if (1 != RAND_bytes(out_iv, 12)) return -1;

    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr)) return -1;
    if (1 != EVP_EncryptInit_ex(ctx, nullptr, nullptr, key, out_iv)) return -1;

    if (1 != EVP_EncryptUpdate(ctx, out_ciphertext, &len, plaintext, plaintext_len)) return -1;
    ciphertext_len = len;

    if (1 != EVP_EncryptFinal_ex(ctx, out_ciphertext + len, &len)) return -1;
    ciphertext_len += len;

    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, out_tag)) return -1;

    EVP_CIPHER_CTX_free(ctx);
    return ciphertext_len;
}

int decrypt_data(const uint8_t* ciphertext, int ciphertext_len, 
                 const uint8_t* in_tag, const uint8_t* in_iv, 
                 const uint8_t* key, 
                 uint8_t* out_plaintext) 
{
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    int len = 0, plaintext_len = 0, ret;

    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr)) return -1;
    if (1 != EVP_DecryptInit_ex(ctx, nullptr, nullptr, key, in_iv)) return -1;

    if (1 != EVP_DecryptUpdate(ctx, out_plaintext, &len, ciphertext, ciphertext_len)) return -1;
    plaintext_len = len;

    if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, (void*)in_tag)) return -1;

    ret = EVP_DecryptFinal_ex(ctx, out_plaintext + len, &len);
    EVP_CIPHER_CTX_free(ctx);

    if (ret > 0) {
        plaintext_len += len;
        return plaintext_len;
    } else {
        return -1;
    }
}