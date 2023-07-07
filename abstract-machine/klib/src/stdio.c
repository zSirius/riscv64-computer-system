#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

#define DEFINE_VAR va_list ap; size_t i=0; int d; char *s; 
#define PUT_STR(str) for(int j=0; j<len; j++) putch(str[j]); i += len
#define GET_INT char str[64]; d = va_arg(ap, int); itoa(str, d)
#define GET_POINTER char str[64]; u64 = va_arg(ap, uint64_t); dec2hex(str, u64)

void dec2hex(char hex[], uint64_t dec){
  if(dec==0){
    hex[0] = '0';
    hex[1] = '\0';
    return;
  }
  size_t num_len=0;
  uint64_t t = dec;
  while(t!=0){
    num_len++;
    t/=16;
  }
  int i=num_len-1;

  while(dec){
    int t = dec % 16;
    if(t>=0 && t<=9){
      hex[i]=t+'0';
    }else{
      hex[i]=t%10+'a';
    }
    dec /= 16;
    i--;
  }
  hex[num_len] = '\0';
}

void itoa(char str[], int d){
  size_t num_len = 0;
  int t = d;
  while(t != 0){
    num_len++;
    t /= 10;
  }
  for(int i = num_len-1; i>=0; i--){
    str[i] = d%10 + '0';
    d /= 10;
  }
  str[num_len] = '\0';
}

int printf(const char *fmt, ...) {
  DEFINE_VAR;
  int total_ch = 0;
  int len=0;

  va_start(ap,fmt);
  while(*fmt){
    switch (*fmt){
    case '%':
      fmt++;
      total_ch++;
      if(*fmt == 'd'){
        GET_INT;
        len = strlen(str);
        PUT_STR(str);
      }else if(*fmt == 's'){
        s = va_arg(ap, char *);
        len = strlen(s);
        PUT_STR(s);
      }else if(*fmt == 'p'){
        uint64_t u64;
        GET_POINTER;
        len = strlen(str);
        PUT_STR(str);
      }
      fmt++;
      total_ch += len + 1;
      break;
    default:
      putch(*fmt++);
      total_ch++;
      break;
    }
  }
  va_end(ap);
  return total_ch;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}


int sprintf(char *out, const char *fmt, ...) {
  DEFINE_VAR;

  va_start(ap,fmt);
  while(*fmt){
    switch (*fmt){
    case '%':
      fmt++;
      if(*fmt == 'd'){
        GET_INT;
        strcpy(out+i, str);
        i += strlen(str);
      }else if(*fmt == 's'){
        s = va_arg(ap, char *);
        strcpy(out+i, s);
        i += strlen(s);
      }
      fmt++;
      break;
    default:
      *(out+(i++)) = *fmt++;
      break;
    }
  }
  *(out+i) = '\0';
  va_end(ap);
  return strlen(out);
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  DEFINE_VAR;
  int len;

  va_start(ap,fmt);
  while(*fmt){
    switch (*fmt){
    case '%':
      fmt++;
      if(*fmt == 'd'){
        GET_INT;
        len = strlen(str);
        if(i+len >= n-1){
          strncpy(out+i, str, n-1-i);
        }else{
          strcpy(out+i, str);
        }
        i += strlen(str);
      }else if(*fmt == 's'){
        s = va_arg(ap, char *);
        len = strlen(s);
        if(i+len >= n-1){
          strncpy(out+i, s, n-1-i);
        }else{
          strcpy(out+i, s);
        }
        i += strlen(s);
      }
      fmt++;
      break;
    default:
      if(i<=n-2)
        *(out+(i++)) = *fmt++;
      break;
    }
    if(i>n-2) break;
  }
  *(out+i) = '\0';
  va_end(ap);
  return strlen(out);
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
