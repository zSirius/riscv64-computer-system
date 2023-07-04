#include <stdio.h>
#include <assert.h>

int _write(int fd, void *buf, size_t count);

int main() {
  FILE *fp = fopen("/share/files/num", "r+");
  assert(fp);

  
  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  assert(size == 5000);

  fseek(fp, 500 * 5, SEEK_SET);
  int i, n;
  char str[16];
  fscanf(fp, "%d", &n);
  // sprintf(str, "%d", n);
  // _write(1, str, 16);
  // for (i = 500; i < 1000; i ++) {
  //   fscanf(fp, "%d", &n);
    //_write(1, &n, 4);
    //assert(n == i + 1);
  //}
  // printf("\n");
  // printf("n=%d\n",n);

  // fseek(fp, 0, SEEK_SET);
  // for (i = 0; i < 500; i ++) {
  //   fprintf(fp, "%4d\n", i + 1 + 1000);
  // }

  // for (i = 500; i < 1000; i ++) {
  //   fscanf(fp, "%d", &n);
  //   assert(n == i + 1);
  // }

  // fseek(fp, 0, SEEK_SET);
  // for (i = 0; i < 500; i ++) {
  //   fscanf(fp, "%d", &n);
  //   assert(n == i + 1 + 1000);
  // }

  // fclose(fp);

  // printf("PASS!!!\n");

  return 0;
}
