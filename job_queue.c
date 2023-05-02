#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <limits.h>

#include "job_queue.h"
#include "basic_commands.h"

Queue *create_queue() {
    Queue *queue = (Queue*) malloc(sizeof(Queue));
    queue->head = NULL;
    queue->tail = NULL;
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->cond_q_non_empty, NULL);
    pthread_cond_init(&queue->cond_q_empty, NULL);
    return queue;
}

void enqueue(Queue *queue, Job *job) {
    pthread_mutex_lock(&queue->mutex);
    job->next = NULL;
    if (queue->tail == NULL) {
        queue->head = job;
        queue->tail = job;
    } else {
        queue->tail->next = job;
        queue->tail = job;
    }
    pthread_cond_signal(&queue->cond_q_non_empty);
    pthread_mutex_unlock(&queue->mutex);
}

Job *dequeue(Queue *queue) {
    pthread_mutex_lock(&queue->mutex);
    while (queue->head == NULL) {
        pthread_cond_signal(&queue->cond_q_empty);
        pthread_cond_wait(&queue->cond_q_non_empty, &queue->mutex);
    }
    Job *job = queue->head;
    queue->head = queue->head->next;
    if (queue->head == NULL) {
        queue->tail = NULL;
    }

    Archive *archive = &queue->archive;
    job->next = archive->head;
    archive->head = job;
    archive->count++;

    pthread_mutex_unlock(&queue->mutex);
    return job;
}

void *worker_thread(void *arg) {
    struct timeval start_time, end_time;
    ThreadData *data = (ThreadData*) arg;
    Queue *queue = data->queue;
    int create_log = data->create_log;
    int thread_num = data->thread_num;


    char log_filename[16];
    FILE *log_file = NULL;
    if (create_log) {
        sprintf(log_filename, "thread%02d.txt", thread_num);
        log_file = fopen(log_filename, "w");
    }
    
    while (1) {
        Job *job = dequeue(queue);

        if (job->function == kill){
            break;
        }

        if (create_log) {
            struct timeval current_time;
            gettimeofday(&current_time, NULL);
            long long total_time = (current_time.tv_sec - start_time.tv_sec) * 1000LL +
                (current_time.tv_usec - start_time.tv_usec) / 1000LL;

            fprintf(log_file, "TIME %lld: START job %s\n", total_time, job->arg);
        }
        
        gettimeofday(&job->start_time, NULL);
        job->function(job->arg);
        gettimeofday(&job->end_time, NULL);

        if (create_log) {
            struct timeval current_time;
            gettimeofday(&current_time, NULL);
            long long total_time = (current_time.tv_sec - start_time.tv_sec) * 1000LL +
                (current_time.tv_usec - start_time.tv_usec) / 1000LL;

            fprintf(log_file, "TIME %lld: END job %s\n", total_time, job->arg);
        }

        
    }
    if (create_log) {
        fclose(log_file);
    }

    free_thread_data(data);
    return NULL;
}

void kill(void *arg){
    
}

void print_archive(Archive *archive) {
    printf("Job archive:\n");
    Job *job = archive->head;
    while (job != NULL) {
        printf("Function: %p, Argument: %s, Start time: %ld.%06ld, End time: %ld.%06ld\n",
            job->function, (char*) job->arg, job->start_time.tv_sec, job->start_time.tv_usec,
            job->end_time.tv_sec, job->end_time.tv_usec);
        job = job->next;
    }
    printf("Total jobs in archive: %d\n", archive->count);
}

void print_job_stats(Archive *archive) {
    // Calculate sum, min, max, and count of job turnaround times
    if (archive->head == NULL) {
            printf("Job archive is empty\n");
            return;
    }

    long long sum = 0;
    long long min_time = LLONG_MAX;
    long long max_time = LLONG_MIN;
    int count = 0;
    Job *job = archive->head;
    while (job != NULL) {
        if (job->arg != NULL) {
            long long turnaround_time = (job->end_time.tv_sec - job->start_time.tv_sec) * 1000 +
                                        (job->end_time.tv_usec - job->start_time.tv_usec) / 1000;
            sum += turnaround_time;
            if (turnaround_time < min_time) {
                min_time = turnaround_time;
            }
            if (turnaround_time > max_time) {
                max_time = turnaround_time;
            }
            count++;
        }
        job = job->next;
    }

    // Calculate average turnaround time
    double average_time = (double) sum / count;

    // Print job stats
    printf("Sum of jobs turnaround time: %lld milliseconds\n", sum);
    printf("Min job turnaround time: %lld milliseconds\n", min_time);
    printf("Average job turnaround time: %f milliseconds\n", average_time);
    printf("Max job turnaround time: %lld milliseconds\n", max_time);
}

void add_cmnd_job(Queue *queue, char *cmnd){
    Job *job = (Job*) malloc(sizeof(Job));
    job->function = run_job;
    job->arg = cmnd;
    enqueue(queue, job);
}

void add_kill_job(Queue *queue){
    Job *job = (Job*) malloc(sizeof(Job));
    job->function = kill;
    job->arg = NULL;
    job->start_time = (struct timeval) {0, 0};
    job->end_time = (struct timeval) {0, 0};
    enqueue(queue, job);
}

void free_queue(Queue *queue) {
    // Check if queue is empty
    pthread_mutex_lock(&queue->mutex);
    int is_empty = (queue->head == NULL);
    pthread_mutex_unlock(&queue->mutex);

    if (!is_empty) {
        // If queue is not empty, do not free anything
        return;
    }

    // Free all jobs in the archive
    Archive *archive = &queue->archive;
    Job *job = archive->head;
    while (job != NULL) {
        Job *next_job = job->next;
        if (job->arg != NULL) {
            free((job->arg)-7);
        }
        free(job);
        job = next_job;
    }

    // Free the mutex and condition variable
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->cond_q_empty);
    pthread_cond_destroy(&queue->cond_q_non_empty);

    // Free the queue
    free(queue);
}

// pthread create wrapper to create log file to each thread
void pthread_create_wrapper(pthread_t *thread, Queue *queue ,int create_log , int thread_num){
    ThreadData *thread_data = create_thread_data(queue, create_log, thread_num);
    pthread_create(thread, NULL, worker_thread, thread_data);
}

ThreadData *create_thread_data(Queue *queue, int create_log, int thread_num) {
    ThreadData *data = (ThreadData*) malloc(sizeof(ThreadData));
    data->queue = queue;
    data->create_log = create_log;
    data->thread_num = thread_num;
    return data;
}

void free_thread_data(ThreadData *data) {
    free(data);
}

void wait_for_queue_empty(Queue *queue) {
    pthread_mutex_lock(&queue->mutex);
    while (queue->head != NULL) {
        pthread_cond_wait(&queue->cond_q_empty, &queue->mutex);
    }
    pthread_mutex_unlock(&queue->mutex);
}
