//
// Created by maxxie on 16-6-11.
//

#ifndef PATHORAM_ORAMSOCKET_H
#define PATHORAM_ORAMSOCKET_H

#include "OramCrypto.h"

#define ORAM_SOCKET_BACKLOG 30
#define ORAM_SOCKET_TIMEOUT_SECOND 5
#define ORAM_SOCKET_TIMEOUT_USECOND 0
#define ORAM_SOCKET_BUFFER_SIZE 2048000
typedef enum oram_socket_type {
    ORAM_SOCKET_READBLOCK,
    ORAM_SOCKET_GETMETA,
    ORAM_SOCKET_READBUCKET_PATH,
    ORAM_SOCKET_WRITEBUCKET_PATH,
    ORAM_SOCKET_READBUCKET,
    ORAM_SOCKET_WRITEBUCKET,
    ORAM_SOCKET_INIT
} oram_socket_type;

typedef struct OramSocketHeader {
    oram_socket_type socket_type;
    /* Suggest a path or a bucket id*/
    int pos_id;
    int len;
    size_t msg_len;
} OramSocketHeader;

typedef struct OramSocketInit {
    int block_size;
    int bucket_real_len;
    int bucket_dummy_len;
    int bucket_count;
} OramSocketInit ;

#define ORAM_SOCKET_HEADER_SIZE sizeof(OramSocketHeader)

class OramSocket {
public:
    int sock;
    char *host;
    int port;
    int if_bind;
    size_t last;
    unsigned char *buf_r;
    unsigned char *buf_s;
    /* buffer for send and recv */

    OramSocket();
    OramSocket(char* host, int port, int bind);
    OramSocket(int sock);
    ~OramSocket();

    int standard_send(size_t len);
    int standard_recv(size_t len);
    int standard_recv();
    int standard_send();
    int recv_continue(size_t len);
    int init();
    void close_connection();
    OramSocket* accept_connection();
    void* get_recv_buf() { return buf_r + ORAM_SOCKET_HEADER_SIZE; }
    void* get_send_buf() { return buf_s + ORAM_SOCKET_HEADER_SIZE; }
    void* get_raw_recv_buf() { return buf_r; }
    void* get_raw_send_buf() { return buf_s; }
    OramSocketHeader* get_recv_header() { return (OramSocketHeader*)buf_r; }
    OramSocketHeader* get_send_header() { return (OramSocketHeader*)buf_s; }
};


#endif //PATHORAM_ORAMSOCKET_H
