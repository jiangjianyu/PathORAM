//
// Created by maxxie on 16-6-11.
//



#include <cstring>
#include <unistd.h>
#include "OramNode.h"
#include "OramLogger.h"

OramNode::OramNode(char *host, int port) {
    sock = new OramSocket(host, port, true);
    storage = NULL;
    running = 0;
}

void OramNode::run() {
    sock->init();
    running = 1;
    while (running) {
        OramSocket *sock_client = sock->accept_connection();
        while(running) {
            if (sock_client->standard_recv() < 0) {
                close(sock_client->sock);
                delete sock_client;
                break;
            }
            switch (sock_client->get_recv_header()->socket_type) {
                case ORAM_SOCKET_GETMETA:
                    get_metadata(sock_client);
                    break;
                case ORAM_SOCKET_READBLOCK:
                    read_block(sock_client);
                    break;
                case ORAM_SOCKET_READBUCKET:
                    read_bucket(sock_client);
                    break;
                case ORAM_SOCKET_WRITEBUCKET:
                    write_bucket(sock_client);
                    break;
                case ORAM_SOCKET_READBUCKET_PATH:
                    break;
                case ORAM_SOCKET_WRITEBUCKET_PATH:
                    break;
                case ORAM_SOCKET_INIT:
                    init_node(sock_client);
                    break;
                default:
                    log_sys << "Wrong Request Header, dropping data\n";
                    break;
            }
            sock_client->standard_send();
        }
    }
}

void OramNode::get_metadata(OramSocket *sock) {
    OramSocketHeader *header_recv = sock->get_recv_header();
    OramSocketHeader *header_send = sock->get_send_header();
    log_detail << "get metadata " << header_recv->pos_id << "\n";
    size_t last = 0;
    for (int pos = header_recv->pos_id;;pos = (pos - 1) >> 1) {
        last += storage->get_bucket(pos)->serialize_metadata((unsigned char *)sock->get_send_buf() + last);
        if (pos == 0) {
            break;
        }
    }
    header_send->msg_len = last;
    header_send->pos_id = header_recv->pos_id;
    header_send->socket_type = ORAM_SOCKET_GETMETA;
}

void OramNode::read_block(OramSocket *sock) {
    OramSocketHeader *header_recv = sock->get_recv_header();
    OramSocketHeader *header_send = sock->get_send_header();
    unsigned char *return_block = (unsigned char *)sock->get_send_buf();
    memset(return_block, 0, OramBucket::block_len + ORAM_CRYPT_OVERSIZE);
    log_detail << "get block " << header_recv->pos_id << "\n";
    int *read_offset = (int *)sock->get_recv_buf();
    unsigned char *get_block;
    size_t last = OramBucket::block_len + ORAM_CRYPT_OVERSIZE;
    for (int pos = header_recv->pos_id, i = 0;;pos = (pos - 1) >> 1, ++i) {
        get_block = storage->get_bucket(pos)->get_block(read_offset[i]);
        storage->get_bucket(pos)->valid_bits[read_offset[i]] = false;
        storage->get_bucket(pos)->read_counter++;
        for (int j = 0;j < OramBucket::block_len + ORAM_CRYPT_OVERSIZE;j++) {
            return_block[j] ^= get_block[j];
        }
        memcpy(sock->get_send_buf() + last, get_block, ORAM_CRYPT_NONCE_LEN);
        last += ORAM_CRYPT_NONCE_LEN;
        if (pos == 0)
            break;
    }
    header_send->msg_len = last;
    header_send->socket_type = ORAM_SOCKET_READBLOCK;
}

void OramNode::write_bucket(OramSocket *sock) {
    OramSocketHeader *header_recv = sock->get_recv_header();
    OramSocketHeader *header_send = sock->get_send_header();
    log_detail << "Write Bucket " << header_recv->pos_id << "\n";
    storage->set_bucket(header_recv->pos_id,
                        new OramBucket((unsigned char *)sock->get_recv_buf()));
    header_send->socket_type = ORAM_SOCKET_WRITEBUCKET;
    header_send->msg_len = 0;
}

void OramNode::read_bucket(OramSocket *sock) {
    OramSocketHeader *header_recv = sock->get_recv_header();
    OramSocketHeader *header_send = sock->get_send_header();
    log_detail << "Read Bucket " << header_recv->pos_id << "\n";
    size_t last = storage->get_bucket(header_recv->pos_id)->serialize((unsigned char *)sock->get_send_buf());
    header_send->socket_type = ORAM_SOCKET_READBUCKET;
    header_send->msg_len = last;
}

void OramNode::read_bucket_path(OramSocket *sock) {
    OramSocketHeader *header_recv = sock->get_recv_header();
    OramSocketHeader *header_send = sock->get_send_header();
    log_detail << "Read Bucket Path " << header_recv->pos_id << "\n";
    size_t last = 0;
    for (int pos = header_recv->pos_id;;pos = (pos - 1) >> 1) {
        last += storage->get_bucket(pos)->serialize((unsigned char *)sock->get_send_buf() + last);
        if (pos == 0)
            break;
    }
    header_send->socket_type = ORAM_SOCKET_READBUCKET_PATH;
    header_send->msg_len = last;
}

void OramNode::write_bucket_path(OramSocket *sock) {
    OramSocketHeader *header_recv = sock->get_recv_header();
    OramSocketHeader *header_send = sock->get_send_header();
    log_detail << "Write Bucket Path " << header_recv->pos_id << "\n";
    OramBucket *bucket;
    size_t last = 0;
    for (int pos = header_recv->pos_id;;pos = (pos - 1) >> 1) {
        bucket = new OramBucket((unsigned char *)sock->get_recv_buf() + last);
        last += bucket->size();
        storage->set_bucket(pos, bucket);
        if (pos == 0)
            break;
    }
    header_send->socket_type = ORAM_SOCKET_WRITEBUCKET_PATH;
    header_send->msg_len = 0;
}

void OramNode::init_node(OramSocket *sock) {
    OramSocketHeader *header_send = sock->get_send_header();
    OramSocketInit *init = (OramSocketInit*)sock->get_recv_buf();
    OramBucket::init(init->block_size, init->bucket_real_len, init->bucket_dummy_len);
    storage = new OramNodeStorage(init->bucket_count);
    header_send->socket_type = ORAM_SOCKET_INIT;
    header_send->msg_len = 0;
}