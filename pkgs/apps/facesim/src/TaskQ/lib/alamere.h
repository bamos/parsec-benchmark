/*
 *  Defines one function that is needed by PThreads and OpenMP programs
 *  in addition to the ones declared by PThreads/OpenMP APIs
 *        Sanjeev Kumar --- June, 2004
 */

#ifndef _ALAMERE_H
#define _ALAMERE_H

#ifdef  __cplusplus
# define __BEGIN_DECLS  extern "C" {
# define __END_DECLS    }
#else
# define __BEGIN_DECLS
# define __END_DECLS
#endif


//  Initializes the Alamere system to support multiple threads. This should
//  be called before any PThreads/OpenMP calls are invoked. When the code
//  has to be run on the simulator, this function needs to be called
//  while it is still running on the real machine (before the Snapshot)

#ifdef _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#endif
#define _XOPEN_SOURCE 700
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef __USE_XOPEN2K
#define __USE_XOPEN2K
#endif
#ifndef __USE_UNIX98
#define __USE_UNIX98
#endif

#define __thread __threadp
#include <pthread.h>
#undef __thread

#ifndef PTHREAD_KEYS_MAX
#define PTHREAD_KEYS_MAX 1024
#endif

#ifdef ALAMERE_PTHREADS
#define ALAMERE_INIT( max_concurrent_threads) \
        pthread_init( max_concurrent_threads, sizeof( pthread_cond_t), \
                      sizeof( pthread_barrier_t))
#define ALAMERE_END() pthread_end()
#define ALAMERE_AFTER_CHECKPOINT() pthread_after_checkpoint()
#else
#define ALAMERE_INIT( max_concurrent_threads)
#define ALAMERE_END()
#define ALAMERE_AFTER_CHECKPOINT()
#endif

#define ALAMERE_OMP_INIT() ALAMERE_INIT( omp_get_max_threads()+1);

__BEGIN_DECLS
void pthread_init (int max_concurrent_threads, int condSize, int barrierSize);
void pthread_end (void);
void pthread_after_checkpoint (void);
__END_DECLS

#endif // _ALAMERE_H
