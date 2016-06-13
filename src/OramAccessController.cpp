//
// Created by maxxie on 16-6-12.
//

#include <cmath>
#include <cstring>
#include "OramAccessController.h"
#include "OramLogger.h"

typedef struct {
    int random;
    int no;
} two_random;

int cmp(const void *a, const void *b) {
    return ((two_random *)a)->random <= ((two_random *)b)->random;
}

int gen_reverse_lexicographic(int g, int oram_size, int tree_height) {
    int i, pos = 0;
    for (i = 0;i < tree_height - 1;i++) {
        pos = pos * 2 + (g & 0x01) + 1;
        g >>= 1;
    }
    if (pos >= oram_size)
        pos >>= 1;
    return pos;
}

void get_random_permutation(int len, int permutation[]) {
    two_random random_list[len];
    int i;
    for (i = 0; i < len; i++) {
        random_list[i].random = OramCrypto::get_random(len << 7);
        random_list[i].no = i;
    }
    qsort(random_list, len, sizeof(two_random), cmp);
    for (i = 0; i < len; i++)
        permutation[i] = (unsigned) random_list[i].no;
}

int OramAccessController::get_random_dummy(bool valid_bits[], int offsets[]) {
    for (int i = OramBucket::bucket_real_len;i < OramBucket::bucket_total_len;i++) {
        if (valid_bits[offsets[i]])
            return offsets[i];
    }
    log_sys << "not enough dummy\n";
    return 0;
}

OramAccessController::OramAccessController(char *host, int port, int oram_bucket_size,
                                            int bucket_real, int bucket_dummy, int reshuffle_rate,
                                            int block_len) {
    socket = new OramSocket(host, port, false);
    this->oram_bucket_size = oram_bucket_size;
    this->oram_bucket_leaf_count = (oram_bucket_size + 1) / 2;
    this->oram_bucket_leaf_start = oram_bucket_size - oram_bucket_leaf_count;
    this->reshuffle_rate = reshuffle_rate;
    this->oram_tree_height = log(oram_bucket_size)/log(2) + 1;
    evict_count = 0;
    evict_g = 0;
    this->position_map = new int[oram_bucket_size * bucket_real];
    stash = new OramStash(oram_bucket_size);
    OramBucket::init(block_len, bucket_real, bucket_dummy);
}

int OramAccessController::oblivious_access(int address, OramAccessOp op, unsigned char *data) {
    int position;
    int position_new;
    unsigned char read_data[OramBucket::block_len];
    int found_in_stash = stash->find_edit_by_address(address, op, data);
    if (found_in_stash < 0) {
//        socket->init();
        position = position_map[address];
        position_new = OramCrypto::get_random(oram_bucket_leaf_count) + oram_bucket_leaf_start;
        position_map[address] = position_new;
        if (read_path(position, address, read_data) <= 0) {
            log_sys << "Access block " << address << " from new " << "\n";
            memcpy(read_data, OramBucket::blank_data, OramBucket::block_len);
        } else {
            log_sys << "Access block " << address << " from server " << "\n";
        }
        if (op == ORAM_ACCESS_READ) {
            memcpy(data, read_data, OramBucket::block_len);
        } else if (op == ORAM_ACCESS_WRITE) {
            memcpy(read_data, data, OramBucket::block_len);
        }
        OramStashBlock *stashBlock = new OramStashBlock(read_data, address, position_new);
        stash->add(stashBlock);
        evict_count = (evict_count + 1) % reshuffle_rate;
        if (evict_count == 0) {
            evict_path(gen_reverse_lexicographic(evict_g, oram_bucket_size, oram_tree_height));
            evict_g = (evict_g + 1) % oram_bucket_leaf_count;
        }
//        socket->close_connection();
    } else {
        log_sys << "Access " << address << " from stash\n";
    }
    return 0;
}

int OramAccessController::get_metadata(int pos, OramBlockMetadata *meta_list) {
    OramSocketHeader *header = socket->get_send_header();
    header->pos_id = pos;
    header->socket_type = ORAM_SOCKET_GETMETA;
    header->msg_len = 0;
    socket->standard_send();
    socket->standard_recv();
    size_t last = 0;
    OramBucket *bucket;
    for (int pos_run = pos,i = 0;;pos_run = (pos_run - 1) >> 1, ++i) {
        bucket = new OramBucket((unsigned char *)socket->get_recv_buf() + last, 1);
        meta_list[i].read_counter = bucket->read_counter;
        if (OramCrypto::decrypt_metadata(&meta_list[i], bucket->encrypted_metadata) == NULL)
            return -1;
        memcpy(meta_list[i].valid_bits, bucket->valid_bits, sizeof(bool) * OramBucket::bucket_total_len);
        last += bucket->size();
        delete bucket;
        if (pos_run == 0)
            break;
    }
    return 0;
}

