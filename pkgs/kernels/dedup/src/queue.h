#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <stdlib.h>

#ifdef ENABLE_PTHREADS
#include <pthread.h>
#endif //ENABLE_PTHREADS

//A simple ring buffer that can store a certain number of elements.
//This is used for two purposes:
// 1. To manage the elements inside a queue
// 2. To allow queue users aggregate queue operations
struct _ringbuffer_t {
  int head, tail;
  void **data;
  size_t size;
};

typedef struct _ringbuffer_t ringbuffer_t;

//A synchronized queue.
//Basically just a ring buffer with some synchronization added
struct _queue_t {
  ringbuffer_t buf;
  int nProducers;
  int nTerminated;
#ifdef ENABLE_PTHREADS
  pthread_mutex_t mutex;
  pthread_cond_t notEmpty, notFull;
#endif //ENABLE_PTHREADS
};

typedef struct _queue_t queue_t;



/*
 * Some simple inline functions to work with ring buffers
 */

//Initialize a ring buffer
static inline int ringbuffer_init(ringbuffer_t *buf, size_t size) {
  //NOTE: We have to allocate one extra element because one element will be unusable (we need to distinguish between full and empty).
  buf->data = (void **)malloc(sizeof(void*) * (size+1));
  buf->size = (size+1);
  buf->head = 0;
  buf->tail = 0;

  return (buf->data==NULL);
}

//Destroy a ring buffer
static inline int ringbuffer_destroy(ringbuffer_t *buf) {
  free(buf->data);
  return 0;
}

//Returns true if and only if the ring buffer is empty
static inline int ringbuffer_isEmpty(ringbuffer_t *buf) {
  return (buf->tail == buf->head);
}

//Returns true if and only if the ring buffer is full
static inline int ringbuffer_isFull(ringbuffer_t *buf) {
  return (buf->head == (buf->tail-1+buf->size)%buf->size);
}

//Get an element from a ringbuffer
//Returns NULL if buffer is empty
static inline void *ringbuffer_remove(ringbuffer_t *buf) {
  void *ptr;

  if(ringbuffer_isEmpty(buf)) {
    ptr = NULL;
  } else {
    ptr = buf->data[buf->tail];
    buf->tail++;
    if(buf->tail >= buf->size) buf->tail = 0;
  }

  return ptr;
}

//Put an element into a ringbuffer
//Returns 0 if the operation succeeded
static inline int ringbuffer_insert(ringbuffer_t *buf, void *ptr) {
  if(ringbuffer_isFull(buf)) return -1;
  buf->data[buf->head] = ptr;
  buf->head++;
  if(buf->head == buf->size) buf->head = 0;

  return 0;
}



/*
 * Queue interface
 */

void queue_init(queue_t * que, size_t size, int nProducers);
void queue_destroy(queue_t * que);

void queue_terminate(queue_t * que);

int queue_dequeue(queue_t *que, ringbuffer_t *buf, int limit);
int queue_enqueue(queue_t *que, ringbuffer_t *buf, int limit);

#endif //_QUEUE_H_

