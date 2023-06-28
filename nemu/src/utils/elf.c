#include <common.h>

#define RESET_FP fseek(elf_fp, 0, SEEK_SET);
#define SET_FP(offset) fseek(elf_fp, (offset), SEEK_SET);

typedef struct{
    char name[16];
    vaddr_t start;
    size_t size;
}func_table;

#define printdubug(str,l)     if(byte_read!=0) printf("str = %lx\n", str);

static uint16_t e_shstrndx;
static uint64_t shoff;
static uint64_t shstrndx_off;

void get_shoff(FILE *elf_fp){
    
    SET_FP(40);
    size_t byte_read = fread(&shoff, sizeof(shoff), 1, elf_fp);
    if(byte_read!=0)
        printf("shoff = %lx\n", shoff);
}

void get_shstrtab(FILE *elf_fp){
    
    SET_FP(62);
    size_t byte_read = fread(&e_shstrndx, sizeof(e_shstrndx), 1, elf_fp);
    if(byte_read!=0)
        printf("e_shstrndx = %hx\n", e_shstrndx);
    
    get_shoff(elf_fp);


    SET_FP(shoff+e_shstrndx*64);
    byte_read = fread(&shstrndx_off, sizeof(shstrndx_off), 1, elf_fp);

    printdubug(shstrndx_off, l);
    return;

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




}