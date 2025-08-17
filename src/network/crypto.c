#include "crypto.h"

keypair_t *gen_keypair(void) {
    EVP_PKEY_CTX *ctx = NULL;
    EVP_PKEY *public_key = NULL;

    OpenSSL_add_all_algorithms();

    /* Generate RSA key */
    ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);

    EVP_PKEY_keygen_init(ctx);
    EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, 1024);

    EVP_PKEY_keygen(ctx, &public_key);

    EVP_PKEY_CTX_free(ctx);
    EVP_cleanup(); // hi
    ERR_free_strings();

    keypair_t *keypair = malloc(sizeof(keypair_t));
    keypair->pkey = public_key;

    unsigned char *pkey_str;
    int len = i2d_PUBKEY(public_key, &pkey_str);
    keypair->pub_key_asn1 = pkey_str;
    keypair->pkey_asn1_len = len;

    return keypair;
}

uint32_t gen_vtok() {
    srand(time(NULL));
    return rand();
}

/* @return NULL on failure */
decrypted_buffer_t *rsa_decrypt_buffer(EVP_PKEY_CTX *ctx, uint8_t *buf, size_t len) {
    if (ctx == NULL) return NULL;
    if (EVP_PKEY_decrypt_init(ctx) <= 0) return NULL;
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0) return NULL;
    size_t result_len;
    if (EVP_PKEY_decrypt(ctx, NULL, &result_len, buf, len) <= 0) return NULL;
    uint8_t *decrypted = OPENSSL_malloc(result_len);
    if (EVP_PKEY_decrypt(ctx, decrypted, &result_len, buf, len) <= 0) {
        OPENSSL_free(decrypted);
        return NULL;
    }
    decrypted_buffer_t *res = malloc(sizeof(decrypted_buffer_t));
    res->data = decrypted;
    res->length = result_len;
    return res;
}

int crypto_main() {
    keypair_t *kp = gen_keypair();
    EVP_PKEY_free(kp->pkey);
    free(kp->pub_key_asn1);
    free(kp);
    return 0;
}
