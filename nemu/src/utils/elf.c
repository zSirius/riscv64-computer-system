#include <common.h>

#define SET_FP(offset) fseek(elf_fp, (offset), SEEK_SET);

typedef struct{
    char name[16];
    vaddr_t start;
    size_t size;
}func_table;


static uint16_t e_shstrndx;
static uint64_t shoff;
static uint64_t shstrtab_off;


void get_shoff(FILE *elf_fp){
    
    SET_FP(40);
    size_t byte_read = fread(&shoff, sizeof(shoff), 1, elf_fp);
    if(byte_read!=0)
        printf("shoff = %lu\n", shoff);
}

void get_shstrtab(FILE *elf_fp){
    
    SET_FP(62);
    size_t byte_read = fread(&e_shstrndx, sizeof(e_shstrndx), 1, elf_fp);
    if(byte_read!=0)
        printf("e_shstrndx = %hu\n", e_shstrndx);
    
    get_shoff(elf_fp);


    SET_FP(shoff+64*e_shstrndx+24);

    byte_read = fread(&shstrtab_off, sizeof(shstrtab_off), 1, elf_fp);

    if(byte_read!=0)
        printf("shstrtab_off = %lx\n", shstrtab_off);

    // SET_FP(shstrtab_off)
    // unsigned char ch[107];
    // byte_read = fread(ch, sizeof(unsigned char), 104 , elf_fp);
    // if(byte_read != 0)
    //     for(int i=0; i<104; i++)
    //         printf("%c", ch[i]);

    SET_FP(shoff+64*8)
    uint64_t a[8];
    byte_read = fread(a, sizeof(uint64_t), 8 , elf_fp);
    if(byte_read != 0)
        for(int i=0; i<8; i++)
            printf("%lx \n", a[i]);
    return;

}


void init_elf(const char *elf_file){
    if(elf_file == NULL) return;
    printf("elf_file = %s\n",elf_file);
    
    FILE *elf_fp;
    FILE *fp = fopen(elf_file, "r");
    Assert(fp, "Can not open '%s'", elf_file);
    elf_fp= fp;

    // uint64_t s[8];
    // size_t byte_read = fread(s, sizeof(uint64_t), 8, elf_fp);
    // if(byte_read != 0)
    //     for(int i=0; i<8; i++)
    //         printf("%lx\n", s[i]);

    get_shstrtab(elf_fp);




}