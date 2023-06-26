#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  va_list ap;
  size_t i=0;
  int d;
  char *s;
  int total_ch = 0;

  va_start(ap,fmt);
  while(*fmt){
    switch (*fmt){
    case '%':
      fmt++;
      total_ch++;
      if(*fmt == 'd'){
        char str[64];
        d = va_arg(ap, int);
        itoa(str, d);
        int len = strlen(str);
        for(int i=0; i<len; i++)
          putch(str[i]);
        i += len;
        total_ch += len;
      }else if(*fmt == 's'){
        s = va_arg(ap, char *);
        int len = strlen(s);
        for(int i=0; i<len; i++)
          putch(s[i]);
        i += len;
        total_ch += len;
      }
      fmt++;
      total_ch++;
      break;
    default:
      putch(*fmt++);
      total_ch++;
      break;
    }
  }
  va_end(ap);
  putch('y');
  putch('e');
  putch('s');
  return total_ch;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
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

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  size_t i=0;
  int d;
  char *s;

  va_start(ap,fmt);
  while(*fmt){
    switch (*fmt){
    case '%':
      fmt++;
      if(*fmt == 'd'){
        char str[64];
        d = va_arg(ap, int);
        itoa(str, d);
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
