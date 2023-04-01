#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "process_manager.h"

#define MAX_LINE 1024
#define MAX_ARGS 64
#define MAX_PROCESSES 4

// function declarations
void print_prompt(void);
void read_command(char *cmd);
void exit_program(int *run, char *cmd, process_manager *ph);
void change_directory(char *cmd);
void parse_command(char *cmd, char *args[], int *is_background, int *is_internal);
int is_internal_command(char *cmd);
int is_background_command(char *last_arg);
void run_external_process(char *args[], int is_background, process_manager *ph);
// void handle_background(char *cmd); 

int main() {
    int run = 1;
    char cmd[MAX_LINE];
    char *args[MAX_ARGS];

    int is_internal;
    int is_background;

    process_manager *ph = create_process_manager();

    while (run) {
        print_prompt(); 

        read_command(cmd);
        parse_command(cmd, args, &is_background, &is_internal);

        run_external_process(args, is_background , ph);
        
        //print_process_manager(ph);
        update_process_manager(ph);

        exit_program(&run, cmd, ph);
        change_directory(cmd); 
    }

    return 0;
}

// prints prompt to console
void print_prompt(void) {
    printf("hw1shell$ "); 
}

//reads command from console
void read_command(char *cmd) {
    fgets(cmd, MAX_LINE, stdin);
    cmd[strlen(cmd) - 1] = '\0';
}

// parse command into arguments
void parse_command(char *cmd, char *args[] , int *is_background, int *is_internal) {
    char separator[] = " ";
    char *parsed_cmd;
    int i = 0;

    parsed_cmd = strtok(cmd, separator);
    while (parsed_cmd != NULL) {
        args[i] = parsed_cmd;
        parsed_cmd = strtok(NULL, separator);
        i++;
    }
    args[i] = NULL;

    *is_background = is_background_command(args[i-1]);
    *is_internal = is_internal_command(args[0]);

    // remove the & from the args if it is a background command
    if (*is_background) {
        args[i - 1] = NULL;
    }
}

// checks if command is a internal command
int is_internal_command(char *cmd) {
    if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "cd") == 0 || strcmp(cmd, "jobs") == 0) {
        return 1;
    }
    return 0;
}

// check if is a background command given the last string argument 
int is_background_command(char *last_arg) {
    if (strcmp(last_arg, "&") == 0){
        return 1;
    }
    return 0;
}

// checks if user typed exit command
void exit_program(int *run, char *cmd , process_manager *ph) {
    if (strcmp(cmd, "exit") == 0) {
        *run = 0;
        free_process_manager(ph);
    }
    
}

// changes the directory if command is cd
void change_directory(char *cmd){
    char separator[] = " ";
    char *parsed_cmd;
    parsed_cmd = strtok(cmd,separator);

    if (parsed_cmd==NULL){
        return;
    }
    if(strcmp(parsed_cmd,"cd")==0){
        parsed_cmd = strtok(NULL,separator);
        if(parsed_cmd==NULL){
            return;
        }
        if(chdir(parsed_cmd)!=0){
            printf("hw1shell: invalid command\\n");
        }
  }
return;
}

//run external process with fork and execvp
void run_external_process(char *args[], int is_background, process_manager *ph) {
    pid_t pid;
    int status;
    int is_process_added;
    
    if (is_background && is_full(ph)){
        printf("hw1shell: too many background commands running\n");
        return;

    }

    pid = fork();

    if (pid == 0) {
        
        if (is_background){
            printf("\nchild: pid %d started\n", getpid());
            
            //print process manager before adding process

            is_process_added = add_process(ph, getpid(), 1);

            //print process manager pointer after adding process
            printf("pm pointer: %p\n", ph);
        }

        execvp(args[0], args);

        if (is_background){
            update_process_status(ph, getpid(), 0);
        }

        perror("execvp() failed");
        exit(1);

    } else if (pid > 0) {
        // Parent process
        if (!is_background) {
            // print pid of child process
            waitpid(pid, &status, 0); 
        }

        else{
            printf("hw1shell: pid %d started\n", pid);
        }
       
    } else {
        // Fork failed
        perror("fork() failed");
        exit(1);
    }
}


