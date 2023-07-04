#include <common.h>
#include "syscall.h"
#include "fs.h"

#define RET(ret) c->GPRx = ret;

int write(int fd, const void *buf, size_t count);

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;

  //if((int)a[0] == SYS_read)
    printf("this is do_syscall, call num = %d\n", (int)a[0]);
  int ret;

  switch (a[0]) {
    case SYS_exit: halt(a[1]); break;
    case SYS_yield: yield(); RET(0); break;
    case SYS_open: ret = fs_open((char *)a[1], a[2], a[3]); RET(ret);break;
    case SYS_read: ret = fs_read(a[1], (void *)a[2], a[3]) ; RET(ret);break;
    case SYS_write: ret = fs_write(a[1], (void *)a[2], a[3]); RET(ret); break;
    case SYS_close: ret = fs_close(a[1]); RET(ret); break;
    case SYS_lseek: ret = fs_lseek(a[1], a[2], a[3]); RET(ret); break;
    case SYS_brk: RET(0); break; //just keep success now.
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

}

// int open(const char *pathname, int flags, int mode){

// }

// int write(int fd, const void *buf, size_t count){
//   //printf("this write: fd = %d, buf = %s, count = %d \n",fd, (char *)buf , count);
//   int cnt, ret=-1;
//   if(fd == 1 || fd == 2){
//     for(cnt=0; cnt<count; cnt++)
//       putch(*((char *)buf + cnt));
//     ret = cnt;
//   }
//   return ret;
// }