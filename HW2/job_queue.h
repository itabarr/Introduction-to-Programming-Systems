#ifndef JOB_QUEUE_H
#define JOB_QUEUE_H

#include <pthread.h>
#include <sys/time.h>


typedef struct Job {
    void (*function)(void *);
    char *arg;
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
    pthread_cond_t cond_q_empty;
    pthread_cond_t cond_q_non_empty;
    pthread_cond_t cond_no_active_threads;
    Archive archive;
    struct timeval start_time;
    int active_threads_num;
} Queue;

typedef struct {
    Queue *queue;
    int create_log;
    int thread_num;
} ThreadData;

Queue *create_queue();
void enqueue(Queue *queue, Job *job);
Job *dequeue(Queue *queue);

void *worker_thread(void *arg);
void kill(void *arg);
void free_queue(Queue *queue);
void add_cmnd_job(Queue *queue, char *cmnd);
void add_kill_job(Queue *queue);
void print_job_stats(Archive *archive);
void print_job_stats_to_file(Archive *archive, FILE *file);
void print_archive(Archive *archive);
void wait_for_non_pending_command(Queue *queue);
void decrement_active_threads(Queue *queue);

ThreadData *create_thread_data(Queue *queue, int create_log, int thread_num);
void free_thread_data(ThreadData *data);

void pthread_create_wrapper(pthread_t *thread, Queue *queue ,int create_log , int thread_num);

#endif
