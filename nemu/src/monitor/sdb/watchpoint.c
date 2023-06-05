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

#include "sdb.h"

#define NR_WP 32



static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp(){
  if(free_ == NULL){
    fprintf(stderr, "Error: Can not create more watchpoint!\n");
    return NULL;
  }
  WP * ret = free_;
  free_ = free_->next;
  return ret;
}

void free_wp(WP *wp){
  if(head == wp){
    if(head->next == NULL) head = NULL;
    else head = head->next;
  }else{
    WP *pre = head, *cur = head->next;
    while(cur!=NULL){
      if(cur==wp){
        pre->next = cur->next;
        break;
      }
      pre = cur;
      cur = cur->next;
    }
    if(cur == NULL){
      fprintf(stderr, "Error: No such watchpoint exist!\n");
      return;
    }
  }
  wp->next = free_;
  free_ = wp;
  return;
}

bool check_watchpoint(){
  if(head == NULL) return false;
  WP *tmp = head;
  bool success;
  bool break_flag = false;
  while(tmp != NULL){
    word_t new_value = expr(tmp->expr, &success);
    if(!success){
      fprintf(stderr, "Error: Please cheack your expression \"%s\"\n", tmp->expr);
    }else{
      if(new_value != tmp->old_value){
        printf("watchpoint %d: %s\n", tmp->NO, tmp->expr);
        printf("Old value = %lu\n", tmp->old_value);
        printf("New value = %lu\n", new_value);
        tmp->hit_cnt++;
        break_flag = true;
      }
    }
    tmp = tmp->next;
  }
  return break_flag;
}

WP *find_wp(int NO){
  WP *tmp = head;
  while(tmp!=NULL){
    if(tmp->NO == NO) return tmp;
  }
  fprintf(stderr, "Error: watchpoint %d is not exist!\n", NO);
  return NULL;
}

