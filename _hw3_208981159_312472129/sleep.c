#include "types.h"
#include "user.h"

int main(void) {
    int pid = fork();
    if (pid < 0) {
        printf(2, "fork failed\n");
        
        exit();
    } else if (pid == 0) {
        // cerate another child process
        int pid2 = fork();
        if (pid2 < 0) {
            printf(2, "fork failed\n");
            exit();

        } else if (pid2 == 0) {
            sleep(20);
            printf(1, "Child process 2 pid: %d\n", getpid());
            sleep(10000); // xv6 sleep takes ticks as arguments, 1 second is TPS (ticks per second) ticks.
            exit();
        }
        
        else {
            
            printf(1, "Child process pid 1: %d\n", getpid());
            sleep(10000); // xv6 sleep takes ticks as arguments, 1 second is TPS (ticks per second) ticks.
            exit();
        }

        
    } else {
        sleep(50);
        exit();
    }
}
