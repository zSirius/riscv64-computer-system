#include <common.h>

void init_elf(const char *elf_file){
    if(elf_file == NULL) return;
    printf("elf_file = %s\n",elf_file);
    
    FILE *elf_fp;
    FILE *fp = fopen(elf_file, "r");
    Assert(fp, "Can not open '%s'", elf_file);
    elf_fp= fp;

    unsigned char byte[3];
    size_t byte_read = fread(&byte, sizeof(unsigned char), 3, elf_fp);
    if(byte_read != 0)
        printf("%x %x %x\n", byte[0], byte[1],byte[2]);

}