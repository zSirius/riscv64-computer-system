#include <NDL.h>
#include <SDL.h>
#include <string.h>

#define keyname(k) #k,

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

int SDL_PushEvent(SDL_Event *ev) {
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  return 0;
}

int SDL_WaitEvent(SDL_Event *event) {
  char buf[64];
  if(NDL_PollEvent(buf, sizeof(buf))){
    printf("%s\n",buf);
    if(strncmp(buf, "kd", 2) == 0){
      event->type = SDL_KEYDOWN;
    }else if(strncmp(buf, "ku", 2) == 0){
      event->type = SDL_KEYUP;
    }
    printf("keytype=%d, str=%d, buf+3=%s\n",event->type,strlen(buf)-3-1,buf+3);

    for(int i=0; ; i++){
      if(strncmp(buf+3, keyname[i], strlen(buf)-3-1) == 0){
        event->key.keysym.sym = i;
        printf("keycode=%d\n", i);
        break;
      }
    }
  }
  return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  return NULL;
}
