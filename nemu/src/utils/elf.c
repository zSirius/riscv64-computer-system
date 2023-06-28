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
static uint64_t shstrtab_size;

typedef struct
{
    char str[32];
    int idx;
}shstrtab;
static shstrtab _shstrtab[32];
static int shstrtab_num=0;


void get_shoff(FILE *elf_fp){
    
    SET_FP(40);
    size_t byte_read = fread(&shoff, sizeof(shoff), 1, elf_fp);
    if(byte_read!=0)
        printf("shoff = %lu\n", shoff);
}

uint64_t get_section_addr_by_name(char *name, FILE *elf_fp){
    int dst_idx=0;
    uint64_t offset;
    for(int i=0; i<shstrtab_num; i++){
        if(strcmp(name, _shstrtab[i].str) == 0){
            dst_idx = _shstrtab[i].idx;
            break;
        }
    }

    for(int i=1; ;i++){
        uint32_t cur_idx=0;
        SET_FP(shoff+64*i);
        size_t byte_read = fread(&cur_idx, sizeof(uint32_t), 1 , elf_fp);
        if(byte_read == 0) return 0;
        if(cur_idx != dst_idx) continue;
        
        SET_FP(shoff+64*i+24);
        byte_read = fread(&offset, sizeof(uint64_t), 1 , elf_fp);
        if(byte_read!=0) return offset;
    }
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

    SET_FP(shoff+64*e_shstrndx+32);
    
    byte_read = fread(&shstrtab_size, sizeof(shstrtab_size), 1, elf_fp);

    if(byte_read!=0)
        printf("shstrtab_size = %lu\n", shstrtab_size);

    //构造节名字符串表
    SET_FP(shstrtab_off)
    unsigned char ch[1024];
    char str[16];
    int cnt=0;
    byte_read = fread(ch, sizeof(unsigned char), shstrtab_size , elf_fp);
    if(byte_read != 0){
        for(int i=0; i<shstrtab_size; i++){
            str[cnt++] = ch[i];
            if(ch[i] == '\0'){
                strcpy(_shstrtab[shstrtab_num].str,str);
                _shstrtab[shstrtab_num++].idx = i-cnt+1;
                cnt=0;
            }
        }
    }
    // printf("\n");        
    // for(int i=0; i<shstrtab_num; i++)
    //     printf("%s , %d\n", _shstrtab[i].str, _shstrtab[i].idx);

    
    //根据节名获取.symtab 和 .strtab的地址

    uint64_t text_offset = get_section_addr_by_name(".text", elf_fp);
    printf("text_offet = %lx", text_offset);


    // SET_FP(shoff+64*2);
    // uint32_t name;
    // byte_read = fread(&name, sizeof(uint32_t), 1 , elf_fp);
    // if(byte_read != 0)
    //     printf("name = %x", name);
    // uint64_t a[8];
    // byte_read = fread(a, sizeof(uint64_t), 8 , elf_fp);
    // if(byte_read != 0)
    //     for(int i=0; i<8; i++)
    //         printf("%lx \n", a[i]);
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