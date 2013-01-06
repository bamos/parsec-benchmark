/*
 * tpool.h: A thread pool
 *
 * Functions to start, manage and terminate a thread pool
 */

#ifndef TPOOL_H
#define TPOOL_H

#include <pthread.h>



/*** Declarations ***/

typedef enum {
    POOL_STATE_READY,
    POOL_STATE_RUNNING,
    POOL_STATE_ERROR
} pool_state_t;

typedef struct tpool_s {
    /* The number of threads in this pool */
    int nthreads;
    /* The thread descriptors */
    pthread_t *threads;
    
    /* The state of the pool */
    pool_state_t state;
} tpool_t;

/* The arguments passed to pthread_create */
typedef void *(*tpool_start_routine_f)(void *);
typedef struct tdesc_s {
    const pthread_attr_t *attr;
    tpool_start_routine_f start_routine;
    void *arg;
} tdesc_t;



/*** Functions ***/

/* Creates a new thread pool.
 * After the function terminates successfully, nthreads threads are running.
 *
 * opts:        Array with the start function and thread arguments for each thread
 * nthreads:    Number of threads to create (and number of entries in opts)
 *
 * return:      Pointer to the thread pool (or NULL if an error occurred)
 */
tpool_t *tpool_create(tdesc_t *opts, int nthreads);

/*
 * Destroys the thread pool.
 * The threads of the thread pool should already have been stopped when this function
 * is called.
 *
 * pool:        Pointer to the thread pool
 *
 */
void tpool_destroy(tpool_t *pool);

/*
 * Waits until all threads have joined.
 *
 * pool:        Pointer to the thread pool
 * value_ptrs:  Array for the return values of the thread functions (can be NULL)
 *
 * return:      -1 if an error occurred, 0 otherwise
 */
int tpool_join(tpool_t *pool, void **value_ptrs);

/*
 * Cancels all threads of the pool.
 *
 * pool:        Pointer to the thread pool
 *
 * return:      -1 if an error occurred, 0 otherwise
 */
int tpool_cancel(tpool_t *pool);

#endif
