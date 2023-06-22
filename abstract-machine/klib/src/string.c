#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t len = 0;
  while(s[len]!='\0')
    ++len;
  return len;
}

char *strcpy(char *dst, const char *src) {
  size_t i;
  for(i=0; src[i] != '\0'; i++)
    dst[i] = src[i];
  dst[i] = src[i];
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  size_t i;
  for(i=0; i<n && src[i] != '\0'; i++)
    dst[i] = src[i];
  for(; i<n; i++)
    dst[i] = '\0';
  return dst;
}

char *strcat(char *dst, const char *src) {
  size_t dest_len = strlen(dst);
  size_t i;
  for(i=0; src[i] != '\0'; i++)
    dst[dest_len + i] = src[i];
  dst[dest_len + i] = '\0';
  return dst; 
}

int strcmp(const char *s1, const char *s2) {
  size_t i;
  for(i=0; s1[i]!='\0' || s2[i]!='\0'; i++){
    if(s1[i] != s2[i]) break;
  }
  if(s1[i] == s2[i]) return 0;
  else if(s1[i] > s2[i]) return 1;
  else return -1;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  size_t i;
  for(i=0; i<n && (s1[i]!='\0'|| s2[i]!='\0'); i++){
    if(s1[i] != s2[i]) break;
  }
  if(i==n || s1[i] == s2[i]) return 0;
  else if(s1[i] > s2[i]) return 1;
  else return -1;
}

void *memset(void *s, int c, size_t n) {
  size_t i;
  for(i=0; i<n; i++)
    *((char*)s+i) = c;
  return s;
}

void *memcpy(void *out, const void *in, size_t n) {
  size_t i;
  for(i=0; i<n; i++)
    *((char *)out+i) = *((char *)in+i);
  return out; 
}

void *memmove(void *dst, const void *src, size_t n) {
  unsigned char* d = (unsigned char *)dst;
  const unsigned char* s = (const unsigned char *)src;
  if( (d > s && d - s > n) || s >= d)
    return memcpy(dst, src, n);
  size_t i;
  for(i=n-1; i>=0; i--){
    *(d+i) = *(s+i);
  }
  return dst;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  size_t i;
  for(i=0; i<n; i++){
    if( (*((unsigned char *)s1+i)) != (*((unsigned char *)s2+i)) )
      break;
  }
  if(i==n) return 0;
  else if((*((unsigned char *)s1+i)) > (*((unsigned char *)s2+i))) return 1;
  else return -1;
}

#endif
