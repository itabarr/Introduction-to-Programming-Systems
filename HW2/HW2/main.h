#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <inttypes.h>
#include <pthread.h>
#include <unistd.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>

#define NUM_COUNTERS  100
#define NUM_THREADS 4096
#define MAX_JOB_WDT 1024
#define COUNTER_FILE_NAME_LENGTH 15

// Helpers
int is_not_worker(char* first_term);
void create_counter_files(int num_counters);
void create_threads(pthread_t* thread_ptrs,struct Queue* queue, int num_threads, int save_logs);
void dispatcher_command(char*line, int save_logs, struct Queue *queue , struct timeval start_time);
void handle_command(char* line, int save_logs, Queue *queue , struct timeval start_time);
void initialize_mutex_list(int num_counters);
void destroy_mutex_list(int num_counters);

// Stats
FILE* create_stats_file();
void write_stats_file(long long total_elapsed_time, FILE *file);

// Main Program
int main(int argc, char *argv[]);

#endif
