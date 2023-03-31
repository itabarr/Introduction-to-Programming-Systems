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
void exit_program(int *run, char *cmd);
void change_directory(char *cmd);
void parse_command(char *cmd, char *args[]);
int is_internal_command(char *cmd);
int is_background_command(char *cmd);
void run_process(char *args[], int is_background);

// void handle_background(char *cmd); 

int main(void) {
    int run = 1;
    char cmd[MAX_LINE];
    char *args[MAX_ARGS];

    int is_internal;
    int is_background;

    process_manager *ph = create_process_manager();

    while (run) {
        print_prompt(); 
        read_command(cmd);
        
        is_internal = is_internal_command(args[0]);
        is_background = is_background_command(cmd);

        parse_command(cmd, args);

        run_process(args, is_background);
        exit_program(&run, cmd);
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
void parse_command(char *cmd, char *args[]) {
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
}

// checks if command is a internal command
int is_internal_command(char *cmd) {
    if (strcmp(cmd, "exit") == 0 || strcmp(cmd, "cd") == 0 || strcmp(cmd, "jobs") == 0) {
        return 1;
    }
    return 0;
}

// checks if command is a background command 
int is_background_command(char *cmd) {
    if (cmd[strlen(cmd) - 1] == '&') {
        return 1;
    }
    return 0;
}

// checks if user typed exit command
void exit_program(int *run, char *cmd) {
    if (strcmp(cmd, "exit") == 0) {
        *run = 0;
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

// run process with fork and execvp
void run_process(char *args[], int is_background, process_manager *ph) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        execvp(args[0], args);
        perror("execvp() failed");
        exit(1);
    } else if (pid > 0) {
        // Parent process
        if (!is_background) {
            wait(NULL);
            printf("Child process finished\n"); 
        }
        
        else{ 
            add_process(ph, pid, 1);
            printf("Child process finished\n");
        }
       
    } else {
        // Fork failed
        perror("fork() failed");
        exit(1);
    }
}



