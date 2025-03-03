#define SDL_malloc  malloc
#define SDL_free    free
#define SDL_realloc realloc

#define SDL_STBIMAGE_IMPLEMENTATION
#include "SDL_stbimage.h"

SDL_Surface* IMG_Load_RW(SDL_RWops *src, int freesrc) {
  assert(src->type == RW_TYPE_MEM);
  assert(freesrc == 0);
  return NULL;
}

SDL_Surface* IMG_Load(const char *filename) {
  printf("in image: filename = %s\n", filename);
  FILE *fp = fopen(filename, "r");
  printf("fp = %p\n", fp);
  if(fp == NULL) return NULL;

  fseek(fp, 0, SEEK_END);
  long fsize = ftell(fp);
  printf("fsize = %l\n", fsize);

  unsigned char *buf = (unsigned char *)malloc(sizeof(unsigned char) * fsize);
  fseek(fp, 0, SEEK_SET);
  fread(buf, 1, fsize, fp);

  SDL_Surface *ret = STBIMG_LoadFromMemory(buf, fsize);
  fclose(fp);
  free(buf);
  printf("this is load end, ret = %p\n", ret);
  return ret;
}

int IMG_isPNG(SDL_RWops *src) {
  return 0;
}

SDL_Surface* IMG_LoadJPG_RW(SDL_RWops *src) {
  return IMG_Load_RW(src, 0);
}

char *IMG_GetError() {
  return "Navy does not support IMG_GetError()";
}
