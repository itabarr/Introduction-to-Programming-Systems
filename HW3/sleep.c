#include "types.h"
#include "user.h"

int main(void) {
    int pid = fork();
    if (pid < 0) {
        printf(2, "fork failed\n");
        exit();
    } else if (pid == 0) {
        // This is the child process
        // Sleep for 100 seconds
        sleep(10000); // xv6 sleep takes ticks as arguments, 1 second is TPS (ticks per second) ticks.
        exit();
    } else {
        // This is the parent process, which will immediately exit and free the terminal
        exit();
    }
}
