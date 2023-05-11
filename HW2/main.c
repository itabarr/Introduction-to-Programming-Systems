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
#include "job_queue.h"
#include "main.h"

#define NUM_COUNTERS  100
#define NUM_THREADS 4096
#define MAX_JOB_WDT 1024
#define COUNTER_FILE_NAME_LENGTH 15

// check if the line is not a worker command
int is_not_worker(char* first_term) { 
    if (strncmp(first_term, "worker", 6) != 0) {
        return 1;
	}
	return 0;
}

// create counter files given the number of counters
void create_counter_files(int num_counters) {
    char name[COUNTER_FILE_NAME_LENGTH];
    for (int i = 0; i < num_counters; i++) {
        snprintf(name, COUNTER_FILE_NAME_LENGTH, "count%02d.txt", i);
        FILE *fp = fopen(name, "w");
        if (fp != NULL) {
            fprintf(fp, "%lld", 0LL);
            fclose(fp);
        }
    }
}

// create threads given the number of threads, the queue and the save_logs flag
void create_threads(pthread_t* thread_ptrs,struct Queue* queue, int num_threads, int save_logs) { 
    for (int i = 0; i < num_threads; i++) {
        pthread_create_wrapper(&thread_ptrs[i], queue, save_logs, i);
    }
}

// do the dipacher command logic
void dispatcher_command(char* line, int save_logs, struct Queue *queue , struct timeval start_time){ 
    
    char* start = line + 11; 
    struct timeval read_time;

    // Handle the dispatcher commands logic
    if (strncmp(start, "msleep", 6) == 0) {

        int time = atoi(start+7);
        usleep(time * 1000);

    } else if (strcmp(start, "wait") == 0) {
        
        //printf("Waiting for queue to be empty\n");
        wait_for_queue_empty(queue);
   
    } else {    

        printf("Error in dispatcher command");
    }
        
    gettimeofday(&read_time, NULL);  
    
    // Handle logging for the dispatcher 
    if (save_logs == 1) {  
        // print start time and read time for debugging (all the object)\

        long long elapsed_time = (read_time.tv_sec - start_time.tv_sec) * 1000LL + (read_time.tv_usec - start_time.tv_usec) / 1000LL;

        FILE* log_file = fopen("dispatcher.txt", "a");
        if (log_file == NULL) {
            printf("Failed to open log file\n");
            return;
        }
        // Write to the log file
        fprintf(log_file, "TIME %lld: read cmd line: %s\n", elapsed_time, line);
        fclose(log_file);
    }
}

// handle the command given the line from the cmnd file
void handle_command(char* line, int save_logs, Queue *queue , struct timeval start_time) {
    char* cmnd = (char*) malloc(MAX_JOB_WDT * sizeof(char));
    strcpy(cmnd, line);

    // cmnd parsing 
    if (cmnd[strlen(cmnd) - 1] == '\n' ){
        cmnd[strlen(cmnd) - 1] = '\0'; 
    }
    char first_term[7] = { 0 };
    strncpy(first_term, cmnd, 6);  
	
    //cmnd logic
    if (is_not_worker(first_term)) {
        dispatcher_command(cmnd, save_logs, queue , start_time);
        free(cmnd);
	}

	else {  
		char* clean_worker_job = cmnd + 7; 
        add_cmnd_job(queue, clean_worker_job);
	}
}
 
// create stats file and return the file pointer
FILE* create_stats_file() {
    FILE *file = fopen("stats.txt", "w");
    if (file == NULL) {
        printf("Failed to open stats file\n");
        return NULL;
    }
    return file;
}

// close stats file given the total elapsed time and the file pointer
void write_stats_file(long long total_elapsed_time, FILE *file) {
    fprintf(file, "total running time: %lld milliseconds\n", total_elapsed_time);
}

// main function
int main(int argc, char *argv[]) {
	FILE* fp;
	char line[MAX_JOB_WDT];
	char raw_command_line[MAX_JOB_WDT];
	int num_threads;
	int num_counters;
    int save_logs;
    
    struct timeval total_start_time;
    struct timeval total_end_time;
    gettimeofday(&total_start_time, NULL); // taking the total_start_time of the whole program
    Queue *queue = create_queue(total_start_time);

    // remove the stats file if it exists 
    remove("dispatcher.txt");


    // args parsing
    if (argc < 5){
        printf("Error: not enough arguments");
        exit(1);
    }
    
    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        printf("Error while opening file %s\n" , argv[1]);
        exit(1);
    }
	
    num_threads = atoi(argv[2]);
    num_counters = atoi(argv[3]);
    save_logs = atoi(argv[4]);
	
    //checking for input errors
    if (num_threads < 1 || num_threads > NUM_THREADS) {
        printf("Wrong number of threads: %d", num_threads);
        exit(1);
    }
    if (num_counters < 1 || num_counters > NUM_COUNTERS) {
        printf("Wrong number of counters: %d", num_counters);
        exit(1);
    }

    //creating the counter files by their names
    create_counter_files(num_counters);
    
    // creating the thread's pointers array and the threads themselves (also logs when needed)
    pthread_t thread_ptrs[num_threads];
    create_threads(thread_ptrs, queue, num_threads, save_logs);

	// executing stage
	while (fgets(raw_command_line, MAX_JOB_WDT, fp)) {// running over the txt file, execute dispatch and send workers
		handle_command(raw_command_line, save_logs, queue , total_start_time); 
	}
	fclose(fp);

    //Send kill jobs to queue to kill threads - need to sent num_of_threads kill jobs
    for (int i = 0; i < num_threads; i++) { 
        add_kill_job(queue);
    }
    
    // Wait for threads to finish        
    for (int i = 0; i < num_threads; i++) { 
        pthread_join(thread_ptrs[i], NULL);  
    }

    // Take total time and create stats files    
    gettimeofday(&total_end_time, NULL);
    long long total_elapsed_time = ((total_end_time.tv_sec - total_start_time.tv_sec) * 1000LL) + ((total_end_time.tv_usec - total_start_time.tv_usec) / 1000LL);

    FILE *stats_file = create_stats_file();
    write_stats_file(total_elapsed_time, stats_file);
    print_job_stats_to_file(&queue->archive, stats_file);
    fclose(stats_file); 

    //print_archive(&queue->archive);
    
    // Free queue (and it's archive and jobs)
    free_queue(queue);

    return 0;
}

