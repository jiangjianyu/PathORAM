//
// Created by maxxie on 16-6-12.
//

#ifndef PATHORAM_ORAMSTASH_H
#define PATHORAM_ORAMSTASH_H

#include <map>
#include <list>
#include "PathORAM.h"

class OramStashBlock {
public:
    unsigned char *data;
    int address;
    int bucket_id;
    OramStashBlock(unsigned char *in_data, int address, int bucket_id);
    ~OramStashBlock();
};

class OramStash {
private:
    std::map<int, OramStashBlock*> stash_hash;
    int *counter;
    int oram_bucket_size;
    int oram_bucket_leaf_start;
    int oram_bucket_leaf_size;
    std::list<OramStashBlock*> *stash_list;
    int remove_by_bucket_helper(int pos, int len, int start, OramStashBlock *blocklist[]);
public:
    void add(OramStashBlock *block);
    OramStashBlock* remove_by_address(int address);
    int remove_by_bucket(int bucket_id, int max, OramStashBlock** block_list);
    int find_edit_by_address(int address, OramAccessOp op, unsigned char data[]);
    void iter();
    OramStash(int oram_bucket_size);
    ~OramStash();
};


#endif //PATHORAM_ORAMSTASH_H
