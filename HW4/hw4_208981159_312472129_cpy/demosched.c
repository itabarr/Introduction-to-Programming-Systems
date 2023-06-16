#include "types.h"
#include "user.h"

// Do some some_work (count to 1000) and return how many ticks it took.
int some_work()
{   
    
    int i;
    int sum = 0;
    int iter = 500000;

    uint start = uptime();
    uint end;

    // do some work (and show use of getprio()) 
    for (i = 0; i < iter ; i++) {
        sum += getprio() * getprio()  + getprio();  
    }

    end = uptime();

    return end-start;
}

int main(void)
{
    int i;
    int pid;
    int proc_time;

    printf(1, "\nThis is a demo program of scheduler priority in xv6.\n8 processes with different priorities will be forked.\nEach process will do some work and then exit.\nDynamic printing will be used to show when a process is finished.\nThe running time takes arround 3 minutes.\n\n");
    
    // Fork 8 child processes with different priorities and do some work
    for (i = 0; i < 8; i++) {
        pid = fork();

    if (pid < 0) {
        printf(1, "fork() failed\n");
        exit();

    } else if (pid == 0) {

        // set priority of child process        
        setprio(i); 

        // do some work
        proc_time = some_work();
        
        // print message and exit
        printf(1, "PID: %d, PRIO: %d, STATUS: Finished in %d ticks!\n",  getpid(),  getprio(), proc_time);
        
        exit();
    }
    }
    // Wait for all child processes to finish
    for (i = 0; i < 10; i++) {
        wait();
    }

    printf(1, "\nFinished demo.\n");
    exit();
}
