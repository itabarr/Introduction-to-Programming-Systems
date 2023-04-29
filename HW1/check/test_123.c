#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

#define MAX_COMMANDS 5
#define MAX_COMMAND_LENGTH 1024
#define MAX_ARGS 64
#define MAX_BACKGROUND_PROCESSES 4

//test_test

typedef struct {
    pid_t pid; // process ID
    char* command; // command string
    int status; // 0 if running, 1 if done
} BackgroundProcess;

int numBackgroundProcesses = 0;
BackgroundProcess backgroundProcesses[MAX_BACKGROUND_PROCESSES];

void handleCd(char* args[]) {
    if (args[1] == NULL) {
        fprintf(stderr, "hw1shell: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            fprintf(stderr, "hw1shell: could not change directory\n");
        }
    }
}

void handleJobs() {
    for (int i = 0; i < numBackgroundProcesses; i++) {
        printf("[%d]\t%s\t%s\n", i + 1, backgroundProcesses[i].status == 0 ? "Running" : "Done", backgroundProcesses[i].command);
    }
}

void freeBackgroundProcess(int index) {
    free(backgroundProcesses[index].command);
    for (int i = index; i < numBackgroundProcesses - 1; i++) {
        backgroundProcesses[i] = backgroundProcesses[i+1];
    }
    numBackgroundProcesses--;
}

void handleBackgroundProcess(pid_t pid, char* command) {
    if (numBackgroundProcesses >= MAX_BACKGROUND_PROCESSES) {
        printf("hw1shell: too many background commands running\n");
        return;
    }

    backgroundProcesses[numBackgroundProcesses].pid = pid;
    backgroundProcesses[numBackgroundProcesses].command = strdup(command);
    backgroundProcesses[numBackgroundProcesses].status = 0;
    numBackgroundProcesses++;
    printf("hw1shell: pid %d started\n", pid);
}

void handleFinishedBackgroundProcesses() {
    for (int i = 0; i < numBackgroundProcesses; i++) {
        int status;
        pid_t result = waitpid(backgroundProcesses[i].pid, &status, WNOHANG);
        if (result == -1) {
            printf("hw1shell: waitpid failed\n");
        } else if (result > 0) {
            if (WIFEXITED(status)) {
                printf("hw1shell: pid %d finished with status %d\n", backgroundProcesses[i].pid, WEXITSTATUS(status));
            } else {
                printf("hw1shell: pid %d finished abnormally\n", backgroundProcesses[i].pid);
            }
            freeBackgroundProcess(i);
            i--;
        }
    }
}

void handleExternalCommand(char* args[], int isBackground) {
    pid_t pid = fork();
    if (pid == -1) {
        printf("hw1shell: fork failed\n");
        return;
    }

    if (pid == 0) { // child process
        char* filename = args[0];
        execvp(filename, args);
        exit(errno);
    } else { // parent process
        if (!isBackground) {
            // wait for the foreground process to finish before proceeding
            int status;
            if (waitpid(pid, &status, 0) == -1) {
                printf("hw1shell: waitpid failed\n");
            }
        } else {
            handleBackgroundProcess(pid, args[0]);
        }
    }
}

void cleanup() {
    // reap all remaining background processes
    for (int i = 0; i < numBackgroundProcesses; i++) {
        int status;
        waitpid(backgroundProcesses[i].pid, &status, 0);
        freeBackgroundProcess(i);
    }
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE_LENGTH 1024
#define MAX_NUM_ARGS 64
#define MAX_BG_PROCS 4

static char *bg_procs_cmds[MAX_BG_PROCS] = { NULL };
static pid_t bg_procs_pids[MAX_BG_PROCS] = { 0 };
static int num_bg_procs = 0;

void print_prompt()
{
    printf("hw1shell$ ");
}

void print_bg_procs()
{
    for (int i = 0; i < num_bg_procs; ++i) {
        printf("[%d]\t%s\n", bg_procs_pids[i], bg_procs_cmds[i]);
    }
}

void remove_bg_proc(pid_t pid)
{
    for (int i = 0; i < num_bg_procs; ++i) {
        if (bg_procs_pids[i] == pid) {
            free(bg_procs_cmds[i]);
            bg_procs_cmds[i] = NULL;
            bg_procs_pids[i] = 0;
            --num_bg_procs;
            for (int j = i; j < num_bg_procs; ++j) {
                bg_procs_cmds[j] = bg_procs_cmds[j+1];
                bg_procs_pids[j] = bg_procs_pids[j+1];
            }
            break;
        }
    }
}

void handle_sigchld(int sig)
{
    pid_t pid;
    int status;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("hw1shell: pid %d finished\n", pid);
        remove_bg_proc(pid);
    }
}

void execute_command(char *args[], int bg)
{
    pid_t pid = fork();
    if (pid == -1) {
        printf("hw1shell: fork failed, errno is %d\n", errno);
        return;
    }
    if (pid == 0) { // child process
        int err = execvp(args[0], args);
        printf("hw1shell: invalid command, errno is %d\n", errno);
        exit(err != 0);
    } else { // parent process
        if (!bg) { // foreground command
            int status;
            if (waitpid(pid, &status, 0) == -1) {
                printf("hw1shell: waitpid failed, errno is %d\n", errno);
            }
        } else { // background command
            if (num_bg_procs < MAX_BG_PROCS) {
                printf("hw1shell: pid %d started\n", pid);
                bg_procs_pids[num_bg_procs] = pid;
                size_t cmd_len = 0;
                for (int i = 0; args[i] != NULL; ++i) {
                    cmd_len += strlen(args[i]) + 1;
                }
                bg_procs_cmds[num_bg_procs] = malloc(cmd_len);
                if (bg_procs_cmds[num_bg_procs] == NULL) {
                    printf("hw1shell: malloc failed, errno is %d\n", errno);
                } else {
                    strcpy(bg_procs_cmds[num_bg_procs], args[0]);
                    for (int i = 1; args[i] != NULL; ++i) {
                        strcat(bg_procs_cmds[num_bg_procs], " ");
                        strcat(bg_procs_cmds[num_bg_procs], args[i]);
                    }
                    ++num_bg_procs;
                }
            } else {
                printf("hw1shell: too many background commands running\n");
            }
        }
    }
}

