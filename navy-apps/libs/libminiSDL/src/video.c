#include <NDL.h>
#include <sdl-video.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

// Function to calculate the distance between two colors
int ColorDistance(SDL_Color color1, SDL_Color color2) {
    int diff_r = color1.r - color2.r;
    int diff_g = color1.g - color2.g;
    int diff_b = color1.b - color2.b;
    return diff_r * diff_r + diff_g * diff_g + diff_b * diff_b;
}

// Function to map source color index to destination color index
uint8_t MapColorIndex(SDL_Color src_color, SDL_Palette *dst_palette) {
    // Search for the closest color in the destination palette
    uint8_t dst_color_index = 0;
    int min_distance = ColorDistance(src_color, dst_palette->colors[0]);
    
    for (int i = 1; i < dst_palette->ncolors; i++) {
        int distance = ColorDistance(src_color, dst_palette->colors[i]);
        if (distance < min_distance) {
            min_distance = distance;
            dst_color_index = i;
        }
        if(min_distance == 0) break;
    }
    
    return dst_color_index;
}

void SDL_BlitSurface(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect) {
  assert(dst && src);
  assert(dst->format->BitsPerPixel == src->format->BitsPerPixel);

  // printf("In SDL_BlitSurface\n");
  // for(int i=0; i<src->format->palette->ncolors; i++){
  //   printf("%d:src=%d,dst=%d\n",i, src->format->palette->colors[i].val, dst->format->palette->colors[i].val);
  // }

  int src_x, src_y, dst_x, dst_y;
  int w, h;
  if(srcrect == NULL){
    src_x = 0, src_y = 0;
    w = src->w, h = src->h;
  }else{
    src_x = srcrect->x, src_y = srcrect->y;
    w = srcrect->w, h = srcrect->h;
  }

  if(dstrect == NULL){
    dst_x = 0, dst_y = 0;
  }else{
    dst_x = dstrect->x, dst_y = dstrect->y;
    //ignore dstrect`s w and h;
  }

  // printf("srcrect = %p, dstrect = %p\n", srcrect, dstrect);
  // printf("src_x = %d, src_y = %d, w = %d, h = %d \n", src_x,src_y, w, h);
  // printf("dst_x = %d, dst_y = %d\n", dst_x, dst_y);

  int bytes_per_pixel_src = src->format->BytesPerPixel;
  int bytes_per_pixel_dst = dst->format->BytesPerPixel;

  // printf("bytes_per_pixel_src = %d ,bytes_per_pixel_dst = %d\n", bytes_per_pixel_src, bytes_per_pixel_dst);
  // printf("src->w = %d, src->h = %d, dst->w = %d, dst->h = %d \n", src->w, src->h, dst->w, dst->h);

  int src_init_off_in_bytes = ((int)src_y*src->w + src_x) * (int)bytes_per_pixel_src;
  int dst_init_off_in_bytes = ((int)dst_y*dst->w + dst_x) * (int)bytes_per_pixel_dst;
  
  // printf("src_init_off_in_bytes = %d, dst_init_off_in_bytes = %d \n", src_init_off_in_bytes, dst_init_off_in_bytes);
  // printf("src->pitch = %d, dst->picth=%d \n", src->pitch, dst->pitch);
  // printf("src->pixels = %p, dst->pixels = %p\n", src->pixels, dst->pixels);

  for(int i=0; i<h; i++){
    // for(int j=0; j<w; j++){
    //   for(int b=0; b<bytes_per_pixel_src; b++) {
    //     //printf("In for, i = %d, j=%d,b=%d\n",i,j,b);
    //     if(bytes_per_pixel_src == 4){
    //       *(dst->pixels + dst_init_off_in_bytes + i * (int)dst->pitch + (j * bytes_per_pixel_dst + b)) = 
    //       *(src->pixels + src_init_off_in_bytes + i * (int)src->pitch + (j * bytes_per_pixel_src + b));
    //     }else if(bytes_per_pixel_src == 1){
    //       *(dst->pixels + dst_init_off_in_bytes + i * (int)dst->pitch + (j * bytes_per_pixel_dst + b)) =
    //       *(src->pixels + src_init_off_in_bytes + i * (int)dst->pitch + (j * bytes_per_pixel_dst + b));
    //     }
    //     //printf("result: dst-- %p ; src--%p \n", dst->pixels + dst_init_off_in_bytes + i * (int)dst->pitch + (j * bytes_per_pixel_dst + b), src->pixels + src_init_off_in_bytes + i * (int)src->pitch + (j * bytes_per_pixel_src + b));
    //   }
    // }
    memcpy(dst->pixels + dst_init_off_in_bytes + i * dst->pitch, 
           src->pixels + src_init_off_in_bytes + i * src->pitch,
           w * bytes_per_pixel_src);
  }
  // printf("return from SDL_BlitSurface\n\n");
}

