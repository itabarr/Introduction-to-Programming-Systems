#include "types.h"
#include "stat.h"
#include "user.h"

enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

char* states[] = {
  "UNUSED  ",
  "EMBRYO  ",
  "SLEEPING",
  "RUNNABLE",
  "RUNNING ",
  "ZOMBIE  "
};

int main(int argc, char *argv[]) {
  struct processInfo pi;
  int maxPid = getMaxPid();
  int numProc = getNumProc();

  printf(1, "Total number of active processes: %d\n", numProc);
  printf(1, "Maximum PID: %d\n", maxPid);
  printf(1, "PID\tSTATE\t\tPPID\tSZ\tNFD\tNRSWITCH\n");

  for(int pid = 0; pid <= maxPid; pid++){
    if(getProcInfo(pid, &pi) == 0){
      printf(1, "%d\t%s\t%d\t%d\t%d\t%d\n", pid, states[pi.state], pi.ppid, pi.sz, pi.nfd, pi.nrswitch);
    }
  }

  exit();
}
