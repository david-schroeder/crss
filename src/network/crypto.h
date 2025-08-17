#ifndef CRYPTO_H
#define CRYPTO_H

#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/asn1.h>
#include <openssl/aes.h>
#include <openssl/err.h>
#include <openssl/pem.h>

#include "utils.h"

typedef struct {
    EVP_PKEY *pkey;
    unsigned char *pub_key_asn1;
    uint32_t pkey_asn1_len;
} keypair_t;

keypair_t *gen_keypair(void);

/* Generate verify token */
uint32_t gen_vtok();

typedef struct {
    uint32_t length;
    uint8_t *data;
} decrypted_buffer_t;

decrypted_buffer_t *rsa_decrypt_buffer(EVP_PKEY_CTX *ctx, uint8_t *buf, size_t len);

#endif // CRYPTO_H
