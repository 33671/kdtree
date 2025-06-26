// msg_queue.c
#include "msg_queue.h"
#include <stdio.h>
#include <stdlib.h>

int msg_queue_init(MessageQueue *q, int capacity) {
  q->messages = malloc(sizeof(void *) * capacity);
  if (!q->messages)
    return -1;

  q->head = q->tail = q->count = 0;
  q->capacity = capacity;
  pthread_mutex_init(&q->mutex, NULL);
  pthread_cond_init(&q->cond_not_empty, NULL);
  pthread_cond_init(&q->cond_not_full, NULL);
  return 0;
}

void msg_queue_destroy(MessageQueue *q) {
  free(q->messages);
  pthread_mutex_destroy(&q->mutex);
  pthread_cond_destroy(&q->cond_not_empty);
  pthread_cond_destroy(&q->cond_not_full);
}

void msg_queue_send_blocking(MessageQueue *q, void *msg) {
  pthread_mutex_lock(&q->mutex);
  while (q->count == q->capacity) {
    pthread_cond_wait(&q->cond_not_full, &q->mutex);
  }

  q->messages[q->tail] = msg;
  q->tail = (q->tail + 1) % q->capacity;
  q->count++;

  pthread_cond_signal(&q->cond_not_empty);
  pthread_mutex_unlock(&q->mutex);
}

void *msg_queue_recv_blocking(MessageQueue *q) {
  pthread_mutex_lock(&q->mutex);
  while (q->count == 0) {
    pthread_cond_wait(&q->cond_not_empty, &q->mutex);
  }

  void *msg = q->messages[q->head];
  q->head = (q->head + 1) % q->capacity;
  q->count--;

  pthread_cond_signal(&q->cond_not_full);
  pthread_mutex_unlock(&q->mutex);
  return msg;
}

void *msg_queue_recv_nonblocking(MessageQueue *q) {
  pthread_mutex_lock(&q->mutex);

  if (q->count == 0) {
    pthread_mutex_unlock(&q->mutex);
    return NULL;
  }

  void *msg = q->messages[q->head];
  q->head = (q->head + 1) % q->capacity;
  q->count--;

  pthread_cond_signal(&q->cond_not_full);
  pthread_mutex_unlock(&q->mutex);
  return msg;
}
