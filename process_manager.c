#define MAX_PROCESSES 4

#include "process_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>



// create process manager
process_manager *create_process_manager() {
    process_manager *ph = malloc(sizeof(process_manager));
    ph->num_processes = 0;

    for (int i = 0; i < MAX_PROCESSES; i++) {
        ph->processes[i].pid = -1;
        ph->processes[i].status = -1;
        ph->processes[i].cmd[0] = '\0';
        ph->processes[i].fp = NULL;
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
void add_process(process_manager *ph, int pid , char *cmd , FILE *fp) {
    if (ph->num_processes < MAX_PROCESSES) {
        for (int i = 0; i < MAX_PROCESSES; i++) {
            if (ph->processes[i].pid == -1) {
                ph->processes[i].pid = pid;
                ph->processes[i].status = 1;
                strcpy(ph->processes[i].cmd, cmd);
                ph->num_processes++;
                ph->processes[i].fp = fp;
                return;
            }
        }
        
    }
    
}

// check if process is running using waitpid and WHOHANG
int is_running(int pid) {
    int status;
    if (waitpid(pid, &status, WNOHANG) == 0) {
        return 1;
    }
    return 0;
}

// clear finished-processes from process manager
void clean_finished_processes(process_manager *ph) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (ph->processes[i].pid != -1 && is_running(ph->processes[i].pid) == 0) {
            printf("hw1shell: pid %d finished\n", ph->processes[i].pid);
            
            ph->processes[i].pid = -1;
            ph->processes[i].status = -1;
            ph->processes[i].cmd[0] = '\0';
            
            if (ph->processes[i].fp != NULL){
                fclose(ph->processes[i].fp);
            }
            ph->processes[i].fp = NULL;
            ph->num_processes--;
        }
    }
}

// print process manager and all processes
void print_process_manager(process_manager *ph) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (ph->processes[i].pid != -1) {
            printf("pid\t%d\t%s\n", ph->processes[i].pid,ph->processes[i].cmd);
        }
    }
}

// kill all remaining processes and wait for them to finish
void kill_all_processes(process_manager *ph) {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (ph->processes[i].pid != -1) {
            kill(ph->processes[i].pid, SIGKILL);
            if (ph->processes[i].fp != NULL){
                fclose(ph->processes[i].fp);
            }
            waitpid(ph->processes[i].pid, NULL, 0);
        }
    }

    //clean_finished_processes(ph);
    //print_process_manager(ph);
}