void SDL_FillRect(SDL_Surface *dst, SDL_Rect *dstrect, uint32_t color) {
  //printf("In SDL_FillRect\n");
  int16_t x, y;
  uint16_t w, h;
  if(dstrect == NULL){
    x = 0, y = 0;
    w = dst->w, h = dst->h;
  }else{
    x = dstrect->x, y = dstrect->y;
    w = dstrect->w, h = dstrect->h;
  }
  //printf("x=%d, y=%d, w=%d, h=%d, color=0x%x\n",x,y,w,h,color);

  int bytes_per_pixels = dst->format->BytesPerPixel;
  int init_offset_in_bytes = (y * (dst->w) + x) * bytes_per_pixels;
  
  //printf("BytesPerPixel = %d, init_off = %d\n", bytes_per_pixels, init_offset_in_bytes);

  uint8_t dst_color_index = 0;
  if(bytes_per_pixels == 1){
    SDL_Color t = {.val = color};
    dst_color_index = MapColorIndex(t, dst->format->palette);
  }

  for(int i=0; i<h; i++){
    for(int j=0; j<w; j++){
      uint32_t t_color = color;
      for(int b=0; b<bytes_per_pixels; b++){
        if(bytes_per_pixels == 4){
          *(dst->pixels + init_offset_in_bytes + i * dst->pitch + (j * bytes_per_pixels + b)) = t_color &(0xff);
          t_color >>= 8;          
        }else if(bytes_per_pixels == 1){
          *(dst->pixels + init_offset_in_bytes + i * dst->pitch + (j * bytes_per_pixels + b)) = dst_color_index;
        }
      }
    }
  }
  //printf("end\n\n");
}

void SDL_UpdateRect(SDL_Surface *s, int x, int y, int w, int h) {
  if(x==0 && y==0 && w==0 && h==0){
    w = s->w;
    h = s->h;
  }
  if(s->format->BitsPerPixel == 8){
    // printf("In Update 8\n");
    uint8_t *init_index_ptr = s->pixels + y * s->w + x;
    uint32_t *pixels = malloc(sizeof(uint32_t) * 300 * 400);
    for(int i=0; i<h; i++){
      for(int j=0; j<w; j++){
        pixels[(y+i) * s->w + (x + j)] = s->format->palette->colors[*(init_index_ptr + i * s->w + j)].val; 
      }
    }
    free(pixels);
    NDL_DrawRect(pixels, x, y, w, h);

  }else if(s->format->BitsPerPixel == 32){
    // printf("In Update 32\n");
    NDL_DrawRect((uint32_t *)s->pixels, x, y, w, h);
  }
}

// APIs below are already implemented.

static inline int maskToShift(uint32_t mask) {
  switch (mask) {
    case 0x000000ff: return 0;
    case 0x0000ff00: return 8;
    case 0x00ff0000: return 16;
    case 0xff000000: return 24;
    case 0x00000000: return 24; // hack
    default: assert(0);
  }
}

SDL_Surface* SDL_CreateRGBSurface(uint32_t flags, int width, int height, int depth,
    uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask) {
  assert(depth == 8 || depth == 32);
  SDL_Surface *s = malloc(sizeof(SDL_Surface));
  assert(s);
  s->flags = flags;
  s->format = malloc(sizeof(SDL_PixelFormat));
  assert(s->format);
  if (depth == 8) {
    s->format->palette = malloc(sizeof(SDL_Palette));
    assert(s->format->palette);
    s->format->palette->colors = malloc(sizeof(SDL_Color) * 256);
    assert(s->format->palette->colors);
    memset(s->format->palette->colors, 0, sizeof(SDL_Color) * 256);
    s->format->palette->ncolors = 256;
  } else {
    s->format->palette = NULL;
    s->format->Rmask = Rmask; s->format->Rshift = maskToShift(Rmask); s->format->Rloss = 0;
    s->format->Gmask = Gmask; s->format->Gshift = maskToShift(Gmask); s->format->Gloss = 0;
    s->format->Bmask = Bmask; s->format->Bshift = maskToShift(Bmask); s->format->Bloss = 0;
    s->format->Amask = Amask; s->format->Ashift = maskToShift(Amask); s->format->Aloss = 0;
  }

  s->format->BitsPerPixel = depth;
  s->format->BytesPerPixel = depth / 8;

  s->w = width;
  s->h = height;
  s->pitch = width * depth / 8;
  assert(s->pitch == width * s->format->BytesPerPixel);

  if (!(flags & SDL_PREALLOC)) {
    s->pixels = malloc(s->pitch * height);
    assert(s->pixels);
  }

  s->locked = 0;

  return s;
}

SDL_Surface* SDL_CreateRGBSurfaceFrom(void *pixels, int width, int height, int depth,
    int pitch, uint32_t Rmask, uint32_t Gmask, uint32_t Bmask, uint32_t Amask) {
  SDL_Surface *s = SDL_CreateRGBSurface(SDL_PREALLOC, width, height, depth,
      Rmask, Gmask, Bmask, Amask);
  assert(pitch == s->pitch);
  s->pixels = pixels;
  return s;
}

