//
// Created by maxxie on 16-6-11.
//

#include <cstring>
#include "OramNodeStorage.h"

OramNodeStorage::OramNodeStorage(int bucket) {
    this->bucket_count = bucket;
    this->bucket_list = new OramBucket*[bucket];
    memset(this->bucket_list, 0, sizeof(OramBucket*) * bucket);
}

void OramNodeStorage::set_bucket(int id, OramBucket *bkt) {
    if (bucket_list[id])
        delete bucket_list[id];
    bucket_list[id] = bkt;
}