#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/_timeval.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;

int _gettimeofday(struct timeval *tv, struct timezone *tz);
int _write(int fd, void *buf, size_t count);
int _read(int fd, void *buf, size_t count);

uint32_t NDL_GetTicks() {
  struct timeval curTime;
  _gettimeofday(&curTime, NULL);
  uint32_t ret = curTime.tv_usec / 1000; 
  return ret;
}

int NDL_PollEvent(char *buf, int len) {
  int ret = _read(3, buf, len);
  return !!ret;
}

void NDL_OpenCanvas(int *w, int *h) {
  printf("this is start of openCanvas\n");
  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  }

  char buf[64];
  int width, height;
  _read(5 ,buf, sizeof(buf));
  sscanf(buf, "[WIDTH]:%d [HEIGHT]:%d/n", &width, &height);
  if(*w==0 && *h==0){
    *w=width;
    *h=height;
  }
  if(*w > width) *w=width;
  if(*h > height) *h=height;
  printf("w=%d, h=%d\n",*w,*h);
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  char buf[64];
  int width, height;
  _read(5 ,buf, sizeof(buf));
  sscanf(buf, "[WIDTH]:%d\n[HEIGHT]:%d/n", &width, &height);
  //for(int i=0; i<h; i++){
  for(int i=0; i<1; i++){ //debug
    printf("(y+i)*width = %d\n",(y+i)*width);
    _lseek(4, (y+i)*width, 0);
    _write(4, pixels+w*i, w);
  }
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  return 0;
}

void NDL_Quit() {
}
