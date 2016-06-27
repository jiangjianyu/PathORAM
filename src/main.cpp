//
// Created by maxxie on 16-6-12.
//

#include <cstring>
#include <cassert>
#include "OramLogger.h"
#include "OramNode.h"
#include "OramAccessController.h"
#include "performance.h"

int main (int argc, char **args) {
    if (argc <= 1)
        return -1;
    else if (!strcmp(args[1], "server")) {
        log_sys << "Server Starting" << std::endl;
        OramNode server = OramNode("127.0.0.1", 30000);
        server.run();
    } else if (!strcmp(args[1], "client")) {
        unsigned char data[4096];
        unsigned char key[ORAM_CRYPT_KEY_LEN];
        OramCrypto::set_key(key);
        OramAccessController *client = new OramAccessController("127.0.0.1", 30000,
                                        254, 120, 135, 120, 4096);
        client->init();
        p_get_performance("127.0.0.1", 30010);
        for (int i = 0;i < 25000;i++) {
            data[0] = i % 256;
            client->oblivious_access(i, ORAM_ACCESS_WRITE, data);
        }
        for (int i = 0;i < 25000;i++) {
            client->oblivious_access(i, ORAM_ACCESS_READ, data);
            assert(data[0] == i % 256);
        }

//        for (int i = 0;i < 240000;i++) {
//            client->oblivious_access(i, ORAM_ACCESS_READ, data);
//            f[i] = data[0];
//        }
        p_get_performance("127.0.0.1", 30010);
    }
    return 0;
}