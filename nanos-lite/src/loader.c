#include <proc.h>
#include <elf.h>
#include <fs.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

size_t ramdisk_read(void *buf, size_t offset, size_t len);

static uintptr_t loader(PCB *pcb, const char *filename) {
  int fd = fs_open(filename, 0, 0);

  Elf64_Ehdr elf_header;
  fs_read(fd, &elf_header, sizeof(elf_header));
  //ramdisk_read(&elf_header, 0, sizeof(elf_header));
  assert(*(uint32_t *)elf_header.e_ident == 0x464c457f);
  assert(elf_header.e_machine == EM_RISCV);
  
  for(int i=0; i<elf_header.e_phnum; i++){
    Elf_Phdr ph_entry;
    fs_lseek(fd, elf_header.e_phoff + i * elf_header.e_phentsize, SEEK_SET);
    fs_read(fd, &ph_entry, sizeof(ph_entry));
    //ramdisk_read(&ph_entry, elf_header.e_phoff + i * elf_header.e_phentsize, sizeof(ph_entry));
    if(ph_entry.p_type == PT_LOAD){
      //printf("this is LOAD\n");
      fs_lseek(fd, ph_entry.p_offset, SEEK_SET);
      fs_read(fd, (void *)ph_entry.p_vaddr, ph_entry.p_memsz);
      //ramdisk_read((void *)ph_entry.p_vaddr, ph_entry.p_offset, ph_entry.p_memsz);
      memset((void *)(ph_entry.p_vaddr + ph_entry.p_filesz), 0, ph_entry.p_memsz - ph_entry.p_filesz);
      //printf("end load\n");
    }
  }

  fs_close(fd);
  return elf_header.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

