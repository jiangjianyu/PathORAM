//
// Created by maxxie on 16-6-11.
//

#ifndef PATHORAM_ORAMNODE_H
#define PATHORAM_ORAMNODE_H


#include "OramNodeStorage.h"
#include "OramSocket.h"

class OramNode {
public:
    bool running;
    OramSocket *sock;
    OramNodeStorage *storage;
    OramNode(char *host, int port);
    void run();
    void get_metadata(OramSocket *sock);
    void read_block(OramSocket *sock);
    void read_bucket(OramSocket *sock);
    void write_bucket(OramSocket *sock);
    void read_bucket_path(OramSocket *sock);
    void write_bucket_path(OramSocket *sock);
    void init_node(OramSocket *sock);
};


#endif //PATHORAM_ORAMNODE_H