void execute_foreground_command(char** params){
    pid_t pid = fork();

    if(pid == -1){ // fork failed
        fprintf(stderr, "hw1shell: fork failed, errno is %d\n", errno);
        return;
    }

    if(pid == 0){ // child process
        int result = execvp(params[0], params);
        if(result == -1){ // execvp failed
            fprintf(stderr, "hw1shell: invalid command\n");
            exit(EXIT_FAILURE);
        }
    } else { // parent process
        int status;
        waitpid(pid, &status, 0);
    }
}


void execute_background_command(char** params){
    pid_t pid = fork();

    if(pid == -1){ // fork failed
        fprintf(stderr, "hw1shell: fork failed, errno is %d\n", errno);
        return;
    }

    if(pid == 0){ // child process
        int result = execvp(params[0], params);
        if(result == -1){ // execvp failed
            fprintf(stderr, "hw1shell: invalid command\n");
            exit(EXIT_FAILURE);
        }
    } else { // parent process
        printf("hw1shell: pid %d started\n", pid);
    }
}

void run_cd(char *args[])
{
    if (args[1] == NULL) {
        printf("hw1shell: expected argument to \"cd\"\n");
    } else if (chdir(args[1]) != 0) {
        printf("hw1shell: invalid command\n");
    }
}

void run_jobs()
{
    print_bg_procs();
}

void parse_command(char *line, char *args[], int *bg)
{
    int num_args = 0;
    *bg = 0;
    char *p = strtok(line, " \t\n");
    while (p != NULL) {
        if (*p == '&') {
            *bg = 1;
        } else {
            args[num_args++] = p;
        }
        p = strtok(NULL, " \t\n");
    }
    args[num_args] = NULL;
}
int main() {
    char line[MAX_LINE_LENGTH];
    char* params[MAX_PARAMS];
    pid_t background_processes[MAX_BACKGROUND_PROCESSES] = {0};
    int num_background_processes = 0;

    while (1) {
        printf("hw1shell$ ");
        fflush(stdout);

        // read user input
        if (fgets(line, MAX_LINE_LENGTH, stdin) == NULL) {
            continue;
        }

        // tokenize input into separate parameters
        char* token = strtok(line, " \n");
        int num_params = 0;
        while (token != NULL) {
            params[num_params++] = token;
            token = strtok(NULL, " \n");
        }
        params[num_params] = NULL;

        if (num_params == 0) {
            continue; // empty command
        }

        if (strcmp(params[0], "exit") == 0) {
            // TODO: wait and reap all child processes running in the background 
            // TODO: do any necessary cleanups such as freeing dynamically allocated memory
            break; // exit loop and terminate shell
        } else if (strcmp(params[0], "cd") == 0) {
            // built-in cd command
            if (num_params != 2) {
                printf("hw1shell: invalid command\n");
            } else if (chdir(params[1]) != 0) {
                printf("hw1shell: invalid directory\n");
            }
        } else if (strcmp(params[0], "jobs") == 0) {
            // list current background processes
            for (int i = 0; i < num_background_processes; i++) {
                char cmd_line[MAX_LINE_LENGTH];
                snprintf(cmd_line, MAX_LINE_LENGTH, "/proc/%d/cmdline", background_processes[i]);
                FILE* cmdline_file = fopen(cmd_line, "r");
                fprintf(stdout, "%d\t", background_processes[i]);
                int first_arg = 1;
                while (fgets(cmd_line, MAX_LINE_LENGTH, cmdline_file) != NULL) {
                    if (!first_arg) {
                        fprintf(stdout, " ");
                    }
                    fprintf(stdout, "%s", cmd_line);
                    first_arg = 0;
                }
                fprintf(stdout, "\n");
                fclose(cmdline_file);
            }
        } else {
            // external command - determine if foreground or background
            if (num_params > 1 && strcmp(params[num_params-1], "&") == 0) {
                execute_background_command(params);
            } else {
                execute_foreground_command(params);
            }
        }

        // check if any background commands have finished and update background_processes array
        for (int i = 0; i < num_background_processes; i++) {
            pid_t result = waitpid(background_processes[i], NULL, WNOHANG);
            if (result == -1) {
                printf("hw1shell: waitpid failed, errno is %d\n", errno);
            } else if (result == 0) {
                // child process still running
            } else {
                printf("hw1shell: pid %d finished\n", background_processes[i]);
                background_processes[i] = 0;
            }
        }

        // check if a new background command can be added
        if (num_background_processes >= MAX_BACKGROUND_PROCESSES) {
            printf("hw1shell: too many background commands running\n");
        } else if (num_params > 1 && strcmp(params[num_params-1], "&") == 0) {
            background_processes[num_background_processes++] = pid;
        }
    }

    return 0;
}