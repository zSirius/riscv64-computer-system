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

#include <isa.h>

#define ETRACE_LOG_PWD "/home/sirius/ics2022/nemu/src/isa/riscv64/system/etrace_log.txt"

#ifdef CONFIG_ETRACE
void add_etrace(word_t NO, vaddr_t epc){
  static bool need_create_log = true;
  FILE *log_fp = NULL;
  if(need_create_log){
    log_fp = fopen(ETRACE_LOG_PWD, "w");
    need_create_log = false;
  }else{
    log_fp = fopen(ETRACE_LOG_PWD, "a");
    fseek(log_fp, 0, SEEK_END);
  }
  fprintf(log_fp, "EPC:[0x%8lx], Exception NO:[%ld]\n", epc, (int64_t)NO);
  fclose(log_fp);
}

void etrace_display(){
    char line[256];
    FILE *log_fp = fopen(ETRACE_LOG_PWD, "r");
    while(fgets(line, sizeof(line), log_fp)!=NULL){
        printf("%s", line);
    }
    fclose(log_fp);
}
#endif

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */
  cpu.csr.mepc = epc;
  cpu.csr.mcause = NO;
#ifdef CONFIG_ETRACE
  add_etrace(NO, epc);
#endif
  return cpu.csr.mtvec;
}

word_t isa_query_intr() {
  return INTR_EMPTY;
}
