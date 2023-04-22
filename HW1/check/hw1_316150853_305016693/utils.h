#ifndef UTILS_H_
#define UTILS_H_

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include <errno.h>

#include "consts.h"

extern pid_t curr_background_cmds[MAX_BACKGROUND_CMDS];
extern char bg_cmds_strings[MAX_BACKGROUND_CMDS][COMMAND_LINE_LENGTH];

void insert_bg_cmd(pid_t pid, char bg_command[COMMAND_LINE_LENGTH]) {
    int i = 0;
    while(1) {
        if (curr_background_cmds[i] == NULL_PID) {
            curr_background_cmds[i] = pid;
            strcpy(bg_cmds_strings[i], bg_command);
            break; 
        }
        i++;
    }
}

int get_background_cmds_num() {
    int i = 0;
    int count = 0;
    while (i < MAX_BACKGROUND_CMDS) {
        if (curr_background_cmds[i] != NULL_PID)
            count++;
        i++;
    }
    return count;
}

void check_single_job(int i) { // i is position in array
    int status;
    if (curr_background_cmds[i] == NULL_PID)
        return;
    pid_t return_pid = waitpid(curr_background_cmds[i], &status, WNOHANG);
    if (return_pid == -1) {
        return;
    } else if (return_pid == 0) { // child is still running
        printf("%d\t%s\n", curr_background_cmds[i], bg_cmds_strings[i]);
    } else if (return_pid == curr_background_cmds[i]) { // child finished
        return;
    }
}

void print_jobs() {
    for (int i = 0; i < MAX_BACKGROUND_CMDS; i++)
        check_single_job(i);
}

int is_cmd_background(char command[COMMAND_LINE_LENGTH]) {
    int is_cmd_background = FALSE;
    if (command[strlen(command)-1] == '&') {
        return TRUE;
    }
    else
        return FALSE;
}

void free_argument_list(int args_count, char* argument_list[MAX_ARGUMENTS_NUM]) {
    for (int i = 0; i <= args_count; i++) {
        free(argument_list[i]);
    }
}

void check_single_bg_cmd(int i) { // i is position in array
    int status;
    if (curr_background_cmds[i] == NULL_PID)
        return;
    pid_t return_pid = waitpid(curr_background_cmds[i], &status, WNOHANG);
    if (return_pid == -1) {
            printf("hw1shell: pid %d finished\n", curr_background_cmds[i]);
        curr_background_cmds[i] = NULL_PID;
        strcpy(bg_cmds_strings[i], "\0");
    } else if (return_pid == 0) { // child is still running
        return;
    } else if (return_pid == curr_background_cmds[i]) { // child finished
        printf("hw1shell: pid %d finished\n", curr_background_cmds[i]);
        curr_background_cmds[i] = NULL_PID;
        strcpy(bg_cmds_strings[i], "\0");
    }
}

void execute_external_command(char command[COMMAND_LINE_LENGTH]) {
        int i = 0;
        int status;
        int is_bg_cmd = is_cmd_background(command);
        char command_original[COMMAND_LINE_LENGTH];
        strcpy(command_original, command);

        if (is_bg_cmd == TRUE)
            command[strlen(command)-1] = '\0';

        char* token = strtok(command," \n");
        char external_command[COMMAND_LINE_LENGTH]; // basic command (no args) like ls, cd, etc... rest will be arguments list
        strcpy(external_command, token);

        if ((is_bg_cmd == TRUE) && (get_background_cmds_num() == MAX_BACKGROUND_CMDS)) {
            printf("hw1shell: too many background commands running\n");
            return;
        }

        char* argument_list[MAX_ARGUMENTS_NUM]; //free later
            while(token != NULL) {
                argument_list[i] = malloc(COMMAND_LINE_LENGTH * sizeof(char));
                strcpy(argument_list[i], token);
                token = strtok(NULL," \n");
                i++;
            }   
        argument_list[i] = NULL;

        pid_t pid = fork(); // fork duplicates process, and execvp replaces the duplicated process with new program

        if (pid == -1) { // unsuccessful forking
            printf("hw1shell: %s failed, errno is %d\n", "fork", errno);
            return;
        }
        else if (pid == 0) { // handle child process, could be before or after parent
            if (execvp(external_command, argument_list) < 0) {
                printf("hw1shell: invalid command\n"); 
                printf("hw1shell: %s failed, errno is %d\n", "execvp", errno);
                free_argument_list(i, argument_list);
                exit(0); // kill process to make space for the next fork @ next command
            }
            free_argument_list(i, argument_list);
            return;
        }
        else { // handle parent process, could be before or after child
            if (is_bg_cmd == FALSE) { // foreground command
                pid_t waitpid_success = waitpid(pid, &status, 0); // wait until foreground command finishes. if system call failed? add error
                if (waitpid_success == -1)
                    printf("hw1shell: %s failed, errno is %d\n", "waitpid", errno);
                return;
            }
            else { // it is indeed a background command
                printf("hw1shell: pid %d started\n", pid);
                insert_bg_cmd(pid, command_original);
            }
            return;
        }
}

void cd(char command[COMMAND_LINE_LENGTH]) {
    char* directory = strtok(command," \n");
    directory = strtok(NULL, " \n");
    if (directory == NULL) {
        if (chdir("") == -1) {
            printf("hw1shell: %s failed, errno is %d\n", "chdir", errno);
            printf("hw1shell: invalid command\n"); 
        }
    }
    else if ((directory != NULL) && (strtok(NULL, " \n") == NULL)) {
        if (chdir(directory) == -1)
            printf("hw1shell: %s failed, errno is %d\n", "chdir", errno);
    }
    else
        printf("hw1shell: invalid command\n");    
}

void exit_cmd() {
    int status;
    pid_t waitpid_success;
    for (int i = 0; i < MAX_BACKGROUND_CMDS; i++) {
        if (curr_background_cmds[i] != NULL_PID) {
            waitpid_success = waitpid(curr_background_cmds[i], &status, 0);
            printf("hw1shell: pid %d finished\n", curr_background_cmds[i]);
        }
    }
    exit(0);
}

int parse_operation(char command[COMMAND_LINE_LENGTH]) { // return 1 to exit
    char command_original[COMMAND_LINE_LENGTH];
    strcpy(command_original, command);
    char* token = strtok(command," \n");
    if (strcmp(token, "cd") == 0) {
        cd(command_original);
        return 0;
    }
    else if (strcmp(token,"exit") == 0) {
        exit_cmd();
        return 1;
    }
    else if (strcmp(token,"jobs") == 0) {
        print_jobs();
        return 0;
    }
    else {
        execute_external_command(command_original);
        return 0;
    }
}

void initialize_bg_cmds() {
    for (int i = 0; i < MAX_BACKGROUND_CMDS; i++) {
        curr_background_cmds[i] = NULL_PID;
        strcpy(bg_cmds_strings[i], "\0");
    }
}

void clean_command(char* command, int cmd_length) {
    int i = 1;
    while (*(command+cmd_length-i) == '\n' || *(command+cmd_length-i) == ' ' ) {// remove spaces and \n from command
        *(command+cmd_length-i) = '\0';
        i++;
    }
}

void check_running_bg_cmds() {
    for (int i = 0; i < MAX_BACKGROUND_CMDS; i++)
        check_single_bg_cmd(i);
}

#endif