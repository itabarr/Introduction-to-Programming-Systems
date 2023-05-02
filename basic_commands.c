#include "basic_commands.h"
#define MAX_COMMANDS 1024

void msleep(int x) {
    sleep(x / 1000);
}

void increment(int x) {
    char filename[16];
    sprintf(filename, "counter%d.txt", x);

    FILE *fp = fopen(filename, "r+");
    if (fp == NULL) {
        fp = fopen(filename, "w+");
        fprintf(fp, "0");
        fseek(fp, 0, SEEK_SET);
    }

    int count;
    fscanf(fp, "%d", &count);
    count++;
    fseek(fp, 0, SEEK_SET);
    fprintf(fp, "%d", count);
    fclose(fp);
}

void decrement(int x) {
    char filename[16];
    sprintf(filename, "counter%d.txt", x);

    FILE *fp = fopen(filename, "r+");
    if (fp == NULL) {
        fp = fopen(filename, "w+");
        fprintf(fp, "0");
        fseek(fp, 0, SEEK_SET);
    }

    int count;
    fscanf(fp, "%d", &count);
    count--;
    fseek(fp, 0, SEEK_SET);
    fprintf(fp, "%d", count);
    fclose(fp);
}

// Run a string of commands in order
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

// int main() {
//     // Test case 1: Basic sequence of commands
//     char *commands_str_1 = "increment 1;msleep 2;decrement 1";
//     printf("Test case 1: Basic sequence of commands\n");
//     printf("Before: Counter1.txt = ");
//     FILE *fp1 = fopen("counter1.txt", "r");
//     int count1;
//     if (fp1 != NULL) {
//         fscanf(fp1, "%d", &count1);
//         fclose(fp1);
//     }
//     printf("%d\n", count1);
//     run_job(commands_str_1);
//     printf("After: Counter1.txt = ");
//     fp1 = fopen("counter1.txt", "r");
//     if (fp1 != NULL) {
//         fscanf(fp1, "%d", &count1);
//         fclose(fp1);
//     }
//     printf("%d\n", count1);

//     // Test case 2: Repeat a sequence of commands
//     char *commands_str_2 = "repeat 3;increment 2;msleep 5;decrement 1";
//     printf("Test case 2: Repeat a sequence of commands\n");
//     printf("Before: Counter2.txt = ");
//     FILE *fp2 = fopen("counter2.txt", "r");
//     int count2;
//     if (fp2 != NULL) {
//         fscanf(fp2, "%d", &count2);
//         fclose(fp2);
//     }
//     printf("%d\n", count2);
//     run_job(commands_str_2);
//     printf("After: Counter2.txt = ");
//     fp2 = fopen("counter2.txt", "r");
//     if (fp2 != NULL) {
//         fscanf(fp2, "%d", &count2);
//         fclose(fp2);
//     }
//     printf("%d\n", count2);

//     return 0;
// }