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
    pthread_cond_init(&queue->cond, NULL);
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
    pthread_cond_signal(&queue->cond);
    pthread_mutex_unlock(&queue->mutex);
}

Job *dequeue(Queue *queue) {
    pthread_mutex_lock(&queue->mutex);
    while (queue->head == NULL) {
        pthread_cond_wait(&queue->cond, &queue->mutex);
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
    Queue *queue = (Queue*) arg;
    while (1) {
        Job *job = dequeue(queue);

        if (job->function == kill){
            break;
        }

        gettimeofday(&job->start_time, NULL);
        job->function(job->arg);
        gettimeofday(&job->end_time, NULL);
    }
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
        free(job);
        job = next_job;
    }

    // Free the mutex and condition variable
    pthread_mutex_destroy(&queue->mutex);
    pthread_cond_destroy(&queue->cond);

    // Free the queue
    free(queue);
}


//Comment To Frumkis:

int main() {
    
    //*** Init queue + jobs***
    Queue *queue = create_queue();
    pthread_t worker1, worker2;
    int num_of_threads = 2;
    pthread_create(&worker1, NULL, worker_thread, queue);
    pthread_create(&worker2, NULL, worker_thread, queue);

    //*** Create jobs arr ***
    char *command_strings[] = {
        "increment 1;msleep 2;decrement 1",
        "repeat 3;increment 2;msleep 5;decrement 1",
        "repeat 3;increment 2;msleep 5;decrement 1",
    };
    
    //*** Send jobs to queue whenever you want,threads will try to take jobs, no need to handle ***
    for (int i = 0; i < 3; i++) {
        add_cmnd_job(queue, command_strings[i]);
    }
    
    //*** Send kill jobs to queue to kill threads - need to sent num_of_threads kill jobs ***
    for (int i = 0; i < num_of_threads; i++) { 
        add_kill_job(queue);
    }
    

    //*** Wait for threads to finish ***
    pthread_join(worker1, NULL);
    pthread_join(worker2, NULL);
    print_archive(&queue->archive);
    print_job_stats(&queue->archive);
    // *** Free queue (and it's archive and jobs) ***
    free_queue(queue);
    return 0;
}