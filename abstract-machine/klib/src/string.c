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

//The  strcpy() function copies the string pointed to by src, including the terminating null byte ('\0'), to the buffer pointed to by dest.
char *strcpy(char *dst, const char *src) {
  size_t i;
  for(i=0; src[i] != '\0'; i++)
    dst[i] = src[i];
  dst[i] = src[i];
  return dst;
}

//Warning: If there is no null byte among the first n bytes of src, the string placed in dest will not be null-terminated.
char *strncpy(char *dst, const char *src, size_t n) {
  size_t i;
  for(i=0; i<n && src[i] != '\0'; i++)
    dst[i] = src[i];
  for(; i<n; i++)
    dst[i] = '\0';
  return dst;
}

//The  strcat() function appends the src string to the dest string, overwriting the terminating null byte ('\0') at the end of dest, and then adds a terminating null byte.
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

//The memset() function fills the first n bytes of the memory area pointed to by s with the constant byte c.
void *memset(void *s, int c, size_t n) {
  size_t i;
  for(i=0; i<n; i++)
    *((char*)s+i) = c;
  return s;
}

//  The  memcpy()  function  copies  n  bytes from memory area src to memory area dest.  The memory areas must not
//  overlap.  Use memmove(3) if the memory areas do overlap.
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
  int i;
  for(i=n-1; i>=0; i--){
    *(d+i) = *(s+i);
  }
  return dst;
}

//The  memcmp()  function  compares the first n bytes (each interpreted as unsigned char) of the memory areas s1 and s2.
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
