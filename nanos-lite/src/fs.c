#include <fs.h>

#define FILE_NUM sizeof(file_table)/sizeof(file_table[0])

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);
typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

size_t serial_write(const void *buf, size_t offset, size_t len);

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
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write, 0},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write, 0},
#include "files.h"
  // {"/bin/file-test", 51888, 0, NULL, NULL, 0},
  // {"/bin/hello", 36792, 51888, NULL, NULL, 0},
  // {"/bin/dummy", 33048, 88680, NULL, NULL, 0},
  // {"/bin/dummy.txt", 248665, 121728, NULL, NULL, 0},
  // {"/share/music/little-star.ogg", 140946, 370393, NULL, NULL, 0},
  // {"/share/music/rhythm/Do.ogg", 6473, 511339, NULL, NULL, 0},
  // {"/share/music/rhythm/empty.ogg", 4071, 517812, NULL, NULL, 0},
  // {"/share/music/rhythm/Re.ogg", 6503, 521883, NULL, NULL, 0},
  // {"/share/music/rhythm/Mi.ogg", 6611, 528386, NULL, NULL, 0},
  // {"/share/music/rhythm/Si.ogg", 6647, 534997, NULL, NULL, 0},
  // {"/share/music/rhythm/Fa.ogg", 6625, 541644, NULL, NULL, 0},
  // {"/share/music/rhythm/La.ogg", 6542, 548269, NULL, NULL, 0},
  // {"/share/music/rhythm/So.ogg", 6538, 554811, NULL, NULL, 0},
  // {"/share/pictures/projectn.bmp", 49290, 561349, NULL, NULL, 0},
  // {"/share/files/num", 5000, 610639, NULL, NULL, 0},
  // {"/share/fonts/Courier-7.bdf", 19567, 615639, NULL, NULL, 0},
  // {"/share/fonts/Courier-11.bdf", 23272, 635206, NULL, NULL, 0},
  // {"/share/fonts/Courier-12.bdf", 24339, 658478, NULL, NULL, 0},
  // {"/share/fonts/Courier-9.bdf", 20488, 682817, NULL, NULL, 0},
  // {"/share/fonts/Courier-8.bdf", 20114, 703305, NULL, NULL, 0},
  // {"/share/fonts/Courier-13.bdf", 25677, 723419, NULL, NULL, 0},
  // {"/share/fonts/Courier-10.bdf", 21440, 749096, NULL, NULL, 0},
};

void init_fs() {
  // TODO: initialize the size of /dev/fb
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
  if(file->open_offset >= file->size) return 0;
  size_t real_len = len > file->size - file->open_offset ? file->size - file->open_offset : len;
  ramdisk_read(buf, file->disk_offset + file->open_offset, real_len);
  file->open_offset += real_len;
  return real_len;
}

size_t fs_write(int fd, const void *buf, size_t len){
  Finfo *file = &file_table[fd];
  if(file->write != NULL){
    return file->write(buf, 0, len);
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
