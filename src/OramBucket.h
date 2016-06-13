//
// Created by maxxie on 16-6-11.
//

#ifndef PATHORAM_ORAMBUCKET_H
#define PATHORAM_ORAMBUCKET_H

#include <cstdlib>
class OramBucket;

class OramBlockMetadata{
private:
    /* address for potential read block
     * and offset of a block, real or dummy */
    int *meta_buf;
public:
    bool *valid_bits;
    int read_counter;
    OramBlockMetadata();

    int* get_address() {return meta_buf;}
    int* get_offset();
    unsigned char *get_meta_buf() {return (unsigned char *)meta_buf;}
    ~OramBlockMetadata() {
        delete  meta_buf;
        delete valid_bits;
    }
};

class OramBucket {
public:
    static int block_len;
    static int bucket_dummy_len;
    static int bucket_real_len;
    static int bucket_total_len;
    static unsigned char *blank_data;

    static void init(int block_len, int bucket_real, int bucket_dummy);

    int bucket_id;
    unsigned char *encrypted_metadata;
    int read_counter;
    bool *valid_bits;

    unsigned char *encrypted_data;
    /* Construct a new bucket with random data */
    OramBucket();
    ~OramBucket();
    OramBucket(unsigned char serialized_data[]);

    OramBucket(unsigned char seralized_metadata[], bool metadata_bool);

    unsigned char* get_block(int id);
    size_t serialize(unsigned char serialized_data[]);
    size_t serialize_metadata(unsigned char serialized_data[]);

    size_t size();
};

#define sizeof_metadata (sizeof(int) * (OramBucket::bucket_real_len + OramBucket::bucket_total_len))

#endif //PATHORAM_ORAMBUCKET_H
