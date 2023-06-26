#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

#define DEFINE_VAR va_list ap; size_t i=0; int d; char *s
#define PUT_STR(str) for(int j=0; j<len; j++) putch(str[j]); i += len
#define GET_INT char str[64]; d = va_arg(ap, int); itoa(str, d)

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
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
