#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    pid_t pid;

    printf("Parent process:     ID: %d\n", getpid());

    pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        int i;
        scanf("%d", &i);
        printf("Child process:      ID: %d\n", getpid());
        printf("Child process:      running...\n");

        for (i = 0; i < argc; i++) {
            printf("Child process:      Argument %d: %s\n", i, argv[i]);
            sleep(1);
        }

        printf("Child process:      done.\n");
        exit(EXIT_SUCCESS);
    } else {
        printf("Parent process:     waiting for child process to complete...\n");
        wait(NULL);
        printf("Parent process:     Child process has completed.\n");
        exit(EXIT_SUCCESS);
    }

    return 0;
}

