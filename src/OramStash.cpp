//
// Created by maxxie on 16-6-12.
//

#include <cstring>
#include "OramStash.h"
#include "OramLogger.h"
#include "OramBucket.h"
#include "performance.h"
#include "OramCrypto.h"

#define min(a,b) (((a) < (b)) ? (a):(b))

OramStashBlock::OramStashBlock(unsigned char *in_data, int address, int bucket_id) {
    this->data = new unsigned char[OramBucket::block_len];
    memcpy(data, in_data, OramBucket::block_len);
    this->address = address;
    this->bucket_id = bucket_id;
}

void OramStash::add(OramStashBlock *block) {
    std::map<int, OramStashBlock*>::iterator itr = stash_hash.find(block->address);
    if (itr == stash_hash.end()) {
        P_ADD_STASH(1);
        stash_hash[block->address] = block;
        stash_list[block->bucket_id].push_back(block);
        for (int pos = block->bucket_id;;pos = (pos - 1) >> 1) {
            counter[pos]++;
            if (pos == 0)
                break;
        }
//        log_detail << "Add block " << block->address << " to stash, bucket " << block->bucket_id << "\n";
    }
}

OramStashBlock* OramStash::remove_by_address(int address) {
    /* if block exists in stash */
    OramStashBlock *block = stash_hash[address];
    if (block != NULL) {
        stash_list[block->bucket_id].remove(block);
        for (int pos = block->bucket_id;; pos = (pos - 1) >> 1) {
            counter[pos]--;
            if (pos == 0)
                break;
        }
        return block;
    }
    return NULL;
}

int OramStash::remove_by_bucket(int bucket_id, int max, OramStashBlock **block_list) {
    int remove = remove_by_bucket_helper(bucket_id, max, 0, block_list);
    P_DEL_STASH(remove);
//    log_detail << "Delete " << remove << " blocks to " << bucket_id << "\n";
    return remove;
}

int OramStash::remove_by_bucket_helper(int pos, int len, int start, OramStashBlock **blocklist) {
    int delete_now, delete_max;
    OramStashBlock *block;
    if (pos >= oram_bucket_size || !counter[pos]) {
        return 0;
    }
    if (pos >= oram_bucket_leaf_start) {
        delete_now = counter[pos];
        delete_max = min(delete_now, len);
        for (int j = 0;j < delete_max;j++) {
            block = stash_list[pos].front();
            blocklist[start++] = block;
            stash_list[pos].pop_front();
            stash_hash.erase(block->address);
//            log_detail << "Evict block " << block->address << " to bucket " << block->bucket_id << "\n";
            for (int pos_run = pos;; pos_run = (pos_run - 1) >> 1) {
                counter[pos_run]--;
                if (pos_run == 0)
                    break;
            }
        }
        return delete_max;
    }
    int random = OramCrypto::get_random(2);
    int left, right;
    if (random) {
        left = 2 * pos + 1;
        right = 2 * pos + 2;
    } else {
        left = 2 * pos + 2;
        right = 2 * pos + 1;
    }


    int add = remove_by_bucket_helper(left, len, start, blocklist);
    len -= add;
    start += add;
    if (len == 0)
        return add;
    return remove_by_bucket_helper(right, len, start, blocklist) + add;
}

int OramStash::find_edit_by_address(int address, OramAccessOp op, unsigned char *data) {
    std::map<int, OramStashBlock*>::iterator itr = stash_hash.find(address);
    if (itr == stash_hash.end()) {
//        log_sys << "block %d not found in stash\n";
        return -1;
    }
    OramStashBlock *block = itr->second;
    if (op == ORAM_ACCESS_READ) {
        memcpy(data, block->data, OramBucket::block_len);
    } else {
        memcpy(block->data, data, OramBucket::block_len);
    }
    return 0;
}

OramStash::OramStash(int oram_bucket_size) {
    this->counter = new int[oram_bucket_size];
    this->oram_bucket_leaf_size = (oram_bucket_size + 1)/2;
    this->oram_bucket_leaf_start = oram_bucket_size - oram_bucket_leaf_size;
    this->oram_bucket_size = oram_bucket_size;
    this->stash_list = new std::list<OramStashBlock*>[oram_bucket_size];
    bzero(this->counter, oram_bucket_size * sizeof(int));
}

OramStash::~OramStash() {
    delete[] stash_list;
    delete[] counter;
}

OramStashBlock::~OramStashBlock() {
    delete[] data;
}

void OramStash::iter() {
    for (std::map<int, OramStashBlock*>::iterator iterator = stash_hash.begin();
            iterator != stash_hash.end();iterator++) {
        log_detail << "Bucket " << iterator->second->bucket_id << " in stash\n";
    }
}