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

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS_GLOBALS
#define NUM_COUNTERS  100
#define NUM_THREADS 4096
#define MAX_JOB_WDT 1024
#define COUNTER_FILE_NAME_LENGTH 15




//helpers

int is_not_worker(char* first_term) { // To check is the first term is for worker, else it is a dispatcher
    if (strncmp(first_term, "worker", 6) != 0) {
        return 1;
	}
	return 0;
}

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

void create_logs(int thread_num, int save_logs) {
    if (save_logs == 0) {
        return;
    }
    char name[13];
    sprintf(name, "thread%02d.txt", thread_num);
    FILE *fp = fopen(name, "w");
    if (fp != NULL) {
        fprintf(fp, "%lld", 0LL);
        fclose(fp);
    }
}


void create_threads(pthread_t* thread_ptrs,struct Queue* queue, int num_threads, int save_logs) { //TODO waiting for Itamar's wrapper
    for (int i = 0; i < num_threads; i++) {
        pthread_create_wrapper(&thread_ptrs[i], queue, save_logs, i);
    }
}

int count_worker_lines(FILE* fp) {
    char line[MAX_JOB_WDT];
    int count = 0;
    
    while (fgets(line, MAX_JOB_WDT, fp)) {
        if (strncmp(line, "worker", 6) == 0) {
            count++;
        }
    }
    
    return count;
}

void dispatcher_command(char*line, int save_logs, struct Queue *queue){ 
    // Extract the command and argument from line
    char* start = line + 11; // Skip "dispatcher_" prefix
    struct timeval start_time;
    struct timeval stop_time;
    gettimeofday(&start_time, NULL); // taking the start_time of the dispatcher_command

    // Executing
    if (strncmp(start, "msleep", 6) == 0) {
        int time = atoi(start+7);
        usleep(time * 1000);
    } else if (strcmp(start, "wait") == 0) {
        // Wait for all pending background commands to complete before continuing to process the next line in the input file
        wait_for_queue_empty(queue);
    } else {
        // Unknown dispatcher command
        printf("Error in dispatcher command");
    }
        
    gettimeofday(&stop_time, NULL);   // taking the start_time of the dispatcher_command
    // Creating and writing to log_file if needed
    if (save_logs == 1) {  
        // calculate elapsed time

        long long elapsed_time = ((stop_time.tv_sec - start_time.tv_sec) * 1000LL) + ((stop_time.tv_usec - start_time.tv_usec) / 1000LL);

        // Open the log file for appending
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

void handle_command(char* line, int save_logs, Queue *queue) { 
    /// *** BUG FOUND HERE IN REMOVING NEW LINE CHARACTER WHEN READING LAST LINE OF FILE *** 

    line[strlen(line) - 1] = '\0'; // Replacing the newline character from the string with null character1

    char first_term[7] = { 0 };
    strncpy(first_term, line, 6);  // Get the first 6 characters of the line
	if (is_not_worker(first_term)) {
        dispatcher_command(line, save_logs, queue);
	}
	else {	//found a worker  
		char* clean_worker_job = line + 7; 
        add_cmnd_job(queue, clean_worker_job);
	}
}

void create_stats_file(long long total_elapsed_time) {
    FILE *file = fopen("stats.txt", "w");
    if (file == NULL) {
        printf("Error opening file\n");
        return;
    }
    fprintf(file, "total running time: %lld milliseconds", total_elapsed_time);
    fclose(file);
}


//main program 

int main(int argc, char *argv[]) {
	FILE* fp;
	char line[MAX_JOB_WDT];
	char raw_command_line[MAX_JOB_WDT];
	int num_threads = 0;
	int num_counters = 0;
    int save_logs = 0;
    int finish_file = 0;
    struct timeval total_start_time;
    struct timeval total_end_time;
    gettimeofday(&total_start_time, NULL); // taking the total_start_time of the whole program
    Queue *queue = create_queue();
                                                    //TODO take real args
    // // checking for command line errors
    // if (strcmp(argv[1], "hw2") != 0) { 
    //     printf("Error while writing command. 'hw2' should be instead of '%s'", argv[1]);
    //     return 1; 
    // }
	// if (argc < 5 || !(fp = fopen(argv[2], "r"))) {
	// 	printf("Error while opening the file");
	// 	exit(1);
	// 	return 1;
	// }

    // // Parsing the command
    // num_threads = atoi(argv[3]);
    // num_counters = atoi(argv[4]);
    // save_logs = atoi(argv[5]);
	// fp = fopen(argv[2], "r");

    num_threads = 3;
    num_counters = 4;
    save_logs = 1;
	fp = fopen("command_file.txt", "r");

    //checking for input errors
    if (fp == NULL) {
        printf("Error while opening the file\n");
        return 1;
    }
    if (num_threads < 0 || num_threads >= NUM_THREADS) {
        printf("Wrong number of threads: %d", num_threads);
        return 1;
    }
    if (num_counters < 0 || num_counters >= NUM_COUNTERS) {
        printf("Wrong number of counters: %d", num_counters);
        return 1;
    }

    //creating the counter files by their names
    create_counter_files(num_counters);
    
    // creating the thread's pointers array and the threads themselves (also logs when needed)
    pthread_t thread_ptrs[num_threads];
    create_threads(thread_ptrs, queue, num_threads, save_logs);

	printf("created counter and logs files, threads and times array. starting parsing file!\n");

	// executing stage
	while (fgets(raw_command_line, MAX_JOB_WDT, fp)) {// running over the txt file, execute dispatch and send workers
		handle_command(raw_command_line, save_logs, queue); 
	}
	fclose(fp);

    //*** Send kill jobs to queue to kill threads - need to sent num_of_threads kill jobs ***
    for (int i = 0; i < num_threads; i++) { 
        add_kill_job(queue);
    }
    
    //*** Wait for threads to finish ***           
    for (int i = 0; i < num_threads; i++) { 
        pthread_join(thread_ptrs[i], NULL);  
    }
    
    gettimeofday(&total_end_time, NULL); // taking the total_end_time of the whole program
    long long total_elapsed_time = ((total_end_time.tv_sec - total_start_time.tv_sec) * 1000LL) + ((total_end_time.tv_usec - total_start_time.tv_usec) / 1000LL);

    // print_job_stats(&queue->archive); //TODO edit create_stats_file to receive print_job_stats values

    create_stats_file(total_elapsed_time);

    // *** Free queue (and it's archive and jobs) ***
    free_queue(queue);



    return 0;
}

