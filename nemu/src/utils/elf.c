#include <common.h>

#define RESET_FP fseek(elf_fp, 0, SEEK_SET);
#define SET_FP(offset) fseek(elf_fp, (offset), SEEK_SET);

typedef struct{
    char name[16];
    vaddr_t start;
    size_t size;
}func_table;

void get_shstrtab(FILE *elf_fp){
    uint16_t e_shstrndx;
    SET_FP(62);
    size_t byte_read = fread(&e_shstrndx, sizeof(e_shstrndx), 1, elf_fp);
    if(byte_read!=0)
        printf("e_shstrndx = %hx\n", e_shstrndx);
    
    return;

}

void get_sh(FILE *elf_fp){
    uint64_t shoff;
    SET_FP(40);
    size_t byte_read = fread(&shoff, sizeof(shoff), 1, elf_fp);
    if(byte_read!=0);
    printf("shoff = %lx", shoff);
}

void init_elf(const char *elf_file){
    if(elf_file == NULL) return;
    printf("elf_file = %s\n",elf_file);
    
    FILE *elf_fp;
    FILE *fp = fopen(elf_file, "r");
    Assert(fp, "Can not open '%s'", elf_file);
    elf_fp= fp;

    uint64_t s[8];
    size_t byte_read = fread(&s, sizeof(uint64_t), 8, elf_fp);
    if(byte_read != 0)
        for(int i=0; i<8; i++)
            printf("%lx\n", s[i]);

    get_shstrtab(elf_fp);
    get_sh(elf_fp);



}