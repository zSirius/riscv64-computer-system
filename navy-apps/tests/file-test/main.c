#include <stdio.h>
#include <assert.h>

int _write(int fd, void *buf, size_t count);

int main() {
  FILE *fp = fopen("/share/files/num", "r+");
  assert(fp);

  
  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);
  printf("size = %d\n", size);
  assert(size == 5000);

  fseek(fp, 500 * 5, SEEK_SET);
  long size2 = ftell(fp);
  printf("size2 = %d\n", size2);
  assert(size2 == 2500);
 

  int i, n;
  for (i = 500; i < 1000; i ++) {
    fscanf(fp, "%d", &n);
    assert(n == i + 1);
  }

  fseek(fp, 0, SEEK_SET);
  for (i = 0; i < 500; i ++) {
    fprintf(fp, "%4d\n", i + 1 + 1000);
  }

  for (i = 500; i < 1000; i ++) {
    fscanf(fp, "%d", &n);
    assert(n == i + 1);
  }

  fseek(fp, 0, SEEK_SET);
  for (i = 0; i < 500; i ++) {
    fscanf(fp, "%d", &n);
    assert(n == i + 1 + 1000);
  }

  fclose(fp);

  printf("PASS!!!\n");

  return 0;
}
