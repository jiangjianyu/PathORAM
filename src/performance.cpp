//
// Created by maxxie on 16-6-13.
//

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "performance.h"
#include "OramLogger.h"

p_per_ctx p_ctx;

void p_init_bandwidth(struct p_bandwidth *p) {
    p->bytes = 0;
    p->kbytes = 0;
    p->mbytes = 0;
    p->gbytes = 0;
}

void p_get_time_now(struct timeval *time_val) {
    gettimeofday(time_val, NULL);
}

void * p_func(void *args) {
    unsigned char buf[sizeof(int)];
    p_ctx.p_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct sockaddr_in addr, client_addr;
    socklen_t sock_len, client_addr_len;
    sock_len = sizeof(struct sockaddr_in);
    inet_aton(p_ctx.host, &addr.sin_addr);
    addr.sin_port = htons(p_ctx.port);
    addr.sin_family = AF_INET;
    if (bind(p_ctx.p_sock, (struct sockaddr *)&addr, sock_len) < 0) {
        log_sys << "performance socket init error\n";
    } else {
        log_sys << "performance thread init finish\n";
    }


    while(1) {
        recvfrom(p_ctx.p_sock, buf, sizeof(int), 0, (struct sockaddr *)&client_addr, &client_addr_len);
        log_sys << "new request\n";
        p_get_time_now(&p_ctx.p_now_time);
        printf("Bandwidth total: %dGB %dMB %dKB %dB, original: %dGB %dMB %dKB %dB\n",
               p_ctx.p_total_bandwidth.gbytes, p_ctx.p_total_bandwidth.mbytes,
               p_ctx.p_total_bandwidth.kbytes, p_ctx.p_total_bandwidth.bytes,
               p_ctx.p_original_bandwidth.gbytes, p_ctx.p_original_bandwidth.mbytes,
               p_ctx.p_original_bandwidth.kbytes, p_ctx.p_original_bandwidth.bytes);
        printf("Stash size: %lld\n", p_ctx.p_stash_size);
        printf("Total Time: %ld seconds %ld useconds\n", p_ctx.p_now_time.tv_sec - p_ctx.p_start_time.tv_sec,
               p_ctx.p_now_time.tv_usec - p_ctx.p_start_time.tv_usec);
//        sendto(p_ctx.p_sock, &p_ctx, sizeof(p_per_ctx), 0, (struct sockaddr *)&client_addr, client_addr_len);
    }
}

void p_init(char *host, int port) {
    p_ctx.host = host;
    p_ctx.port = port;
    p_init_bandwidth(&p_ctx.p_total_bandwidth);
    p_init_bandwidth(&p_ctx.p_original_bandwidth);
    p_get_time_now(&p_ctx.p_start_time);
    pthread_create(&p_ctx.p_pid, NULL, p_func, NULL);
}

void p_bandwidth_add(struct p_bandwidth *p, int add) {
    p->bytes += add;
    int left;
    if (p->bytes >= 1024) {
        left = p->bytes >> 10;
        p->bytes = p->bytes % 1024;
        p->kbytes += left;
    }
    if (p->kbytes >= 1024) {
        left = p->kbytes >> 10;
        p->kbytes = p->kbytes % 1024;
        p->mbytes += left;
    }
    if (p->mbytes >= 1024) {
        left = p->mbytes >> 10;
        p->mbytes = p->mbytes % 1024;
        p->gbytes += left;
    }
}

void p_get_performance(char *host, int port) {
    struct sockaddr_in addr;
    unsigned char buf[sizeof(int)];
    socklen_t sock_len;
    inet_aton(host, &addr.sin_addr);
    addr.sin_port = htons(port);
    addr.sin_family = AF_INET;
    sock_len = sizeof(addr);
    p_ctx.p_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
//    sendto(p_ctx.p_sock, buf, sizeof(int), 0, (struct sockaddr *)&addr, sock_len);
//    recvfrom(p_ctx.p_sock, &p_ctx, sizeof(p_per_ctx), 0, (struct sockaddr *)&addr, &sock_len);
    p_get_time_now(&p_ctx.p_now_time);
    printf("Bandwidth total: %dGB %dMB %dKB %dB, original: %dGB %dMB %dKB %dB\n",
           p_ctx.p_total_bandwidth.gbytes, p_ctx.p_total_bandwidth.mbytes,
           p_ctx.p_total_bandwidth.kbytes, p_ctx.p_total_bandwidth.bytes,
           p_ctx.p_original_bandwidth.gbytes, p_ctx.p_original_bandwidth.mbytes,
           p_ctx.p_original_bandwidth.kbytes, p_ctx.p_original_bandwidth.bytes);
    printf("Stash size: %lld\n", p_ctx.p_stash_size);
    printf("Total Time: %ld seconds %ld useconds\n", p_ctx.p_now_time.tv_sec - p_ctx.p_start_time.tv_sec,
           p_ctx.p_now_time.tv_usec - p_ctx.p_start_time.tv_usec);
}