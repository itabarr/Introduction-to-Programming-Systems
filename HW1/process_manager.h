/* myfunctions.h */
#include <stdio.h>

#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H
#define MAX_PROCESSES 4
#define MAX_LINE 1024

// create process handler struct
struct process {
    int pid;
    int status;
    char cmd[MAX_LINE];
    FILE *fp;
};

// create process handler struct
struct process_manager {
    int num_processes;
    struct process processes[MAX_PROCESSES];
};

typedef struct process process;
typedef struct process_manager process_manager;

// function declarations
process_manager *create_process_manager(void);
void free_process_manager(process_manager *ph);
int is_full(process_manager *ph);
void add_process(process_manager *ph, int pid, char *cmd , FILE *fp);
void remove_process(process_manager *ph, int pid);
int is_running(int pid);
void clean_finished_processes(process_manager *ph);
void print_process_manager(process_manager *ph);
void kill_all_processes(process_manager *ph);
#endif /* MY_FUNCTIONS_H */

