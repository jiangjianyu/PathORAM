//
// Created by maxxie on 16-6-11.
//

#include <cstring>
#include "OramBucket.h"
#include "OramCrypto.h"

int OramBucket::block_len = 0;
int OramBucket::bucket_dummy_len = 0;
int OramBucket::bucket_real_len = 0;
int OramBucket::bucket_total_len = 0;
unsigned char* OramBucket::blank_data = new unsigned char[OramBucket::block_len];


OramBucket::OramBucket() {
    encrypted_metadata = new unsigned char[sizeof_metadata + ORAM_CRYPT_OVERSIZE];
    encrypted_data = new unsigned char[OramBucket::bucket_total_len * (OramBucket::block_len + ORAM_CRYPT_OVERSIZE)];
    valid_bits = new bool[OramBucket::bucket_total_len];
    read_counter = 0;
    memset(valid_bits, 1, sizeof(bool) * OramBucket::bucket_total_len);
}


OramBlockMetadata::OramBlockMetadata() {
    meta_buf = new int[OramBucket::bucket_real_len + OramBucket::bucket_total_len];
    valid_bits = new bool[OramBucket::bucket_total_len];
}

int* OramBlockMetadata::get_offset() {
    return meta_buf + OramBucket::bucket_real_len;
}


OramBucket::OramBucket(unsigned char serialized_data[]){
    encrypted_metadata = new unsigned char[sizeof_metadata + ORAM_CRYPT_OVERSIZE];
    encrypted_data = new unsigned char[OramBucket::bucket_total_len * (OramBucket::block_len + ORAM_CRYPT_OVERSIZE)];
    valid_bits = new bool[OramBucket::bucket_total_len];
    read_counter = 0;
    size_t last = 0;
    memcpy(encrypted_metadata, serialized_data, sizeof_metadata + ORAM_CRYPT_OVERSIZE);
    last += sizeof_metadata + ORAM_CRYPT_OVERSIZE;
    memcpy(encrypted_data,
           serialized_data + last,
           OramBucket::bucket_total_len * (OramBucket::block_len + ORAM_CRYPT_OVERSIZE));
    last += OramBucket::bucket_total_len * (OramBucket::block_len + ORAM_CRYPT_OVERSIZE);
    memcpy(&read_counter, serialized_data + last, sizeof(int));
    last += sizeof(int);
    memcpy(valid_bits, serialized_data + last, sizeof(bool) * OramBucket::bucket_total_len);
}

size_t OramBucket::serialize_metadata(unsigned char *serialized_data) {
    size_t last = 0;
    memcpy(serialized_data, encrypted_metadata, sizeof_metadata + ORAM_CRYPT_OVERSIZE);
    last += sizeof_metadata + ORAM_CRYPT_OVERSIZE;
    memcpy(serialized_data + last, &read_counter, sizeof(int));
    last += sizeof(int);
    memcpy(serialized_data + last, valid_bits, sizeof(bool) * OramBucket::bucket_total_len);
    return sizeof_metadata + ORAM_CRYPT_OVERSIZE + sizeof(int)
           + sizeof(bool) * OramBucket::bucket_total_len;
}

size_t OramBucket::serialize(unsigned char serialized_data[]) {
    size_t last = 0;
    memcpy(serialized_data, encrypted_metadata, sizeof_metadata + ORAM_CRYPT_OVERSIZE);
    last += sizeof_metadata + ORAM_CRYPT_OVERSIZE;
    memcpy(serialized_data + last,
           encrypted_data,
           OramBucket::bucket_total_len * (OramBucket::block_len + ORAM_CRYPT_OVERSIZE));
    last += OramBucket::bucket_total_len * (OramBucket::block_len + ORAM_CRYPT_OVERSIZE);
    memcpy(serialized_data + last, &read_counter, sizeof(int));
    last += sizeof(int);
    memcpy(serialized_data + last, valid_bits, sizeof(bool) * OramBucket::bucket_total_len);
    return OramBucket::bucket_total_len * (OramBucket::block_len + ORAM_CRYPT_OVERSIZE) +
            sizeof_metadata + ORAM_CRYPT_OVERSIZE + sizeof(int)
            + sizeof(bool) * OramBucket::bucket_total_len;
}

size_t OramBucket::size() {
    size_t base = sizeof_metadata + ORAM_CRYPT_OVERSIZE + sizeof(int)
                  + sizeof(bool) * OramBucket::bucket_total_len;
    if (encrypted_data)
        return base + OramBucket::bucket_total_len * (OramBucket::block_len + ORAM_CRYPT_OVERSIZE);
    else
        return base;
}

OramBucket::OramBucket(unsigned char *serialized_metadata, bool metadata_bool) {
    encrypted_metadata = new unsigned char[sizeof_metadata + ORAM_CRYPT_OVERSIZE];
    encrypted_data = NULL;
    valid_bits = new bool[OramBucket::bucket_total_len];
    read_counter = 0;
    size_t last = 0;
    memcpy(encrypted_metadata, serialized_metadata, sizeof_metadata + ORAM_CRYPT_OVERSIZE);
    last += sizeof_metadata + ORAM_CRYPT_OVERSIZE;
    memcpy(&read_counter, serialized_metadata + last, sizeof(int));
    last += sizeof(int);
    memcpy(valid_bits, serialized_metadata + last, sizeof(bool) * OramBucket::bucket_total_len);
}

unsigned char* OramBucket::get_block(int id) {
    return encrypted_data + id * (OramBucket::block_len + ORAM_CRYPT_OVERSIZE);
}

void OramBucket::init(int block, int bucket_real, int bucket_dummy) {
    block_len = block;
    bucket_real_len = bucket_real;
    bucket_dummy_len = bucket_dummy;
    bucket_total_len = bucket_real + bucket_dummy;
    blank_data = new unsigned char[block_len];
    bzero(blank_data, block_len);
}

OramBucket::~OramBucket() {
    delete[] valid_bits;
    delete[] encrypted_metadata;
    delete[] encrypted_data;
}