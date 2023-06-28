#include <common.h>

#define SET_FP(offset) fseek(elf_fp, (offset), SEEK_SET)
#define ELF64_ST_TYPE(info)    ((info) & 0x0F)

typedef struct{
    char name[64];
    vaddr_t start;
    size_t size;
}FunctionTable;

typedef struct
{
    char str[64];
    int idx;
}StringTable;

static uint16_t e_shstrndx;
static uint64_t shoff;

static uint64_t shstrtab_off;
static uint64_t shstrtab_size;
static StringTable shstrtab[64];
static int shstrtab_num=0;

static uint64_t symtab_off;
static uint64_t symtab_size;

static uint64_t strtab_off;
static uint64_t strtab_size;
static StringTable strtab[64];
static int strtab_num=0;



void get_shoff(FILE *elf_fp){
    
    SET_FP(40);
    size_t byte_read = fread(&shoff, sizeof(shoff), 1, elf_fp);
    if(byte_read!=0)
        printf("shoff = %lu\n", shoff);
}

uint64_t get_section_addr_by_name(char *name, FILE *elf_fp, uint64_t *size){
    int dst_idx=0;
    uint64_t offset;
    for(int i=0; i<shstrtab_num; i++){
        if(strcmp(name, shstrtab[i].str) == 0){
            dst_idx = shstrtab[i].idx;
            break;
        }
    }

    for(int i=1; ;i++){
        uint32_t cur_idx=0;
        SET_FP(shoff+64*i);
        size_t byte_read = fread(&cur_idx, sizeof(uint32_t), 1 , elf_fp);
        if(byte_read == 0) return 0;
        if(cur_idx != dst_idx) continue;
        
        SET_FP(shoff+64*i+32);
        byte_read = fread(size, sizeof(uint64_t), 1, elf_fp);
        if(byte_read == 0) return 0;

        SET_FP(shoff+64*i+24);
        byte_read = fread(&offset, sizeof(uint64_t), 1 , elf_fp);
        if(byte_read!=0) return offset;
    }
}

void get_symbol_name_by_idx(int idx, char *name){
    for(int i=1; i<strtab_num; i++){
        if(idx == strtab[i].idx){
            strcpy(name, strtab[i].str);
            return;
        }
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


    unsigned char ch[1024];
    char str[16];
    int cnt=0;

    //构造节名字符串表
    SET_FP(shstrtab_off);
    byte_read = fread(ch, sizeof(unsigned char), shstrtab_size , elf_fp);
    if(byte_read != 0){
        for(int i=0; i<shstrtab_size; i++){
            str[cnt++] = ch[i];
            if(ch[i] == '\0'){
                strcpy(shstrtab[shstrtab_num].str,str);
                shstrtab[shstrtab_num++].idx = i-cnt+1;
                cnt=0;
            }
        }
    }
    // printf("\n");        
    // for(int i=0; i<shstrtab_num; i++)
    //     printf("%s , %d\n", shstrtab[i].str, shstrtab[i].idx);

    
    //根据节名获取.symtab 和 .strtab的地址

    symtab_off = get_section_addr_by_name(".symtab", elf_fp, &symtab_size);
    strtab_off = get_section_addr_by_name(".strtab", elf_fp, &strtab_size);
    printf("sym=%lx, str=%lx, symsize=%lx, strsize=%lx\n", symtab_off, strtab_off, symtab_size, strtab_size);



    //构造字符串表
    cnt=0;
    SET_FP(strtab_off);
    byte_read = fread(ch, sizeof(unsigned char), strtab_size, elf_fp);
    if(byte_read != 0){
        for(int i=0; i<strtab_size; i++){
            str[cnt++] = ch[i];
            if(ch[i]=='\0'){
                strcpy(strtab[strtab_num].str, str);
                strtab[strtab_num++].idx = i-cnt+1;
                cnt=0;
            }
        }
    }
    for(int i=0; i<strtab_num; i++)
        printf("%s, %d\n", strtab[i].str, strtab[i].idx);
    
    //遍历符号表
    SET_FP(symtab_off+24+4);

    for(int i=1; i<symtab_size/24; i++){
        SET_FP(symtab_off+24*i+4);
        unsigned char info=0;
        byte_read = fread(&info, sizeof(info), 1, elf_fp);
        if(byte_read!=0 && ELF64_ST_TYPE(info) == 2){ //func
            uint64_t value;
            uint64_t size;
            uint32_t name_idx;
            char name[64];
            SET_FP(symtab_off+24*i+8);
            byte_read = fread(&value, sizeof(value), 1, elf_fp);
            byte_read = fread(&size, sizeof(size), 1, elf_fp);
            byte_read = fread(&name_idx, sizeof(name_idx), 1, elf_fp);
            get_symbol_name_by_idx(name_idx ,name);
            printf("value=%lx, size = %lu, name_idx=%d , name=%s\n", value, size, name_idx,name);
        }
    }

    
    
    
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