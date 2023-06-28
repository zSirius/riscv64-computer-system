#include <common.h>

void init_elf(const char *elf_file){
    if(elf_file == NULL) return;
    printf("elf_file = %s\n",elf_file);
    
    FILE *elf_fp;
    FILE *fp = fopen(elf_file, "r");
    Assert(fp, "Can not open '%s'", elf_file);
    elf_fp= fp;

    unsigned char byte;
    size_t byte_read = fread(&byte, sizeof(byte), 1, elf_fp);
    if(byte_read != 0)
        printf("%x", byte);

}