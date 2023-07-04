#include <fs.h>

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);
typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  size_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write, 0},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, invalid_write, 0},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, invalid_write, 0},
#include "files.h"
};

void init_fs() {
  // TODO: initialize the size of /dev/fb
}

int fs_open(const char *pathname, int flags, int mode){
  int idx;
  for(idx=3; idx<=22; idx++){
    if(strcmp(pathname, file_table[idx].name) == 0){
      return idx;
    }
  }
  panic("File pathname is error\n");
}

int fs_close(int fd){
  return 0; //always close successfully.
}

size_t fs_read(int fd, void *buf, size_t len){
  Finfo *file = &file_table[fd];
  if(file->open_offset >= file->size) return 0;
  size_t real_len = len > file->size - file->open_offset ? file->size - file->open_offset : len;
  ramdisk_read(buf, file->disk_offset + file->open_offset, real_len);
  file->open_offset += real_len;
  return real_len;
}

size_t fs_write(int fd, const void *buf, size_t len){
  //stdout,stderr.
  if(fd==1 || fd==2){
    size_t cnt;
    for(cnt=0; cnt<len; cnt++)
      putch(*((char *)buf + cnt));
    return cnt;
  }

  Finfo *file = &file_table[fd];
  if(file->open_offset >= file->size) return 0;
  size_t real_len = len > file->size - file->open_offset ? file->size - file->open_offset : len;
  ramdisk_write(buf, file->disk_offset + file->open_offset, real_len);
  file->open_offset += real_len;
  return real_len;
}

size_t fs_lseek(int fd, size_t offset, int whence){
  Finfo *file = &file_table[fd];
  size_t cur_offset;
  switch (whence)
  {
  case SEEK_SET:
    cur_offset = offset;
    break;
  case SEEK_CUR:
    cur_offset = file->open_offset + offset;
    break;
  case SEEK_END:
    cur_offset = file->size + offset;
  default:
    panic("Error argument: whence\n");
  }
  if(cur_offset > file->size || cur_offset < 0) return -1;
  return cur_offset;
}
