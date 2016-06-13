//
// Created by maxxie on 16-6-11.
//

#ifndef PATHORAM_ORAMNODESTORAGE_H
#define PATHORAM_ORAMNODESTORAGE_H


#include "OramBucket.h"

class OramNodeStorage {
private:
    int bucket_count;
    OramBucket **bucket_list;

public:
    OramNodeStorage(int bucket);
    OramBucket *get_bucket(int id) { return bucket_list[id]; }
    void set_bucket(int id, OramBucket *bkt);
};


#endif //PATHORAM_ORAMNODESTORAGE_H
