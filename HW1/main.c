#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
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
void print_jobs(char *args[] , process_manager *ph);
int is_valid_dir(char *dir);
int is_internal_command(char *cmd);
int is_background_command(char *last_arg);
void run_external_process(char cmd[], char *args[], int is_background , process_manager *ph , int redirect , char *redirect_file);
void run_internal_process(char cmd[], char *args[], int *run, process_manager *ph , int is_background , int redirect , char *redirect_file);
void handle_redirect(char *args[], int *redirect , char *redirect_file);
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
            run_internal_process(cmd, args, &run, ph , is_background , redirect , redirect_file);
        }
        else{
            run_external_process(cmd, args, is_background , ph, redirect, redirect_file);
        }
        
    }

    return 0;
}

// prints prompt to console
void print_prompt() {
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

    if (args[0] == NULL) {
        *is_internal = 1;
        return 1;
    }

    *is_background = is_background_command(args[i-1]);
    *is_internal = is_internal_command(args[0]);

    // remove the & from the args if it is a background command
    if (*is_background) {
        args[i - 1] = NULL;
        i--;
    }

    // check if there is a redirect
    handle_redirect(args, redirect, redirect_file);

    if (*redirect == 1 && *redirect_file != '\0') {
        // remove 2 last args
        args[i - 1] = NULL;
        args[i - 2] = NULL;

    }
    if (*redirect == 1 && *redirect_file == '\0') {
        return 0;
    }
    return 1;
}

// checks if command is a internal command
int is_internal_command(char *cmd) {
    if (cmd == NULL || strcmp(cmd, "exit") == 0 || strcmp(cmd, "cd") == 0 || strcmp(cmd, "jobs") == 0) {
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

        if (!is_valid_dir(args[1])) {
            printf("hw1shell: invalid command\n");
        }

        else
        {
            result = chdir(args[1]);
            if (result == -1) {
                fprintf(stderr, "hw1shell: chdir failed, errno is %d\n", errno);
            }
        }
    
    }
}

//function that checks if str is a valid directory format
int is_valid_dir(char *str){
    int i;
    int len;

    if (str == NULL){
    return 0;
    }
    // if .. than ok
    if (strcmp(str, "..") == 0){
        return 1;
    }
    
    // if str is null or - or ~ or . then not valid (by question definition)
    if (str == NULL || strcmp(str, "-") == 0 || strcmp(str, "~") == 0 || strcmp(str, ".") == 0){
        return 0;
    }

    // if str is combination of . and / then not valid (cmdir will try to open it)
    len = strlen(str);
    for (i = 0; i < len; i++) {
        if (str[i] != '.' && str[i] != '/') {
            return 1;
        }
    }
    return 0;
}

//run external process with fork and execvp
void run_external_process(char cmd[], char *args[], int is_background, process_manager *ph , int redirect , char *redirect_file) {
    pid_t pid;
    int status;
    FILE *fp = NULL;
    int fd; 
    if (is_background && is_full(ph)){
        printf("hw1shell: too many background commands running\n");
        clean_finished_processes(ph);
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
        fprintf(stderr, "hw1shell: execvp failed, errno is %d\n", errno);
        printf("hw1shell: invalid command\n");
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
        fprintf(stderr, "hw1shell: fork failed, errno is %d\n", errno);
        exit(1);
    }

    clean_finished_processes(ph);

}

// run internal process
void run_internal_process(char cmd[], char *args[], int *run,  process_manager *ph, int is_background, int redirect , char *redirect_file) {
    pid_t pid;
    //int status;
    FILE *fp = NULL;
    int fd; 
    int original_stdout = dup(STDOUT_FILENO);

    // empty command, continue
    if (args[0] == NULL) {
        clean_finished_processes(ph);
        return;
    }

    // exit and other commands with args not allowed
    if ((strcmp(args[0], "exit") == 0)   && ( args[1] != NULL || redirect || is_background)) {
        printf("hw1shell: invalid command\n");
        clean_finished_processes(ph);
        return;
    }

    exit_program(run, args[0] , ph);
    
    // redirect output if redirect
    if (redirect){
        fp = fopen(redirect_file, "w");
        fd = fileno(fp);
    }

    // run forward command (and handle redirect)
    if (!is_background){
        if (redirect){
                dup2(fd, STDOUT_FILENO);
            }

        change_directory(args[0], args);
        print_jobs(args , ph);

        if (redirect){
                fclose(fp);
                dup2(original_stdout, STDOUT_FILENO);
            }
        
        clean_finished_processes(ph);
        return;

    }

    // run background command
    else{
        pid = fork();

        if (pid == 0) {
            if (redirect){
                dup2(fd, STDOUT_FILENO);
            }
            change_directory(args[0], args);
            print_jobs(args , ph);
            exit(0);

        } else if (pid > 0) {
            // Parent process
            printf("hw1shell: pid %d started\n", pid);
            add_process(ph, pid , cmd , fp);
        } else {
            // Fork failed
            fprintf(stderr, "hw1shell: fork failed, errno is %d\n",  errno);
            exit(1);
        }
    }
    clean_finished_processes(ph);
    
}

// prints the jobs if command is jobs
void print_jobs(char *args[], process_manager *ph){
    if(strcmp(args[0],"jobs") == 0){

        if (args[1] != NULL){
            printf("hw1shell: invalid command\n");
            return;
        }

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
