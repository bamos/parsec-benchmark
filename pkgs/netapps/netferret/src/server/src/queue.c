#include "queue.h"

#include <pthread.h>
#include <stdlib.h>

void queue_init(struct queue * que, int size, int prod_threads) {
  pthread_mutex_init(&que->mutex, NULL);
  pthread_cond_init(&que->empty, NULL);
  pthread_cond_init(&que->full, NULL);
  que->head = que->tail = 0;
  que->data = (void **)malloc(sizeof(void*) * size);
  que->size = size;
  que->prod_threads = prod_threads;
  que->end_count = 0;
}

void queue_destroy(struct queue* que)
{
    pthread_mutex_destroy(&que->mutex);
    pthread_cond_destroy(&que->empty);
    pthread_cond_destroy(&que->full);
    free(que->data);
    que->data = NULL;
}

void queue_signal_terminate(struct queue * que) {
  pthread_mutex_lock(&que->mutex);
  que->end_count++;
  pthread_cond_broadcast(&que->empty);
  pthread_mutex_unlock(&que->mutex);
}

int dequeue(struct queue * que, void **to_buf) {
    pthread_mutex_lock(&que->mutex);
    // chceck if queue is empty?
    while (que->tail == que->head && (que->end_count) < que->prod_threads) {
        pthread_cond_wait(&que->empty, &que->mutex);
    }
    // check if queue has been terminated?
    if (que->tail == que->head && (que->end_count) == que->prod_threads) {
        pthread_cond_broadcast(&que->empty);
        pthread_mutex_unlock(&que->mutex);
        return -1;
    }

    *to_buf = que->data[que->tail];
    que->tail ++;
    if (que->tail == que->size)
	que->tail = 0;
    pthread_cond_signal(&que->full);
    pthread_mutex_unlock(&que->mutex);
    return 0;
}

void enqueue(struct queue * que, void *from_buf) {
    pthread_mutex_lock(&que->mutex);
    while (que->head == (que->tail-1+que->size)%que->size)
	pthread_cond_wait(&que->full, &que->mutex);

    que->data[que->head] = from_buf;
    que->head ++;
    if (que->head == que->size)
	que->head = 0;

    pthread_cond_signal(&que->empty);
    pthread_mutex_unlock(&que->mutex);
}
