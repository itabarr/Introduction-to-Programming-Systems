#include "basic_commands.h"
#include "global_counter_mutex_list.h"
#include <pthread.h>

#define MAX_COMMANDS 1024

// msleep function 
void msleep(int x) {
    usleep(x * 1000);
}

// increment function 
void increment(int x) {
    char filename[16];
    sprintf(filename,  "count%02d.txt", x);

    // lock the mutex
    pthread_mutex_lock(&mutex_list[x]);

    FILE *fp = fopen(filename, "r+");
    long long count;
    fscanf(fp, "%lld", &count);
    count++;
    fseek(fp, 0, SEEK_SET);
    fprintf(fp, "%lld", count);
    fclose(fp);

    // unlock the mutex
    pthread_mutex_unlock(&mutex_list[x]);
}

// decrement function
void decrement(int x) {
    char filename[16];
    sprintf(filename, "count%02d.txt", x);

    // lock the mutex
    pthread_mutex_lock(&mutex_list[x]);

    FILE *fp = fopen(filename, "r+");
    long long count;
    fscanf(fp, "%lld", &count);
    count--;
    fseek(fp, 0, SEEK_SET);
    fprintf(fp, "%lld", count);
    fclose(fp);

    // unlock the mutex
    pthread_mutex_unlock(&mutex_list[x]);
}

// Run a string of commands in order (and handle repeats)
void run_job(void *_commands_str) {

    char *commands_str = (char *) _commands_str;
    char *command;
    char *param;
    int x;
    char *commands[MAX_COMMANDS + 1];
    int args[MAX_COMMANDS + 1];
    int arg;

    int commands_num = 0;
    int repeat_count = 0;
    int repeat_index = 0;
    char tmp[1024];
    strcpy(tmp, commands_str);

    
    // Create an array of commands and an array of arguments
    const char s[2] = ";";
    command = strtok(tmp, s);

    while (command != NULL) {
        commands[commands_num++] = command;
        command = strtok(NULL, s);
    }
    commands[commands_num] = NULL;

    for (int i = 0; i < commands_num; i++) {
        command = strtok(commands[i], " ");
        param = strtok(NULL, " ");
        x = atoi(param);
        args[i] = x;
    }

    // First excute all commands that are not "repeat"
    for (int i = 0; i < commands_num; i++) {

        command = commands[i];

        if (*command == ' '){
            command = command + 1;
        }

        //printf("%s\n", command);
        arg = args[i];
        if (strcmp(command, "msleep") == 0) {
            msleep(arg);
        } else if (strcmp(command, "increment") == 0) {
            increment(arg);
        } else if (strcmp(command, "decrement") == 0) {
            decrement(arg);
        } else if (strcmp(command, "repeat") == 0) {
            repeat_count = arg;
            repeat_index = i;
            break;
        }
    }

    // Repeat the sequence of commands a specified number of times
    for (int j = 0; j < repeat_count; j++) {
        for (int i = repeat_index+1; i < commands_num; i++) {
            command = commands[i];
            arg = args[i];
            if (*command == ' '){
                command = command + 1;
            }

            //printf("%s\n", command);
            if (strcmp(command, "msleep") == 0) {
                msleep(arg);
            } else if (strcmp(command, "increment") == 0) {
                increment(arg);
            } else if (strcmp(command, "decrement") == 0) {
                decrement(arg);
            }
        }
    }
}
