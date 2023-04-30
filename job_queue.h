#ifndef JOB_QUEUE_H
#define JOB_QUEUE_H

#include <pthread.h>


typedef struct Job {
    void (*function)(void *);
    void *arg;
    struct Job *next;
    struct timeval start_time;
    struct timeval end_time;
} Job;

typedef struct Archive {
    Job *head;
    int count;
} Archive;

typedef struct Queue {
    Job *head;
    Job *tail;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    Archive archive;
} Queue;

Queue *create_queue();
void enqueue(Queue *queue, Job *job);
Job *dequeue(Queue *queue);
void *worker_thread(void *arg);
void kill(void *arg);
void free_queue(Queue *queue);

#endif
