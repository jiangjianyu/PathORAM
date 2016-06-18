//
// Created by maxxie on 16-6-13.
//

#ifndef PATHORAM_PERFORMANCE_H
#define PATHORAM_PERFORMANCE_H

/* Performance test for oram
 * Here defines some marco
 *
 * Statistic data are listed below:
 *  bandwidth of original data
 *  bandwidth of transferred data
 *  start time
 *  end time
 *  stash size
 * */

/* Meaning of the marcos
 * P_INIT(host init the performance system,
 * */


#include <pthread.h>
#include <sys/time.h>

#define ORAM_DEBUG_PERFORAMANCE
typedef struct p_bandwidth {
    int bytes;
    int kbytes;
    int mbytes;
    int gbytes;
} p_bandwidth;

typedef struct {
    char *host;
    int port;
    struct p_bandwidth p_total_bandwidth;
    struct p_bandwidth p_original_bandwidth;
    long long p_stash_size;
    struct timeval p_start_time;
    struct timeval p_now_time;
    int p_sock;
    pthread_t p_pid;
} p_per_ctx;

void p_bandwidth_add(struct p_bandwidth *p, int add);
void p_init(char *host, int port);
void p_get_performance(char *host, int port);

#ifdef ORAM_DEBUG_PERFORAMANCE

extern p_per_ctx p_ctx;

#define __P_ADD(size, total) total += size
#define __P_DEL(size, total) total -= size
#define __P_BANDWIDTH_ADD(size, band) p_bandwidth_add(&band, size)
#define P_INIT(host, port) p_init(host, port)
#define P_ADD_BANDWIDTH(size) __P_BANDWIDTH_ADD(size, p_ctx.p_total_bandwidth)
#define P_ADD_BANDWIDTH_ORIGINAL(size) __P_BANDWIDTH_ADD(size, p_ctx.p_original_bandwidth)
#define P_ADD_STASH(size) __P_ADD(size, p_ctx.p_stash_size)
#define P_DEL_STASH(size) __P_DEL(size, p_ctx.p_stash_size)

#else
#define P_INIT(host, port)
#define P_ADD_BANDWIDTH(size)
#define P_ADD_BANDWIDTH_ORIGINAL(size)
#define P_ADD_STASH(size)
#define P_DEL_STASH(size)

#endif


#endif //PATHORAM_PERFORMANCE_H
