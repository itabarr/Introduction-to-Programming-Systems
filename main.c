#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <inttypes.h>
#include <pthread.h>
#include <ctype.h>
#include <time.h>
#include <sys/time.h>

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS_GLOBALS
#define NUM_COUNTERS  100
#define NUM_THREADS 4096
#define MAX_JOB_WDT 1024
#define COUNTER_FILE_NAME_LENGTH 15




//helpers

int is_not_worker(char* command) { // To check is the first term is for worker, else it is a dispatcher
    if (strcmp(command, 'worker') > 0) {
        return 1;
	}
	return 0;
}

void create_counters(int num_counters) {
    char name[COUNTER_FILE_NAME_LENGTH];
    for (int i = 0; i < num_counters; i++) {
        char *name;
        asprintf(&name, "count%02d.txt", i);
        FILE *fp = fopen(name, "w");
        if (fp != NULL) {
            fprintf(fp, "%lld", 0LL);
            fclose(fp);
        }
        free(name);
    }
}

void create_logs(int thread_num, int save_logs) {
    if (save_logs == 0) {
        return;
    }
    char name[13];
        asprintf(&name, "count%02d.txt", thread_num);
        FILE *fp = fopen(name, "w");
        if (fp != NULL) {
            fprintf(fp, "%lld", 0LL);
            fclose(fp);
        }
        free(name);
}

void *thread_function(void *args){
    wait for number to not be zero:

    
        Queue q = (Queue *) arg
        pthread_mutex_lock
        take_job_from_queue, and remove job;

        decrement num of jobs
        thread_unlock

}

void create_threads(pthread_t* thread_ptrs, int num_threads, int save_logs) {
    for (int i = 0; i < num_threads; i++) {
        create_logs(i, save_logs);  
        if (pthread_create(&thread_ptrs[i], NULL, thread_function, NULL) != 0) {
            printf("Error while creating the thread %d", i);
            exit(EXIT_FAILURE);
        }
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

void dispatcher_command(char*line, int save_logs, struct work_queue *work_queue){
    // Extract the command and argument from line
    char* start = line + 11; // Skip "dispatcher_" prefix
    char* end = strchr(start, '_'); // Find the first '_' character
    if (!end) {
        // Missing argument
        printf("Missing argument for dispatcher command");
        return;
    }
    *end = '\0';
    int time = atoi(end+1);
    // Creating and writing to log_file if needed
    if (save_logs == 1):                                     // TODO take Itamar's way of measuring start time and current time
        // Get current time and calculate elapsed time
        struct timeval current_time;
        struct timeval START_TIME, current_time;
        gettimeofday(&START_TIME, NULL);
        gettimeofday(&current_time, NULL);
        long long elapsed_time = ((current_time.tv_sec - START_TIME.tv_sec) * 1000LL) + ((current_time.tv_usec - START_TIME.tv_usec) / 1000LL);

        // Open the log file for appending
        FILE* log_file = fopen("dispatcher.txt", "a");
        if (log_file == NULL) {
            printf("Failed to open log file\n");
            return;
        }
        // Write to the log file
        fprintf(log_file, "TIME %lld: read cmd line: %s\n", elapsed_time, line);
        fclose(log_file);


    // Executing
    if (strncmp(start, "msleep", 6) == 0) {
        usleep(time * 1000);
    } else if (strcmp(start, "wait") == 0) {
        // Wait for all pending background commands to complete before continuing to process the next line in the input file
        while (!is_empty(work_queue)) {
            usleep(1000);
        }
    } else {
        // Unknown dispatcher command
        printf("Unknown dispatcher command: %s\n", line);
    }
}

void find_workers(char* line, int save_logs, struct work_queue *work_queue) { 
    // Get the first 6 characters of the line
    line[strcspn(line, "\n")] = 0; // TODO check if affects, saw that in Almog's work - Replacing the newline character from the string with null character
    // line[strlen(line) - 1] = '\0'; //TODO another implementation for Almog's work. If necessary, this one better
    char first_term[7] = { 0 };
    strncpy(first_term, line, 6);
	if (is_not_worker(first_term)) {
        dispatcher_command(line, save_logs, work_queue);
	}
	else {	//found a worker  
		char* clean_worker = strtok(line, " ");
        //TODO send the clean_worker str to the queue 
	}
}


//main program 

int main(int argc, char *argv[]) {
	FILE* fp;
	char line[MAX_JOB_WDT];
	char buffer[MAX_JOB_WDT];
	int num_threads = 0;
	int num_counters = 0;
    int save_logs = 0;
    int finish_file = 0;

    // checking for command line errors
    if (strcmp(argv[1], "hw2") != 0) { 
        printf("Error while writing command. 'hw2' should be instead of '%s'", argv[1]);
        return 1; 
    }
	if (argc < 5 || !(fp = fopen(argv[2], "r"))) {
		printf("Error while opening the file");
		exit(1);
		return 1;
	}

    // Parsing the command
    num_threads = atoi(argv[3]);
    num_counters = atoi(argv[4]);
    save_logs = atoi(argv[5]);
	fp = fopen(argv[2], "r");

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

    //creating time array and reset the pointer to the start of the file
    int num_worker_lines = count_worker_lines(fp);
    float* time_arr = (float*) malloc(num_worker_lines * sizeof(float));
    fseek(fp, 0, SEEK_SET);
            // TODO do not forget to :  free(arr);

    //creating the counter files by their names
    create_counters(num_counters);
    
    // creating the thread's pointers array and the threads themselves (also logs when needed)
    pthread_t thread_ptrs[num_threads];
    create_threads(thread_ptrs, num_threads, save_logs);

	printf("created counter and logs files, threads and times array. starting parsing file!\n");

	// executing stage
	while (fgets(buffer, MAX_JOB_WDT, fp)) {// running over the txt file, execute dispatch and send workers
		find_workers(buffer, save_logs, work_queue);  //TODO think about how to synchronize the work_queue with the worker function
	}
	fclose(fp);

//TODO add statistics

    return 0;
}

