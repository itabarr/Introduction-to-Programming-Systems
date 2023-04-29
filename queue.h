#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Job structure definition
typedef struct Job {
    char *name;
    struct Job *next;
} Job;

// Queue structure definition
typedef struct Queue {
    Job *head;
    Job *tail;
} Queue;

// Function prototypes
Queue *create_queue();
void enqueue(Queue *queue, char *job_name);
char *dequeue(Queue *queue);
void print_queue(Queue *queue);

#endif /* QUEUE_H */



