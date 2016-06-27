//
// Created by maxxie on 16-6-11.
//

#include <cstring>
#include "OramCrypto.h"
#include "OramLogger.h"

unsigned char OramCrypto::key[ORAM_CRYPT_KEY_LEN];

void OramCrypto::set_key(unsigned char *key_init) {
    memcpy(OramCrypto::key, key_init, ORAM_CRYPT_KEY_LEN);
}

void OramCrypto::encrypt_data(unsigned char ciphertext[],
                              unsigned char *data) {
    unsigned char nonce[ORAM_CRYPT_NONCE_LEN];
    randombytes_buf(nonce, ORAM_CRYPT_NONCE_LEN);
    crypto_secretbox_easy(ciphertext + ORAM_CRYPT_NONCE_LEN,
                          data, OramBucket::block_len, nonce, key);
    memcpy(ciphertext, nonce, ORAM_CRYPT_NONCE_LEN);
}

void OramCrypto::encrypt_data(unsigned char ciphertext[],
                         unsigned char *data,
                         unsigned char *nonce) {
    crypto_secretbox_easy(ciphertext + ORAM_CRYPT_NONCE_LEN,
                          data, OramBucket::block_len, nonce, key);
}




int OramCrypto::decrypt_data(unsigned char *plaintext,
                             unsigned char *encrypted_data) {
    if (crypto_secretbox_open_easy(plaintext, encrypted_data + ORAM_CRYPT_NONCE_LEN,
                               OramBucket::block_len + ORAM_CRYPT_OVERHEAD,
                               encrypted_data, key) != 0) {
        log_sys << "Decrypting data error\n";
        return -1;
    }

    return 0;
}

void OramCrypto::encrypt_metadata(unsigned char ciphertext[],
                                  OramBlockMetadata *metadata) {
    unsigned char nonce[ORAM_CRYPT_NONCE_LEN];
    crypto_secretbox_easy(ciphertext + ORAM_CRYPT_NONCE_LEN,metadata->get_meta_buf()
                          , sizeof_metadata, nonce, key);
    memcpy(ciphertext, nonce, ORAM_CRYPT_NONCE_LEN);
}

OramBlockMetadata* OramCrypto::decrypt_metadata(OramBlockMetadata *metadata ,unsigned char *encrypted_metadata) {
    if (metadata == NULL)
        metadata = new OramBlockMetadata();
    if (crypto_secretbox_open_easy(metadata->get_meta_buf(),
                               encrypted_metadata + ORAM_CRYPT_NONCE_LEN,
                               sizeof_metadata + ORAM_CRYPT_OVERHEAD,
                               encrypted_metadata,
                               key) != 0) {
        log_sys << "Decrypting meta error\n";
        return NULL;
    }
    return metadata;
}