/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <memory/host.h>
#include <memory/paddr.h>
#include <device/mmio.h>
#include <isa.h>

#if   defined(CONFIG_PMEM_MALLOC)
static uint8_t *pmem = NULL;
#else // CONFIG_PMEM_GARRAY
static uint8_t pmem[CONFIG_MSIZE] PG_ALIGN = {};
#endif

uint8_t* guest_to_host(paddr_t paddr) { return pmem + paddr - CONFIG_MBASE; }
paddr_t host_to_guest(uint8_t *haddr) { return haddr - pmem + CONFIG_MBASE; }

static word_t pmem_read(paddr_t addr, int len) {
  word_t ret = host_read(guest_to_host(addr), len);
  return ret;
}

static void pmem_write(paddr_t addr, int len, word_t data) {
  host_write(guest_to_host(addr), len, data);
}

static void out_of_bound(paddr_t addr) {
  panic("address = " FMT_PADDR " is out of bound of pmem [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
      addr, PMEM_LEFT, PMEM_RIGHT, cpu.pc);
}

void init_mem() {
#if   defined(CONFIG_PMEM_MALLOC)
  pmem = malloc(CONFIG_MSIZE);
  assert(pmem);
#endif
#ifdef CONFIG_MEM_RANDOM
  uint32_t *p = (uint32_t *)pmem;
  int i;
  for (i = 0; i < (int) (CONFIG_MSIZE / sizeof(p[0])); i ++) {
    p[i] = rand();
  }
#endif
  Log("physical memory area [" FMT_PADDR ", " FMT_PADDR "]", PMEM_LEFT, PMEM_RIGHT);
}

#ifdef CONFIG_MTRACE
struct mtrace _mtrace = { .read_start=0, .read_end=-1, .write_start=0, .write_end=-1};

void mtrace_display(){
  if(_mtrace.read_end == -1){
    printf("Never read memory!\n");
  }else{
    printf("Read memory history:\n");
    printf("   addr      len(bytes)\n");
    for(int i=_mtrace.read_start; i!=_mtrace.read_end; i = (i+1)%64){
      printf("0x%08x     %d\n", _mtrace.read_mtrace[i].addr, _mtrace.read_mtrace[i].len);
    }
    printf("0x%08x      %d\n", _mtrace.read_mtrace[_mtrace.read_end].addr, _mtrace.read_mtrace[_mtrace.read_end].len);
  }

  if(_mtrace.write_end == -1){
    printf("Never wrote to memory!\n");
  }else{
    printf("Write memory history:\n");
    printf("   addr      len(bytes)\n");
    for(int i=_mtrace.write_start; i!=_mtrace.write_end; i = (i+1)%64){
      printf("0x%08x     %d\n", _mtrace.write_mtrace[i].addr, _mtrace.write_mtrace[i].len);
    }
    printf("0x%08x      %d\n", _mtrace.write_mtrace[_mtrace.write_end].addr, _mtrace.write_mtrace[_mtrace.write_end].len);
  }
}

void add_mrecord(struct mtrace_item records[], int *start, int *end, paddr_t addr, int len){
  if(*end != -1 && (*end+1)%64 == *start)
    ++(*start);
  *end = (*end + 1) % 64;
  records[*end].addr = addr;
  records[*end].len = len;
}
#endif

word_t paddr_read(paddr_t addr, int len) {
#ifdef CONFIG_MTRACE
  add_mrecord(_mtrace.read_mtrace, &_mtrace.read_start, &_mtrace.read_end, addr, len);
#endif
  if (likely(in_pmem(addr))) return pmem_read(addr, len);
  IFDEF(CONFIG_DEVICE, return mmio_read(addr, len));
  out_of_bound(addr);
  return 0;
}

void paddr_write(paddr_t addr, int len, word_t data) {
#ifdef CONFIG_MTRACE
  add_mrecord(_mtrace.write_mtrace, &_mtrace.write_start, &_mtrace.write_end, addr, len);
#endif
  if (likely(in_pmem(addr))) { pmem_write(addr, len, data); return; }
  IFDEF(CONFIG_DEVICE, mmio_write(addr, len, data); return);
  out_of_bound(addr);
}
