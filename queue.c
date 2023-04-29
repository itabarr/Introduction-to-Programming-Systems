#include "queue.h"

// Implementation of the Queue functions
Queue *create_queue() {
    Queue *queue = (Queue*) malloc(sizeof(Queue));
    queue->head = NULL;
    queue->tail = NULL;
    return queue;
}

void enqueue(Queue *queue, char *job_name) {
    Job *new_job = (Job*) malloc(sizeof(Job));
    new_job->name = (char*) malloc(strlen(job_name) + 1);
    strcpy(new_job->name, job_name);
    new_job->next = NULL;

    if (queue->tail == NULL) {
        queue->head = new_job;
        queue->tail = new_job;
    } else {
        queue->tail->next = new_job;
        queue->tail = new_job;
    }
}

char *dequeue(Queue *queue) {
    if (queue->head == NULL) {
        return NULL;
    }

    Job *job = queue->head;
    char *job_name = job->name;

    if (queue->head == queue->tail) {
        queue->head = NULL;
        queue->tail = NULL;
    } else {
        queue->head = job->next;
    }

    free(job);
    return job_name;
}

void print_queue(Queue *queue) {
    if (queue->head == NULL) {
        printf("Queue is empty\n");
        return;
    }

    printf("Queue contents:\n");
    Job *current_job = queue->head;
    while (current_job != NULL) {
        printf("%s\n", current_job->name);
        current_job = current_job->next;
    }
}

// Test the Queue functions
int main() {
    // Create a new queue
    Queue *queue = create_queue();

    // Add some jobs to the queue
    enqueue(queue, "Job 1");
    enqueue(queue, "Job 2");
    enqueue(queue, "Job 3");
    enqueue(queue, "Job 4");

    // Print the contents of the queue
    printf("After enqueue:\n");
    print_queue(queue);

    // Remove a job from the queue
    char *removed_job = dequeue(queue);
    printf("Removed job: %s\n", removed_job);

    // Print the contents of the queue again
    printf("After dequeue:\n");
    print_queue(queue);

    // Add another job to the queue
    enqueue(queue, "Job 5");

    // Print the contents of the queue one more time
    printf("After another enqueue:\n");
    print_queue(queue);

    // Remove all jobs from the queue
    while (queue->head != NULL) {
        char *removed_job = dequeue(queue);
        printf("Removed job: %s\n", removed_job);
    }

    // Print the contents of the empty queue
    printf("After dequeueing all jobs:\n");
    print_queue(queue);

    return 0;
}