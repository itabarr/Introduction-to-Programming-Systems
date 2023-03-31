#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        char* args[] = { "/bin/ls", "-l", NULL };
        execvp(args[0], args);
        perror("execvp() failed");
        exit(1);
    } else if (pid > 0) {
        // Parent process
        wait(NULL);  // Wait for child to finish
        printf("Child process finished\n");
    } else {
        // Fork failed
        perror("fork() failed");
        exit(1);
    }

    return 0;
}
