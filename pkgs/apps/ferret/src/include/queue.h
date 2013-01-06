#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>

struct queue {
  int head, tail;
  void ** data;
  int size;
  int prod_threads;		// no of producing threads
  int end_count;
  pthread_mutex_t mutex;
  pthread_cond_t empty, full;
};

void queue_signal_terminate(struct queue * que);
void queue_init(struct queue* que, int size, int prod_threads);
void queue_destroy(struct queue* que);
int  dequeue(struct queue* que, void** to_buf);
void enqueue(struct queue* que, void* from_buf);

#endif //QUEUE
