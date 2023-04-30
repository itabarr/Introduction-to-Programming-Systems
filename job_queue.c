#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
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
    pthread_mutex_unlock(&queue->mutex);
    return job;
}

void *worker_thread(void *arg) {
    Queue *queue = (Queue*) arg;
    while (1) {
        Job *job = dequeue(queue);

        if (job->function == kill){
            free(job);
            break;
        }

        job->function(job->arg);
        free(job);
    }
    return NULL;
}

void kill(void *arg){
    
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
        Job *job = (Job*) malloc(sizeof(Job));
        job->function = run_job;
        job->arg = command_strings[i]; // this is the part you enter the command string
        enqueue(queue, job);
    }
    
    //*** Send kill jobs to queue to kill threads - need to sent num_of_threads kill jobs ***
    for (int i = 0; i < num_of_threads; i++) { 
        Job *job = (Job*) malloc(sizeof(Job));
        job->function = kill;
        job->arg = " ";
        enqueue(queue, job);
    }
    
    //*** Wait for threads to finish ***
    pthread_join(worker1, NULL);
    pthread_join(worker2, NULL);

    return 0;
}