int OramAccessController::read_block(int pos, int address, OramBlockMetadata *meta_list, unsigned char *data) {
    bool found = false;
    int found_i = -1, len;
    unsigned char xor_tem[OramBucket::block_len + ORAM_CRYPT_OVERSIZE];
    int *read_offset = (int *)socket->get_send_buf();

    for (int i = 0, pos_run = pos;; pos_run = (pos_run - 1) >> 1, ++i) {
        if (found) {
            read_offset[i] = get_random_dummy(meta_list[i].valid_bits, meta_list[i].get_offset());
        }
        else {
            for (int j = 0; j < OramBucket::bucket_real_len; j++) {
                if (meta_list[i].get_address()[j] == address && meta_list[i].valid_bits[meta_list[i].get_offset()[j]]) {
                    read_offset[i] = meta_list[i].get_offset()[j];
                    found = true;
                    found_i = i;
                    break;
                }
            }
            if (!found) {
                read_offset[i] = get_random_dummy(meta_list[i].valid_bits, meta_list[i].get_offset());
            }
        }

        if (pos_run == 0) {
            len = i + 1;
            break;
        }
    }
    socket->get_send_header()->pos_id = pos;
    socket->get_send_header()->socket_type = ORAM_SOCKET_READBLOCK;
    socket->get_send_header()->msg_len = sizeof(int) * len;
    socket->standard_send();
    socket->standard_recv();
    unsigned char *return_block = (unsigned char *)socket->get_recv_buf();
    unsigned char *nonce = return_block + OramBucket::block_len + ORAM_CRYPT_OVERSIZE;

    for (int i = 0, pos_run = pos;; pos_run = (pos_run - 1) >> 1, ++i) {
        OramCrypto::encrypt_data(xor_tem, OramBucket::blank_data, nonce + ORAM_CRYPT_NONCE_LEN * i);
        if (i != found_i) {
            for (int j = 0;j < OramBucket::block_len + ORAM_CRYPT_OVERSIZE;j++) {
                return_block[j] ^= xor_tem[j];
            }
        }
        if (pos_run == 0)
            break;
    }
    if (found) {
        memcpy(return_block, nonce + found_i * ORAM_CRYPT_NONCE_LEN, ORAM_CRYPT_NONCE_LEN);
        OramCrypto::decrypt_data(data, return_block);
    }
    return found;
}

int OramAccessController::read_path(int pos, int address, unsigned char *data) {
    OramBlockMetadata *metadata_list = new OramBlockMetadata[oram_tree_height + 1];
    if (get_metadata(pos, metadata_list) < 0) {
        return -1;
    }

    int found = read_block(pos, address, metadata_list, data);
    early_reshuffle(pos, metadata_list);
    delete[] metadata_list;
    return found;
}

int OramAccessController::read_bucket(int bucket_id) {
    socket->get_send_header()->pos_id = bucket_id;
    socket->get_send_header()->socket_type = ORAM_SOCKET_READBUCKET;
    socket->get_send_header()->msg_len = 0;
    socket->standard_send();
    socket->standard_recv();
    OramBucket *bucket = new OramBucket((unsigned char *)socket->get_recv_buf());
    OramBlockMetadata *metadata = OramCrypto::decrypt_metadata(NULL, bucket->encrypted_metadata);
    unsigned char decrypted_data[OramBucket::block_len];
    memcpy(metadata->valid_bits, bucket->valid_bits, sizeof(bool) * OramBucket::bucket_total_len);
    for (int i = 0;i < OramBucket::bucket_real_len;i++) {
        if (metadata->get_address()[i] != -1 && metadata->valid_bits[metadata->get_offset()[i]]) {
            OramCrypto::decrypt_data(decrypted_data, bucket->get_block(metadata->get_offset()[i]));
//            log_detail << "add " << metadata->get_address()[i] << " to stash, bucket " << bucket_id << "\n";
            stash->add(new OramStashBlock(decrypted_data, metadata->get_address()[i], position_map[metadata->get_address()[i]]));
        }
    }
    delete bucket;
    delete metadata;
    return 0;
}

