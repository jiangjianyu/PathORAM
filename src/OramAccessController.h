//
// Created by maxxie on 16-6-12.
//

#ifndef PATHORAM_ORAMACCESSCONTROLLER_H
#define PATHORAM_ORAMACCESSCONTROLLER_H


#include "OramStash.h"
#include "OramSocket.h"

class OramAccessController {
private:
    OramSocket *socket;

    int reshuffle_rate;
    int evict_count;
    int evict_g;
    int *position_map;

    int oram_bucket_size;
    int oram_tree_height;
    int oram_bucket_leaf_start;
    int oram_bucket_leaf_count;
    OramStash *stash;

    int read_path(int pos, int address, unsigned char data[]);
    int get_metadata(int pos, OramBlockMetadata *meta_list);
    int read_block(int pos, int address, OramBlockMetadata *meta_list,unsigned char data[]);
    int early_reshuffle(int pos, OramBlockMetadata *metadata_list);
    void evict_path(int pos);
    int get_random_dummy(bool valid_bits[], int offset[]);
    int read_bucket(int bucket_id);
    int write_bucket(int bucket_id);
public:
    OramAccessController(char *host, int port, int oram_bucket_size, int bucket_read,
                         int bucket_dummy, int reshuffle_rate, int block_len);
    int oblivious_access(int address, OramAccessOp op, unsigned char data[]);
    void init();
};


#endif //PATHORAM_ORAMACCESSCONTROLLER_H
