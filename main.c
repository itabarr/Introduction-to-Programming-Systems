#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS_GLOBALS
#define NUM_COUNTERS  100
#define NUM_THREADS 4096
#define MAX_JOB_WDT 1024
#define COUNTER_FILE_NAME_LENGTH 15


#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <inttypes.h>
#include <pthread.h>
#include <ctype.h>
#include <time.h>


int is_not_worker(char* command) { // To check is the first term is for worker, else it is a dispatcher
    if (strcmp(command, 'worker') > 0) {
        return 1;
	}
	return 0;
}

void create_counters(int num_counters) {
    char name[COUNTER_FILE_NAME_LENGTH];
    for (int i = 0; i < num_counters; i++) {
        char *filename;
        asprintf(&filename, "count%02d.txt", i);
        FILE *fp = fopen(filename, "w");
        if (fp != NULL) {
            fprintf(fp, "%lld", 0LL);
            fclose(fp);
        }
        free(filename);
    }
}


void *thread_function(void *arg){
    wait for number to not be zero:

    
        Queue q = (Queue *) arg
        pthread_mutex_lock
        take_job_from_queue, and remove job;
        
        decrement num of jobs
        thread_unlock

}

void create_threads(pthread_t* thread_ptrs, int num_threads) {
    for (int i = 0; i < num_threads; i++) {
        create_logs(i);
        if (pthread_create(&thread_ptrs[i], NULL, thread_function, args) != 0) {
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

void find_workers(char* line) { //TODO change the splitting
	char first_term[50] = { 0 };
	char second_term[50] = { 0 };
	char third_term[50] = { 0 };
	char fourth_term[50] = { 0 };
	char fifth_term[50] = { 0 };
	char word_op[6] = { 0 };

	if (is_not_worker(first_term)) {
        //TODO add dispatcher functions
	}
	else {	//found a worker  
		char* clean_worker = strtok(first_term, " ");
        //TODO send the clean_worker str to the queue 
	}
}

int main(int argc, char *argv[]) {
	FILE* fp;
	char line[MAX_JOB_WDT];
	char buffer[MAX_JOB_WDT];
	int num_threads = 0;
	int num_counters = 0;
    int log_enabled = 0;
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
    log_enabled = atoi(argv[5]);
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
    
    // creating the thread's pointers array and the threads themselves
    pthread_t thread_ptrs[num_threads];
    create_threads(thread_ptrs, num_threads);




	printf("created counter files, threads and times array. starting parsing file!\n");

	// executing stage
	while (fgets(buffer, MAX_JOB_WDT, fp)) {// running over the txt file, execute dispatch and send workers
		find_workers(buffer);
	}
	fclose(fp);
	if (!(fp = fopen(argv[1], "r"))) {//Opening the file for the second time to fill memin
		exit(EXIT_FAILURE);
	}
    return 0;
}