int OramAccessController::write_bucket(int bucket_id) {
    int i;
    OramBlockMetadata *metadata = new OramBlockMetadata();
    OramStashBlock **stash_list = new OramStashBlock*[OramBucket::bucket_real_len];
    OramBucket *bucket = new OramBucket();
    int count = stash->remove_by_bucket(bucket_id, OramBucket::bucket_real_len, stash_list);
    get_random_permutation(OramBucket::bucket_total_len, metadata->get_offset());
    for (i = 0;i < count;i++) {
        OramCrypto::encrypt_data(bucket->get_block(metadata->get_offset()[i]), stash_list[i]->data);
        metadata->get_address()[i] = stash_list[i]->address;
        delete stash_list[i];
//        log_detail << "Evict " << stash_list[i]->address << " to bucket " << bucket_id << " \n";
    }
    for (i = count;i < OramBucket::bucket_total_len;i++) {
        OramCrypto::encrypt_data(bucket->get_block(metadata->get_offset()[i]), OramBucket::blank_data);
        if (i < OramBucket::bucket_real_len)
            metadata->get_address()[i] = -1;
    }
    OramCrypto::encrypt_metadata(bucket->encrypted_metadata, metadata);
    socket->get_send_header()->msg_len = bucket->serialize((unsigned char *)socket->get_send_buf());
    socket->get_send_header()->pos_id = bucket_id;
    socket->get_send_header()->socket_type = ORAM_SOCKET_WRITEBUCKET;
    socket->standard_send();
    socket->standard_recv();
    delete metadata;
    delete bucket;
    delete[] stash_list;
    return 0;
}

int OramAccessController::early_reshuffle(int pos, OramBlockMetadata *metadata_list) {
    for (int pos_run = pos,i = 0;;pos_run = (pos_run - 1) >> 1, ++i) {
        if (metadata_list[i].read_counter >= OramBucket::bucket_dummy_len - 1) {
            log_sys << "early reshuffle in pos " << pos_run << "\n";
            read_bucket(pos_run);
            write_bucket(pos_run);
        }
        if (pos_run == 0)
            break;
    }
    return 0;
}

void OramAccessController::evict_path(int pos) {
    log_sys << "evict path in pos " << pos << "\n";
    for (int pos_run = pos,i = 0;;pos_run = (pos_run - 1) >> 1, ++i) {
        read_bucket(pos_run);
        write_bucket(pos_run);
        if (pos_run == 0)
            break;
    }
}

void OramAccessController::init() {
    socket->init();
    socket->get_send_header()->socket_type = ORAM_SOCKET_INIT;
    OramSocketInit *init = (OramSocketInit *)socket->get_send_buf();
    init->block_size = OramBucket::block_len;
    init->bucket_count = oram_bucket_size;
    init->bucket_real_len = OramBucket::bucket_real_len;
    init->bucket_dummy_len = OramBucket::bucket_dummy_len;
    socket->get_send_header()->msg_len = sizeof(OramSocketInit);
    socket->standard_send();
    socket->standard_recv();
    OramBucket *bucket = new OramBucket();
    OramBlockMetadata *metadata = new OramBlockMetadata();
    for (int i = 0;i < oram_bucket_size * OramBucket::bucket_real_len;i++) {
        position_map[i] = OramCrypto::get_random(oram_bucket_leaf_count) + oram_bucket_leaf_start;
    }

    for (int i = 0;i < OramBucket::bucket_real_len;i++) {
        metadata->get_address()[i] = -1;
    }
    get_random_permutation(OramBucket::bucket_total_len, metadata->get_offset());
    for (int i = 0;i < oram_bucket_size;i++) {
        OramCrypto::encrypt_metadata(bucket->encrypted_metadata, metadata);
        for (int j = 0;j < OramBucket::bucket_total_len;j++) {
            OramCrypto::encrypt_data(bucket->get_block(j), OramBucket::blank_data);
        }
        socket->get_send_header()->msg_len = bucket->serialize((unsigned char *)socket->get_send_buf());
        socket->get_send_header()->pos_id = i;
        socket->get_send_header()->socket_type = ORAM_SOCKET_WRITEBUCKET;
        socket->standard_send();
        socket->standard_recv();
    }
    delete bucket;
    delete metadata;
//    socket->close_connection();
}