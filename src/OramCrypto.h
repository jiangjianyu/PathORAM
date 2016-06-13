//
// Created by maxxie on 16-6-11.
//

#ifndef PATHORAM_ORAMCRYPTO_H
#define PATHORAM_ORAMCRYPTO_H

#include <sodium.h>
#include "OramBucket.h"

#define ORAM_CRYPT_KEY_LEN crypto_secretbox_KEYBYTES
#define ORAM_CRYPT_NONCE_LEN crypto_secretbox_NONCEBYTES
#define ORAM_CRYPT_OVERSIZE crypto_secretbox_MACBYTES + ORAM_CRYPT_NONCE_LEN
#define ORAM_CRYPT_OVERHEAD crypto_secretbox_MACBYTES

class OramCrypto {
public:
    static unsigned char key[ORAM_CRYPT_KEY_LEN];
    static void set_key(unsigned char key[]);
    static void encrypt_data(unsigned char ciphertext[], unsigned char data[]);
    static void encrypt_data(unsigned char ciphertext[], unsigned char data[], unsigned char nonce[]);
    static int decrypt_data(unsigned char plaintext[], unsigned char encrypted_data[]);
    static void encrypt_metadata(unsigned char ciphertext[], OramBlockMetadata *metadata);
    static OramBlockMetadata* decrypt_metadata(OramBlockMetadata *metadata, unsigned char encrypted_metadata[]);
    static int get_random(int range) {return randombytes_uniform(range);}
};


#endif //PATHORAM_ORAMCRYPTO_H
