#ifndef JOB_QUEUE_H
#define JOB_QUEUE_H

#include <pthread.h>

typedef struct Job {
    void (*function)(void *);
    void *arg;
    struct Job *next;
} Job;

typedef struct Queue {
    Job *head;
    Job *tail;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} Queue;

Queue *create_queue();
void enqueue(Queue *queue, Job *job);
Job *dequeue(Queue *queue);
void *worker_thread(void *arg);
void thread_func(void *arg);
void kill(void *arg);

#endif
