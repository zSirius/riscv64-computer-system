#include <NDL.h>
#include <SDL.h>
#include <string.h>

#define keyname(k) #k,
#define KEY_NUM 82


uint8_t keystate[KEY_NUM];

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

int SDL_PushEvent(SDL_Event *ev) {
  return 0;
}

int SDL_PollEvent(SDL_Event *event) {
  char buf[64];
  if(NDL_PollEvent(buf, sizeof(buf))){ //last char is '/n'
    printf("Get Event:%s",buf);
    if(strncmp(buf, "kd", 2) == 0){
      event->type = SDL_KEYDOWN;
    }else if(strncmp(buf, "ku", 2) == 0){
      event->type = SDL_KEYUP;
    }

    for(int i=0; ; i++){
      if(strncmp(keyname[i], buf+3, strlen(keyname[i])) == 0 && (strlen(keyname[i]) == (strlen(buf)-3-1))){
        keystate[i] = !event->type;
        event->key.keysym.sym = i;
        break;
      }
    }
    return 1;
  }
  return 0;
}

int SDL_WaitEvent(SDL_Event *event) {
  char buf[64];
  while(1){
    if(NDL_PollEvent(buf, sizeof(buf))){ //last char is '/n'
      printf("Get Event:%s",buf);
      if(strncmp(buf, "kd", 2) == 0){
        event->type = SDL_KEYDOWN;
      }else if(strncmp(buf, "ku", 2) == 0){
        event->type = SDL_KEYUP;
      }

      for(int i=0; ; i++){
        if(strncmp(keyname[i], buf+3, strlen(keyname[i])) == 0 && (strlen(keyname[i]) == (strlen(buf)-3-1))){
          keystate[i] = !event->type;
          event->key.keysym.sym = i;
          break;
        }
      }
      return 1;
    }    
  }
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  if(numkeys) *numkeys = KEY_NUM;
  return keystate;
}
