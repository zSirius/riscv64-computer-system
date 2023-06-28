#include <common.h>

#define RESET_FP fseek(elf_fp, 0, SEEK_SET);

typedef struct{
    char name[16];
    vaddr_t start;
    size_t size;
}func_table;

void init_elf(const char *elf_file){
    if(elf_file == NULL) return;
    printf("elf_file = %s\n",elf_file);
    
    FILE *elf_fp;
    FILE *fp = fopen(elf_file, "r");
    Assert(fp, "Can not open '%s'", elf_file);
    elf_fp= fp;

    uint64_t s;
    size_t byte_read = fread(&s, sizeof(unsigned char), 8, elf_fp);
    if(byte_read != 0)
        printf("%lx\n", s);




}