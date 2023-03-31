/* myfunctions.h */
#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

// create process handler struct
struct process {
    int pid;
    int status;
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
int add_process(process_manager *ph, int pid, int status);
int remove_process(process_manager *ph, int pid);
int get_process_status(int pid);
int *get_child_pids(int pid);


#endif /* MY_FUNCTIONS_H */

