#include "OramSocket.h"
#include <arpa/inet.h>
#include <cerrno>
#include <unistd.h>
#include "OramLogger.h"

OramSocket::OramSocket()
{
}


OramSocket::~OramSocket()
{
    delete[] buf_r;
    delete[] buf_s;
}

int OramSocket::init() {
    int r = 0;
    struct sockaddr_in addr;
    socklen_t addrlen;
    inet_aton(host, &addr.sin_addr);
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    this->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    addrlen = sizeof(struct sockaddr_in);
    if (if_bind) {
        if (bind(sock, (struct sockaddr *) &addr, addrlen) < 0) {
            log_sys << "Bind Error" << std::endl;
            return -1;
        }
        if (listen(sock, ORAM_SOCKET_BACKLOG) < 0) {
            log_sys << "Listen Error" << std::endl;
            return -1;
        }
    }
    else {
        if (connect(sock, (struct sockaddr *) &addr, addrlen) < 0) {
            int e = errno;
            log_sys << "Connect Error" << std::endl;
            return -1;
        }
    }
    return 1;
}

OramSocket::OramSocket(char *host, int port, int if_bind) {
    this->host = host;
    this->port = port;
    this->if_bind = if_bind;
    buf_r = (unsigned char *)malloc(ORAM_SOCKET_BUFFER_SIZE);
    buf_s = (unsigned char *)malloc(ORAM_SOCKET_BUFFER_SIZE);
}

int OramSocket::standard_send(size_t len) {
    if (len <= 0)
        return 0;
    ssize_t r;
    int retry = 0;
    struct timeval tv;
    tv.tv_sec = ORAM_SOCKET_TIMEOUT_SECOND;
    tv.tv_usec = ORAM_SOCKET_TIMEOUT_USECOND;
#ifdef ORAM_SOCKET_TIMEOUT
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
#endif
    while (1) {
        if (retry >= 5) {
            return -1;
        }
        r = send(sock, buf_s, len, 0);
        if (r < 0) {
            if (errno == EWOULDBLOCK) {
                continue;
            }
            return -1;
        }
        else if (r == 0) {
            return -1;
        }
        else {
            return 0;
        }
    }
}

int OramSocket::recv_continue(size_t len) {
    if (len <= 0)
        return 0;
    int total = 0, r = 0, retry = 0;
    struct timeval tv;
    tv.tv_sec = ORAM_SOCKET_TIMEOUT_SECOND;
    tv.tv_usec = ORAM_SOCKET_TIMEOUT_USECOND;
#ifdef ORAM_SOCKET_TIMEOUT
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif
    while (total < len) {
        if (retry >= 5) {
            return -1;
        }
        r = recv(sock, buf_r + last + total, len, 0);
        if (r < 0) {
            if (errno == EWOULDBLOCK) {
                retry++;
                continue;
            }
            return -1;
        }
        else if (r == 0) {
            return -1;
        }
        total += r;
    }
    last += len;
    return 0;
}

int OramSocket::standard_recv(size_t len) {
    int total = 0, r = 0, retry = 0;
    struct timeval tv;
    tv.tv_sec = ORAM_SOCKET_TIMEOUT_SECOND;
    tv.tv_usec = ORAM_SOCKET_TIMEOUT_USECOND;
#ifdef ORAM_SOCKET_TIMEOUT
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif
    while (total < len) {
        if (retry >= 5) {
            return -1;
        }
        r = recv(sock, buf_r + total, len, 0);
        if (r < 0) {
            if (errno == EWOULDBLOCK) {
                retry++;
                continue;
            }
            return -1;
        }
        else if (r == 0) {
            log_sys << "connection disconnected\n";
            return -1;
        }
        total += r;
    }
    last = len;
    return 0;
}

OramSocket* OramSocket::accept_connection() {
    if (!if_bind)
        return NULL;
    log_all << "Waiting for connection" << std::endl;
    int fd = accept(sock, NULL, NULL);
    if (fd <= 0) {
        return NULL;
    }
    log_all << "New Connection connected" << std::endl;
    return new OramSocket(fd);
}

OramSocket::OramSocket(int sock) {
    buf_r = (unsigned char *)malloc(ORAM_SOCKET_BUFFER_SIZE);
    buf_s = (unsigned char *)malloc(ORAM_SOCKET_BUFFER_SIZE);
    this->sock = sock;
    if_bind = 0;
}

int OramSocket::standard_recv() {
    int status = standard_recv(ORAM_SOCKET_HEADER_SIZE);
    if (status < 0)
        return -1;
    return recv_continue(get_recv_header()->msg_len);
}

int OramSocket::standard_send() {
    return standard_send(get_send_header()->msg_len + ORAM_SOCKET_HEADER_SIZE);
}

void OramSocket::close_connection() {
    close(this->sock);
}