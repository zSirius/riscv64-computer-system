#include <fs.h>

#define FILE_NUM sizeof(file_table)/sizeof(file_table[0])

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);
typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

size_t serial_write(const void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len);
size_t dispinfo_read(void *buf, size_t offset, size_t len);
size_t fb_write(const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  size_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_EVENTS, FD_FB, FD_DISPLAYINFO};

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
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write, 0},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write, 0},
  [FD_EVENTS] = {"/dev/event", 0, 0, events_read, invalid_write, 0},
  [FD_FB] = {"/dev/fb", 0, 0, invalid_read, fb_write, 0},
  [FD_DISPLAYINFO] = {"/proc/dispinfo", 0, 0, dispinfo_read, invalid_write, 0},
#include "files.h"
};

void init_fs() {
  // initialize the size of /dev/fb
  int screen_w = io_read(AM_GPU_CONFIG).width, screen_h = io_read(AM_GPU_CONFIG).height;
  file_table[FD_FB].size = screen_w * screen_h *sizeof(uint32_t);

  // initialize the normal files.
  for(int i=6; i<FILE_NUM; i++){
    file_table[i].read = NULL;
    file_table[i].write = NULL;
    file_table[i].open_offset =0;
  }
}

int fs_open(const char *pathname, int flags, int mode){
  int idx;
  for(idx=3; idx<FILE_NUM; idx++){
    if(strcmp(pathname, file_table[idx].name) == 0){
      return idx;
    }
  }
  return -1;
}

int fs_close(int fd){
  return 0; //always close successfully.
}

size_t fs_read(int fd, void *buf, size_t len){
  if(fd==0 || fd==1 || fd ==2) return 0;
  Finfo *file = &file_table[fd];
  if(file->read != NULL){
    return file->read(buf, file->open_offset, len);
  }
  if(file->open_offset >= file->size) return 0;
  size_t real_len = len > file->size - file->open_offset ? file->size - file->open_offset : len;
  ramdisk_read(buf, file->disk_offset + file->open_offset, real_len);
  file->open_offset += real_len;
  return real_len;
}

size_t fs_write(int fd, const void *buf, size_t len){
  Finfo *file = &file_table[fd];
  if(file->write != NULL){
    return file->write(buf, file->open_offset, len);
  }
  if(file->open_offset >= file->size) return 0;
  size_t real_len = len > file->size - file->open_offset ? file->size - file->open_offset : len;
  ramdisk_write(buf, file->disk_offset + file->open_offset, real_len);
  file->open_offset += real_len;
  return real_len;
}

size_t fs_lseek(int fd, size_t offset, int whence){
  if(fd==0 || fd==1 || fd==2) return 0;
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
    break;
  default:
    panic("Error argument: whence\n");
  }
  if(cur_offset > file->size || cur_offset < 0) return 0;
  file->open_offset = cur_offset;
  return cur_offset;
}
