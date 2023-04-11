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

// TODO : handle redirection in internal commands
// TODO : ask frumkis to do tests
// TODO : check for edge cases

// function declarations
void print_prompt(void);
void read_command(char *cmd , char *parsed_cmd);
void exit_program(int *run, char *cmd, process_manager *ph);
void change_directory(char *cmd , char *args[]);
int parse_command(char *cmd, char *args[], int *is_background, int *is_internal, int *redirect, char *redirect_file);
void print_jobs(char *cmd , process_manager *ph);
int is_valid_dir(char *dir);
int is_internal_command(char *cmd);
int is_background_command(char *last_arg);
void run_external_process(char cmd[], char *args[], int is_background , process_manager *ph , int redirect , char *redirect_file);
// void handle_background(char *cmd); 

int main() {
    int run = 1;
    char cmd[MAX_LINE];
    char parsed_cmd[MAX_LINE];
    char *args[MAX_ARGS];
    char redirect_file[MAX_LINE];

    int is_internal;
    int is_background;
    int redirect;
    int valid_cmd;

    process_manager *ph = create_process_manager();

    while (run) {
        print_prompt(); 

        read_command(cmd, parsed_cmd);
        valid_cmd = parse_command(parsed_cmd, args, &is_background, &is_internal, &redirect, redirect_file);

        if (!valid_cmd) {
            printf("hw1shell: invalid command\n");
            continue;
        }

        if (is_internal) {
            exit_program(&run, args[0] , ph);
            change_directory(args[0], args);
            print_jobs(args[0] , ph);
        }
        else{
            run_external_process(cmd, args, is_background , ph, redirect, redirect_file);
        }
        
    }

    return 0;
}

// prints prompt to console
void print_prompt(void) {
    printf("hw1shell$ "); 
}

//reads command from console
void read_command(char *cmd , char *parsed_cmd) {
    fgets(cmd, MAX_LINE, stdin);
    cmd[strlen(cmd) - 1] = '\0';
    strcpy(parsed_cmd, cmd);
}

// parse command into arguments
int parse_command(char *cmd, char *args[] , int *is_background, int *is_internal , int *redirect, char *redirect_file) {
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
        i--;
    }

    // check if there is a redirect
    handle_redirect(args, redirect, redirect_file);

    if (*redirect == 1 && *redirect_file != NULL) {
        // remove 2 last args
        args[i - 1] = NULL;
        args[i - 2] = NULL;

    }
    if (*redirect == 1 && *redirect_file == NULL) {
        return 0;
    }
    return 1;
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
void exit_program(int *run, char *cmd, process_manager *ph) {
    if (strcmp(cmd, "exit") == 0) {
        *run = 0;
        kill_all_processes(ph);
        free_process_manager(ph);
        exit(0);
    }

    
    
}

// changes the directory if command is cd
void change_directory(char *cmd , char *args[]){
    int result;
    if (strcmp(cmd, "cd") == 0) {

        if (args[1] == NULL) {
            printf("hw1shell: invalid command\n");
        }
        else if (strcmp(args[1], "..") == 0) {
            chdir("..");
        }

        else if (!is_valid_dir(args[1])) {
            printf("hw1shell: invalid command\n");
        }

        else
        {
            result = chdir(args[1]);
            if (result == -1) {
                printf("hw1shell: invalid command\n");

            }
        }
    
    }
}

//function that checks if str is a valid directory format
int is_valid_dir(char *str){
    int i = 0;
    while(str[i] != '\0'){
        if(str[i] == '/'){
            return 1;
        }
        i++;
    }
    return 0;
}

//run external process with fork and execvp
void run_external_process(char cmd[], char *args[], int is_background, process_manager *ph , int redirect , char *redirect_file) {
    pid_t pid;
    int status;
    FILE *fp;
    int fd; 
    if (is_background && is_full(ph)){
        printf("hw1shell: too many background commands running\n");
        return;

    }

    if (redirect){
        fp = fopen(redirect_file, "w");
        fd = fileno(fp);
    }

    
    pid = fork();

    if (pid == 0) {
        if (redirect){
            dup2(fd, STDOUT_FILENO);
        }
        
        execvp(args[0], args);
        perror("hw1shell: invalid command");
        exit(1);

    } else if (pid > 0) {
        // Parent process
        if (!is_background) {
            // print pid of child process
            waitpid(pid, &status, 0); 

            if (redirect){
                fclose(fp);
            }
        }
        else{
            printf("hw1shell: pid %d started\n", pid);
            add_process(ph, pid , cmd , fp);
        }
       
    } else {
        // Fork failed
        perror("fork() failed");
        exit(1);
    }

    clean_finished_processes(ph);

}

// prints the jobs if command is jobs
void print_jobs(char *cmd, process_manager *ph){
    if(strcmp(cmd,"jobs") == 0){
        print_process_manager(ph);
    }
    return;
}

// handle output redirection
void handle_redirect(char *args[], int *redirect , char *redirect_file){
    // check if one of the args is >
    // if it is, then the next arg is the file name
    for (int i = 0; i < MAX_ARGS; i++){
        if (args[i] == NULL){
            *redirect = 0;
            redirect_file = NULL;
            return;
        }

        if (strcmp(args[i], ">") == 0){
            *redirect = 1;

            if (args[i+1] != NULL){
                strcpy(redirect_file, args[i+1]);
            }

            else{
                redirect_file = NULL;
                return;
            }
            return ;
        }
    }

}