void SDL_FreeSurface(SDL_Surface *s) {
  if (s != NULL) {
    if (s->format != NULL) {
      if (s->format->palette != NULL) {
        if (s->format->palette->colors != NULL) free(s->format->palette->colors);
        free(s->format->palette);
      }
      free(s->format);
    }
    if (s->pixels != NULL && !(s->flags & SDL_PREALLOC)) free(s->pixels);
    free(s);
  }
}

SDL_Surface* SDL_SetVideoMode(int width, int height, int bpp, uint32_t flags) {
  if (flags & SDL_HWSURFACE) NDL_OpenCanvas(&width, &height);
  return SDL_CreateRGBSurface(flags, width, height, bpp,
      DEFAULT_RMASK, DEFAULT_GMASK, DEFAULT_BMASK, DEFAULT_AMASK);
}

void SDL_SoftStretch(SDL_Surface *src, SDL_Rect *srcrect, SDL_Surface *dst, SDL_Rect *dstrect) {
  assert(src && dst);
  assert(dst->format->BitsPerPixel == src->format->BitsPerPixel);
  assert(dst->format->BitsPerPixel == 8);

  int x = (srcrect == NULL ? 0 : srcrect->x);
  int y = (srcrect == NULL ? 0 : srcrect->y);
  int w = (srcrect == NULL ? src->w : srcrect->w);
  int h = (srcrect == NULL ? src->h : srcrect->h);

  assert(dstrect);
  if(w == dstrect->w && h == dstrect->h) {
    /* The source rectangle and the destination rectangle
     * are of the same size. If that is the case, there
     * is no need to stretch, just copy. */
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    SDL_BlitSurface(src, &rect, dst, dstrect);
  }
  else {
    assert(0);
  }
}

void SDL_SetPalette(SDL_Surface *s, int flags, SDL_Color *colors, int firstcolor, int ncolors) {
  assert(s);
  assert(s->format);
  assert(s->format->palette);
  assert(firstcolor == 0);

  s->format->palette->ncolors = ncolors;
  memcpy(s->format->palette->colors, colors, sizeof(SDL_Color) * ncolors);

  if(s->flags & SDL_HWSURFACE) {
    assert(ncolors == 256);
    for (int i = 0; i < ncolors; i ++) {
      uint8_t r = colors[i].r;
      uint8_t g = colors[i].g;
      uint8_t b = colors[i].b;
    }
    SDL_UpdateRect(s, 0, 0, 0, 0);
  }
}

static void ConvertPixelsARGB_ABGR(void *dst, void *src, int len) {
  int i;
  uint8_t (*pdst)[4] = dst;
  uint8_t (*psrc)[4] = src;
  union {
    uint8_t val8[4];
    uint32_t val32;
  } tmp;
  int first = len & ~0xf;
  for (i = 0; i < first; i += 16) {
#define macro(i) \
    tmp.val32 = *((uint32_t *)psrc[i]); \
    *((uint32_t *)pdst[i]) = tmp.val32; \
    pdst[i][0] = tmp.val8[2]; \
    pdst[i][2] = tmp.val8[0];

    macro(i + 0); macro(i + 1); macro(i + 2); macro(i + 3);
    macro(i + 4); macro(i + 5); macro(i + 6); macro(i + 7);
    macro(i + 8); macro(i + 9); macro(i +10); macro(i +11);
    macro(i +12); macro(i +13); macro(i +14); macro(i +15);
  }

  for (; i < len; i ++) {
    macro(i);
  }
}

SDL_Surface *SDL_ConvertSurface(SDL_Surface *src, SDL_PixelFormat *fmt, uint32_t flags) {
  assert(src->format->BitsPerPixel == 32);
  assert(src->w * src->format->BytesPerPixel == src->pitch);
  assert(src->format->BitsPerPixel == fmt->BitsPerPixel);

  SDL_Surface* ret = SDL_CreateRGBSurface(flags, src->w, src->h, fmt->BitsPerPixel,
    fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);

  assert(fmt->Gmask == src->format->Gmask);
  assert(fmt->Amask == 0 || src->format->Amask == 0 || (fmt->Amask == src->format->Amask));
  ConvertPixelsARGB_ABGR(ret->pixels, src->pixels, src->w * src->h);

  return ret;
}

uint32_t SDL_MapRGBA(SDL_PixelFormat *fmt, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
  assert(fmt->BytesPerPixel == 4);
  uint32_t p = (r << fmt->Rshift) | (g << fmt->Gshift) | (b << fmt->Bshift);
  if (fmt->Amask) p |= (a << fmt->Ashift);
  return p;
}

int SDL_LockSurface(SDL_Surface *s) {
  if(s->locked == 1) return -1;
  s->locked = 1;
  return 0;
}

void SDL_UnlockSurface(SDL_Surface *s) {
  s->locked = 0;
}
