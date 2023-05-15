#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <limits.h>


#include "job_queue.h"
#include "basic_commands.h"

// Create a new queue
Queue *create_queue(struct timeval start_time) {
    Queue *queue = (Queue*) malloc(sizeof(Queue));
    queue->head = NULL;
    queue->tail = NULL;
    queue->start_time = start_time;
    queue->active_threads_num = 0;

    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->cond_q_non_empty, NULL);
    pthread_cond_init(&queue->cond_q_empty, NULL);
    pthread_cond_init(&queue->cond_no_active_threads, NULL);

    return queue;
}

// Enqueue a job using mutex and condition variables 
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

// Dequeue a job using mutex and condition variables + Increment active threads
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
    
    //increment number of active threads
    queue->active_threads_num = queue->active_threads_num + 1;
    //printf("active threads: %d\n", queue->active_threads_num);

    Archive *archive = &queue->archive;
    job->next = archive->head;
    archive->head = job;
    archive->count++;

    pthread_mutex_unlock(&queue->mutex);
    return job;
}

// Worker thread function - a "container" function that each worker runs all the time
// that "pulls" jobs from queue
void *worker_thread(void *arg) {
    struct timeval start_time, end_time , current_time;
    ThreadData *data = (ThreadData*) arg;
    Queue *queue = data->queue;
    int create_log = data->create_log;
    int thread_num = data->thread_num;
    long long total_time;

    char log_filename[16];
    FILE *log_file = NULL;

    // create logger if needed
    if (create_log) {
        sprintf(log_filename, "thread%02d.txt", thread_num);
        log_file = fopen(log_filename, "w");
    }
    
    while (1) {
        Job *job = dequeue(queue);

        // if kill_job detected than end thread (get out of loop)
        if (job->function == kill){
            break;
        }
        
        // Do logging if needed
        if (create_log) {
            gettimeofday(&current_time, NULL);
            total_time = (current_time.tv_sec - queue->start_time.tv_sec) * 1000LL +
                (current_time.tv_usec - queue->start_time.tv_usec) / 1000LL;

            fprintf(log_file, "TIME %lld: START job %s\n", total_time, job->arg);
        }
        
        gettimeofday(&job->start_time, NULL);

        // print job name and thread number and number of active threads
        //printf("Thread %d: Running job \"%s\", active threads: %d\n", thread_num, job->arg, queue->active_threads_num);
         
        job->function(job->arg);
        gettimeofday(&job->end_time, NULL);
        
        //decrement active threads number using mutex
        decrement_active_threads(queue);

        
        // Do logging if needed
        if (create_log) {
            struct timeval current_time;
            gettimeofday(&current_time, NULL);
            total_time = (current_time.tv_sec - queue->start_time.tv_sec) * 1000LL +
                (current_time.tv_usec - queue->start_time.tv_usec) / 1000LL;

            fprintf(log_file, "TIME %lld: END job %s\n", total_time, job->arg);
        }

        
    }
    if (create_log) {
        fclose(log_file);
    }

    free_thread_data(data);
    return NULL;
}

// Decrement the active threads number
void decrement_active_threads(Queue *queue){
    pthread_mutex_lock(&queue->mutex);
    queue->active_threads_num--;
    // print number of active threads for debugging
    //printf("active threads: %d\n", queue->active_threads_num);
    if (queue->active_threads_num == 0){
        pthread_cond_signal(&queue->cond_no_active_threads);
    }

    pthread_mutex_unlock(&queue->mutex);
}

// ghost kill function to kill the kob in the end of the program
void kill(void *arg){
    
}

// print archive for debugging
void print_archive(Archive *archive) {
    printf("Job archive:\n");
    long long elapsed_time = 0;
    Job *job = archive->head;


    while (job != NULL) {
        elapsed_time = ((job->end_time.tv_sec - job->start_time.tv_sec) * 1000000LL +
                          (job->end_time.tv_usec - job->start_time.tv_usec));

        printf("Function: %s, Start time: %ld.%06ld, End time: %ld.%06ld, Running time: %lld microseconds\n",
                    (char*) job->arg, job->start_time.tv_sec, job->start_time.tv_usec,
                    job->end_time.tv_sec, job->end_time.tv_usec, elapsed_time);
        job = job->next;
    }
    printf("Total jobs in archive: %d\n", archive->count);
}

// print job stats for debugging
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

// print archive to file
void print_job_stats_to_file(Archive *archive, FILE *file) {
    // Calculate sum, min, max, and count of job turnaround times
    if (archive->head == NULL) {
        fprintf(file, "Job archive is empty\n");
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

    // Print job stats to file
    fprintf(file, "sum of jobs turnaround time: %lld milliseconds\n", sum);
    fprintf(file, "min job turnaround time: %lld milliseconds\n", min_time);
    fprintf(file, "average job turnaround time: %f milliseconds\n", average_time);
    fprintf(file, "max job turnaround time: %lld milliseconds\n", max_time);
}

// add cmnd job to queue
void add_cmnd_job(Queue *queue, char *cmnd){
    Job *job = (Job*) malloc(sizeof(Job));
    job->function = run_job;
    job->arg = cmnd;
    enqueue(queue, job);
}

// add kill job to queue
void add_kill_job(Queue *queue){
    Job *job = (Job*) malloc(sizeof(Job));
    job->function = kill;
    job->arg = NULL;
    job->start_time = (struct timeval) {0, 0};
    job->end_time = (struct timeval) {0, 0};
    enqueue(queue, job);
}

// free queue and all the other dynamic memory objects
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

// create thread data object that contains diffrent data for each thread
ThreadData *create_thread_data(Queue *queue, int create_log, int thread_num) {
    ThreadData *data = (ThreadData*) malloc(sizeof(ThreadData));
    data->queue = queue;
    data->create_log = create_log;
    data->thread_num = thread_num;
    return data;
}

// free thread data object
void free_thread_data(ThreadData *data) {
    free(data);
}

// wait for non pending commands - queue is empty and all no active threads
void wait_for_non_pending_command(Queue *queue) {
    pthread_mutex_lock(&queue->mutex);
    //printf("\n**** Waiting for queue to be empty ***\n\n");
    pthread_cond_wait(&queue->cond_q_empty, &queue->mutex);
    
    // if is meant to prevent deadlock for dequeueing jobs in edge condition
    if (queue->active_threads_num > 0) {
        //printf("\n*** Queue is empty, waiting for all threads to finish ***\n\n");
        pthread_cond_wait(&queue->cond_no_active_threads, &queue->mutex);
    }
    
    // else{
    //     printf("\n*** Queue is empty and no active threads ***\n\n");
    // }
    pthread_mutex_unlock(&queue->mutex);
}
