#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

size_t serial_write(const void *buf, size_t offset, size_t len) {
  size_t cnt;
  for(cnt=0; cnt<len; cnt++)
    putch(*((char *)buf + cnt));
  return cnt;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
  if(ev.keycode == AM_KEY_NONE){
    return 0;
  }
  size_t ret = snprintf((char *)buf, len, "%s %s\n", ev.keydown?"kd":"ku", keyname[ev.keycode]);
  return ret;
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  //lseek is not supposed, so ignore offset
  AM_GPU_CONFIG_T gpuconfig = io_read(AM_GPU_CONFIG);
  size_t ret = snprintf((char *)buf, len, "[WIDTH]:%d [HEIGHT]:%d\n", gpuconfig.width, gpuconfig.height);
  return ret;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  int screen_w = io_read(AM_GPU_CONFIG).width;
  size_t i = 0; //had written bytes
  while(i != len){
    int incr = ((screen_w-offset%screen_w) < (len - i) ? (screen_w-offset%screen_w) : (len - i));
    io_write(AM_GPU_FBDRAW, offset%screen_w, offset/screen_w, (uint32_t *)buf+i, incr, 1, false);
    i += incr;
    offset += incr;
    io_write(AM_GPU_FBDRAW, 0, 0, NULL, 0, 0, true);
  }
  io_write(AM_GPU_FBDRAW, 0, 0, NULL, 0, 0, true);
  return len;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
