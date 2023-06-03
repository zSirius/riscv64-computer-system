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

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_NUM,

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal
  {"\\-", '-'},
  {"\\*", '*'},
  {"/", '/'},
  {"[0-9]+", TK_NUM},     // number
  {"\\(", '('},
  {"\\)", ')'},

};

void test();

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }

  test();
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        if(substr_len>32){
            fprintf(stderr,"Error: The number is too large!");
            return false;
        }

        if(rules[i].token_type == TK_NOTYPE) break;   //ignore space
        else {
          tokens[nr_token].type = rules[i].token_type;
        }

        switch (rules[i].token_type) {
          case TK_NUM : 
            strncpy(tokens[nr_token].str, substr_start, substr_len);
          default: 
        }

        ++nr_token;
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}


word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
  //TODO();

  return 0;
}


bool check_parentheses(int p, int q){
  int l=p, r=q;
  if(tokens[l++].type!='(' || tokens[r--].type!=')')
    return false;
  int top = -1;
  static char stack[16];
  while(l<=r){
    if(tokens[l].type=='(') stack[++top] = '(';
     else if(tokens[l].type==')') {
       if(top!=-1 && stack[top] == '(') top--;
       else return false;
     }
    l++;
  }
  if(top == -1) return true;
  else{
    fprintf(stderr, "Error: The parentheses do not match!\n");
    assert(0);
  }
}

bool is_lower(int p,int res){
  //printf("tokens[p]: %c, tokens[res]: %c\n", tokens[p].type, tokens[res].type);
  if( (tokens[p].type=='*'||tokens[p].type=='/') && (tokens[res].type== '+' || tokens[res].type== '-') ) 
    return false;
  return true;
}


int find_primary_operator(int p, int q){
  int res=-1;
  int l=p;
  bool parent_flag = false; 
  while(l<q){
    if(parent_flag || tokens[l].type == TK_NUM){
      l++;
      continue;
    }else if(tokens[l].type == '('){
      parent_flag = true;
    }else if(tokens[l].type == ')'){
      parent_flag = false;
    }else if(res==-1 || is_lower(l, res)){
      res = l;
    }
    l++;
  }
  return res;
}

int eval(int p, int q){
  if( p > q){
    fprintf(stderr, "Error: Bad expression!\n");
    assert(0);
  }
  else if(p == q){
    return atoi(tokens[p].str);
  }
  else if( check_parentheses(p,q) == true){
    return eval(p+1,q-1);
  }
  else{
    int op = find_primary_operator(p,q);
    int val1 = eval(p, op-1);
    int val2 = eval(op+1, q);
    switch (tokens[op].type)
    {
    case '+': return val1 + val2;
    case '-': return val1 - val2;
    case '*': return val1 * val2;
    case '/': return val1 / val2;
    default: assert(0);
    }
  }
}

void test(){
  bool success;
  expr(" 20*((((97*61))))", &success);
  // printf("cnts of tokens:%d\n", nr_token);
  // for(int i=0; i<nr_token; i++){
  //   printf("token type:%d, str:%s\n", tokens[i].type, tokens[i].str);
  // }
  int val = eval(0, nr_token-1);
  printf("result val = %d\n", val);
  // int op = find_primary_operator(0, nr_token-1);
  // printf("primary op index is %d\n", op);
}
//((40-90)*(((40+(18)+41/22))/98))/((5))