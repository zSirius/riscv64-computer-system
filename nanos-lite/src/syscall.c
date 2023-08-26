#include <common.h>
#include "syscall.h"
#include "fs.h"
#include <proc.h>

#define RET(ret) c->GPRx = ret;

//int write(int fd, const void *buf, size_t count);

//gettime
struct timeval {
	long		tv_sec;		/* seconds */
	long	tv_usec;	/* and microseconds */
};
struct timezone {
	int	tz_minuteswest;	/* minutes west of Greenwich */
	int	tz_dsttime;	/* type of dst correction */
};
int gettimeofday(struct timeval *tv, struct timezone *tz);
int execve(const char *pathname, char *const argv[], char *const envp[]);
void exit(int status);
void naive_uload(PCB *pcb, const char *filename);


void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;

  int ret;

  switch (a[0]) {
    case SYS_exit: exit(a[1]); break;
    case SYS_yield: yield(); RET(0); break;
    case SYS_open: ret = fs_open((char *)a[1], a[2], a[3]); RET(ret);break;
    case SYS_read: ret = fs_read(a[1], (void *)a[2], a[3]) ; RET(ret);break;
    case SYS_write: ret = fs_write(a[1], (void *)a[2], a[3]); RET(ret); break;
    case SYS_close: ret = fs_close(a[1]); RET(ret); break;
    case SYS_lseek: ret = fs_lseek(a[1], a[2], a[3]); RET(ret); break;
    case SYS_brk: RET(0); break; //just keep success now.
    case SYS_execve: ret = execve((char *)a[1], (char **)a[2], (char **)a[3]); RET(ret); break;
    case SYS_gettimeofday: ret = gettimeofday((struct timeval *)a[1], (struct timezone *)a[2]); RET(ret); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

}

void exit(int status){
  if(status != 0) halt(status);
  int ret = execve("/bin/nterm", NULL, NULL);
  halt(ret);
}

int gettimeofday(struct timeval *tv, struct timezone *tz){
  //tz = NULL
  uint64_t us = io_read(AM_TIMER_UPTIME).us;
  tv->tv_sec = us/1000000;
  tv->tv_usec = us;
  return 0;
}

int execve(const char *pathname, char *const argv[], char *const envp[]){
  printf("pathname = %s\n",pathname);
  naive_uload(NULL, pathname);
  return -1;
}
