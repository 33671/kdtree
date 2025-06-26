// msg_queue.h
#ifndef MSG_QUEUE_H
#define MSG_QUEUE_H

#include <pthread.h>

typedef struct {
  void **messages;
  int head, tail, count;
  int capacity;
  pthread_mutex_t mutex;
  pthread_cond_t cond_not_empty;
  pthread_cond_t cond_not_full;
} MessageQueue;

int msg_queue_init(MessageQueue *q, int capacity);
void msg_queue_destroy(MessageQueue *q);
void msg_queue_send_blocking(MessageQueue *q, void *msg);
void *msg_queue_recv_blocking(MessageQueue *q);
void *msg_queue_recv_nonblocking(MessageQueue *q); // 新增：非阻塞接收

#endif // MSG_QUEUE_H
