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
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <memory/vaddr.h>
#include "sdb.h"


static int is_batch_mode = false;


#ifdef CONFIG_IRINGBUF
struct iringbuf _iringbuf = {.start = 0, .end = -1};

void iringbuf_display(){
  if(_iringbuf.end == -1){
    printf("iringbuf is empty!\n");
    return;
  }
  for(int i=_iringbuf.start; i!=_iringbuf.end; i = (i+1)%10){
    printf("     ");
    puts(_iringbuf.logbuf[i]);
  }
  printf(" --> %s\n", _iringbuf.logbuf[_iringbuf.end]);
  return;
}
#endif

void mtrace_display();

void init_regex();
void init_wp_pool();
word_t expr(char *e, bool *success);
WP *find_wp(int NO);
void free_wp(WP *wp);
WP* new_wp();
void watchpoint_display();

void print_elf_log();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  nemu_state.state = NEMU_END;
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args){
  if(args == NULL){
    fprintf(stderr,"Error: si need one argument!\n");
    return 0;
  }
  if(!(strspn(args, "0123456789") == strlen(args))){
    fprintf(stderr,"Error: argument must be a number!\n");
    return 0;
  }
  int steps = atoi(args);
  cpu_exec(steps);
  return 0;
}

static int cmd_info(char *args){
  if(args == NULL) return 0;
  if(strcmp(args,"r")==0)
    isa_reg_display();
  else if(strcmp(args,"iringbuf")==0){
  #ifdef CONFIG_IRINGBUF
    iringbuf_display();
  #else
    fprintf(stderr, "Error: Please enable iringbuf!\n");
  #endif
  }else if(strcmp(args,"mtrace")==0){
  #ifdef CONFIG_MTRACE
    mtrace_display();
  #else
    fprintf(stderr, "Error: Please enable mtrace!\n");
  #endif
  }else if(strcmp(args, "w")==0){
  #ifdef CONFIG_WATCHPOINT
    watchpoint_display();
  #else
    fprintf(stderr, "Error: Please enable watchpoint!\n");
  #endif
  }else if(strcmp(args, "elflog")==0){
    print_elf_log();
  }
  return 0;
}

static int cmd_x(char *args){
  int N;
  vaddr_t EXPR_vddr;
  char str[128];
  bool success;

  if(args == NULL){
    fprintf(stderr, "Error: x need two arguments!\n");
    return 0;
  }

  sscanf(args, "%d %[^\n]", &N, str);
  EXPR_vddr = expr(str, &success);

  if(!success){
    fprintf(stderr, "Error: Please check your expression!\n");
    return 0;
  }
  //format print data
  for(int i=0; i<N; i++){
    printf("0x%016lx: ",EXPR_vddr+4*i);
    for(int j=3; j>=0; j--)
      printf("%02lx ", vaddr_read(EXPR_vddr+4*i+j, 1));  //small-ended sequence
    printf("\n");
  }
  return 0;
}

static int cmd_p(char *args){
  bool success;
  word_t res = expr(args, &success);
  if(!success){
    fprintf(stderr, "Error: Please check your expression!\n");
  }else{
    printf("%lu\n", res);
  }
  return 0;
}

static int cmd_w(char *args){
#ifndef CONFIG_WATCHPOINT
  fprintf(stderr, "Error: Please enable watchpoint!\n");
  return 0;
#endif
  bool success;
  word_t val = expr(args, &success);
  if(!success){
    fprintf(stderr, "Error: Please check your expression!\n");
    return 0;
  }

  WP *wp = new_wp();
  if(wp != NULL){
    strcpy(wp->expr, args);
    wp->old_value = val;
    wp->hit_cnt=0;
  }
  return 0;
}

static int cmd_d(char *args){
#ifndef CONFIG_WATCHPOINT
  fprintf(stderr, "Error: Please enable watchpoint!\n");
  return 0;
#endif
  bool success;
  int NO = expr(args, &success);
  if(!success){
    fprintf(stderr, "Error: Please check your expression!\n");
    return 0;
  }
  WP *wp = find_wp(NO);
  if(wp != NULL){
    free_wp(wp);
  }
  return 0;
}

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Single step of the execution of the program", cmd_si},
  { "info", "Print infomation of the program", cmd_info},
  { "x", "Scan memory", cmd_x},
  { "p", "Expression evaluation", cmd_p},
  { "w", "Add watchpoint", cmd_w},
  { "d", "Delete watchpoint", cmd_d},

  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
