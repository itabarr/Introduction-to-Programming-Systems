#define MAX_PROCESSES 4

#include "process_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    process_manager* pm = create_process_manager();

    // add some processes
    add_process(pm, 1000, 1);
    add_process(pm, 2000, 1);

    // remove a process
    int removed = remove_process(pm, 1000);
    printf("Removed process 1000: %d\n", removed);


    free_process_manager(pm);
    return 0;
}

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

// remove process from process manager and return 1 if successful, 0 if not
int remove_process(process_manager *ph, int pid) {
    int i;
    for (i = 0; i < ph->num_processes; i++) {
        if (ph->processes[i].pid == pid) {
            ph->processes[i].pid = -1;
            ph->processes[i].status = -1;
            ph->num_processes--;
            return 1;
        }
    }
    return 0;
}


// get process status by pid from linux /proc filesystem
int get_process_status(int pid) {
    char path[100];
    sprintf(path, "/proc/%d/status", pid);
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        return -1;
    }

    char line[100];
    while (fgets(line, 100, fp) != NULL) {
        if (strncmp(line, "State:", 6) == 0) {
            fclose(fp);
            return line[7];
        }
    }

    fclose(fp);
    return -1;
}

// get all processes child pids from linux /proc filesystem
int *get_child_pids(int pid) {
    char path[100];
    sprintf(path, "/proc/%d/task/%d/children", pid, pid);
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        return NULL;
    }

    int *pids = malloc(sizeof(int) * 100);
    int i = 0;
    char line[100];
    while (fgets(line, 100, fp) != NULL) {
        pids[i] = atoi(line);
        i++;
    }

    fclose(fp);
    return pids;
}