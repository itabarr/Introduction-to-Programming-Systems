#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}


// define a new system call of getNumProc that uses getNumProc() in proc.c
int sys_getNumProc(void) {
  return getNumProc();
}

// define a new system call of getMaxPid that uses getMaxPid() in proc.c
int sys_getMaxPid(void) {
  return getMaxPid();
}

// define a new system call of getProcInfo that uses getProcInfo() in proc.c
int sys_getProcInfo(void) {
  int pid;
  struct processInfo* pi;

  // fetch the arguments from the user stack to kernel memory to call getProcInfo() and validate the arguments
  if(argint(0, &pid) < 0 || argptr(1, (void*)&pi, sizeof(*pi)) < 0)
    return -1;

  return getProcInfo(pid, pi);
}