#include <common.h>

#define SET_FP(offset) fseek(elf_fp, (offset), SEEK_SET)
#define ELF64_ST_TYPE(info)    ((info) & 0x0F)

struct log
{
    char log[4096][2045];
    int num;
}elf_log = { .num=0 };
static int layer = 0;

void add_elf_log(char *type, char *name, uint64_t pc, uint64_t addr){
    if(strcmp("call", type)==0)
        sprintf(elf_log.log[elf_log.num++], "0x%8lx:%*s %s [%s @ 0x%8lx]", pc, layer, "", type, name, addr);
    else
        sprintf(elf_log.log[elf_log.num++], "0x%8lx:%*s %s [%s]", pc, layer, "", type, name);
}

void print_elf_log(){
    for(int i=0; i<elf_log.num; i++){
        printf("%s\n", elf_log.log[i]);
    }
}


typedef struct{
    char name[64];
    uint64_t start;
    uint64_t size;
}FunctionTable;

FunctionTable ftab[64];
int ftab_num=0;

void insert_ftab(char name[], uint64_t addr, uint64_t size){
    strcpy(ftab[ftab_num].name, name);
    ftab[ftab_num].start = addr;
    ftab[ftab_num].size = size;
    ftab_num++;
}

void is_func_addr(uint64_t pc,uint64_t addr){
    //printf("pc=0x%8lx, addr=0x%8lx\n",pc,addr);
    for(int i=0; i<ftab_num; i++){
        if(addr == ftab[i].start){
            add_elf_log("call", ftab[i].name, pc, addr);
            layer++;
            return;
        }
    }
}

void is_func_ret(uint64_t pc){
    for(int i=0; i<ftab_num; i++){
        if(pc >= ftab[i].start && pc <= ftab[i].start+ftab[i].size-1){
            add_elf_log("ret", ftab[i].name, pc, 0);
            layer--;
            return;
        }
    }
}

typedef struct
{
    char str[64];
    int idx;
}StringTable;

static uint16_t e_shstrndx;
static uint64_t shoff;

static uint64_t shstrtab_off;
static uint64_t shstrtab_size;
static StringTable shstrtab[128];
static int shstrtab_num=0;

static uint64_t symtab_off;
static uint64_t symtab_size;

static uint64_t strtab_off;
static uint64_t strtab_size;
static StringTable strtab[128];
static int strtab_num=0;


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
        }else if(idx > strtab[i].idx && idx < strtab[i].idx + strlen(strtab[i].str) + 1){
            strcpy(name,strtab[i].str+(idx - strtab[i].idx));
            return;
        }
    }
}

void get_ftab(FILE *elf_fp){
    
    //process secrion header, get e_shstrndx and shoff
    SET_FP(62);
    size_t byte_read = fread(&e_shstrndx, sizeof(e_shstrndx), 1, elf_fp);
    if(byte_read!=0)
        printf("e_shstrndx = %hu\n", e_shstrndx);
    
    SET_FP(40);
    byte_read = fread(&shoff, sizeof(shoff), 1, elf_fp);

    //process section header, get shstrtab offset and size
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

    //construct shstrtab
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
    
    //get .symtab and .strtab addr/size
    symtab_off = get_section_addr_by_name(".symtab", elf_fp, &symtab_size);
    strtab_off = get_section_addr_by_name(".strtab", elf_fp, &strtab_size);

    printf("symtab:off=%lx,size=%lx ; strtab:off=%lx,size=%lx\n", symtab_off, symtab_size, strtab_off, strtab_size);

    //construct strtab
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
    
    // //travel symtab to construct ftab
    // for(int i=1; i<symtab_size/24; i++){
    //     SET_FP(symtab_off+24*i+4);
    //     unsigned char info=0;
    //     byte_read = fread(&info, sizeof(info), 1, elf_fp);
    //     if(byte_read!=0 && ELF64_ST_TYPE(info) == 2){ //func
    //         uint64_t value;
    //         uint64_t size;
    //         uint32_t name_idx;
    //         char name[64];
    //         SET_FP(symtab_off+24*i+8);
    //         byte_read = fread(&value, sizeof(value), 1, elf_fp);
    //         byte_read = fread(&size, sizeof(size), 1, elf_fp);
    //         SET_FP(symtab_off+24*i);
    //         byte_read = fread(&name_idx, sizeof(name_idx), 1, elf_fp);
    //         get_symbol_name_by_idx(name_idx ,name);
    //         insert_ftab(name, value, size);
    //         //printf("value=%lx, size = %lu, name_idx=%d , name=%s\n", value, size, name_idx,name);
    //     }
    // }

    // for(int i=0; i<ftab_num; i++)
    //     printf("%s, 0x%lx, %lu\n", ftab[i].name, ftab[i].start, ftab[i].size);

    return;
}


void init_elf(const char *elf_file){
    if(elf_file == NULL) return;
    
    FILE *elf_fp;
    FILE *fp = fopen(elf_file, "r");
    Assert(fp, "Can not open '%s'", elf_file);
    elf_fp= fp;

    get_ftab(elf_fp);
    fclose(elf_fp);
}