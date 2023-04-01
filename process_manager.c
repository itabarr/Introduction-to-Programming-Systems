#define MAX_PROCESSES 4

#include "process_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>


// Test code
// int main() {
//     process_manager* pm = create_process_manager();

//     // add some processes
//     add_process(pm, 1000, 1);
//     add_process(pm, 2000, 1);

//     // remove a process
//     int removed = remove_process(pm, 1000);
//     printf("Removed process 1000: %d\n", removed);


//     free_process_manager(pm);
//     return 0;
// }

// create process manager
process_manager *create_process_manager() {
    process_manager *ph = malloc(sizeof(process_manager));
    ph->num_processes = 0;

    for (int i = 0; i < MAX_PROCESSES; i++) {
        ph->processes[i].pid = -1;
        ph->processes[i].status = -1;
    }

    return ph;
}

// free process manager
void free_process_manager(process_manager *ph) {
    free(ph);
}

//check if process manager is full
int is_full(process_manager *ph) {
    return ph->num_processes == MAX_PROCESSES;
}

// add process to process manager and return 1 if successful, 0 if not 
int add_process(process_manager *ph, int pid, int status) {
    if (ph->num_processes < MAX_PROCESSES) {
        for (int i = 0; i < MAX_PROCESSES; i++) {
            if (ph->processes[i].pid == -1) {
                ph->processes[i].pid = pid;
                ph->processes[i].status = status;
                ph->num_processes++;
                return 1;
            }
        }
        
    }
    return 0;
}

//remove process by index and return 1 if successful, 0 if not
int remove_process_by_index(process_manager *ph, int index) {
    if (index < MAX_PROCESSES) {
        ph->processes[index].pid = -1;
        ph->processes[index].status = -1;
        ph->num_processes--;
        return 1;
    }
    return 0;
}

// remove a process from process manager if it's in zombie state, and print its pid
void update_process_manager(process_manager *ph) {
    int i;
    int status;
    int res;
    for (i = 0; i < ph->num_processes; i++) {
        // if process state is zombie, remove it and print hw1shell: pid %d finished
        if (ph->processes[i].status == 0) {
            res = remove_process_by_index(ph, i);
            if (res == 1) {
                printf("hw1shell: pid %d finished\n", ph->processes[i].pid);
            }
        }
    }
}

// print process manager
void print_process_manager(process_manager *ph) {
    printf("Number of processes: %d\n", ph->num_processes);
    for (int i = 0; i < MAX_PROCESSES; i++) {
        printf("Process %d: pid = %d, status = %d\n", i, ph->processes[i].pid, ph->processes[i].status);
    }
}

// update process status
void update_process_status(process_manager *ph, int pid, int status) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (ph->processes[i].pid == pid) {
            ph->processes[i].status = status;
        }
    }
}

