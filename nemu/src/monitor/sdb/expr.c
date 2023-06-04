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

int eval(int p, int q, bool *success);

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

  //test();
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[4096] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  nr_token=0;
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
  *success = true;
  int res = eval(0, nr_token-1, success);
  return res;
}


bool check_parentheses(int p, int q, bool *success){
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
    *success = false;
    return true;
  }
}

bool is_lower(int p,int res){
  //printf("------is_lower------\n");
  //printf("p is %d, res is %d\n", p, res);
  //printf("p type is %c, res type is %c\n\n", tokens[p].type, tokens[res].type);
  //printf("tokens[p]: %c, tokens[res]: %c\n", tokens[p].type, tokens[res].type);
  if( (tokens[p].type=='*'||tokens[p].type=='/') && (tokens[res].type== '+' || tokens[res].type== '-') ) 
    return false;
  return true;
}


int find_primary_operator(int p, int q){
  //printf("====find op====\n");
  //printf("p is %d, q is %d\n", p, q);
  int res=-1;
  int l=p;
  int parent_flag = 0; 
  while(l<q){
    //printf("token type is %d,  res=%d, flags = %d\n", tokens[l].type, res, parent_flag);
    if(tokens[l].type == '('){
      parent_flag++;
    }else if(tokens[l].type == ')'){
      parent_flag--;
    }else if(parent_flag || tokens[l].type == TK_NUM){
      //if(tokens[l].type == TK_NUM) printf("this is a num, is %s\n", tokens[l].str);
      l++;
      continue;
    }else if(res==-1 || is_lower(l, res)){
      //printf("lower tokens is %c\n", tokens[l].type);
      res = l;
    }
    l++;
  }
  //printf("====over====\n");
  return res;
}

int eval(int p, int q, bool *success){
  //printf("here is %d, p is %d, q is %d\n", ++cnt, p ,q);
  if((*success)==false) return 0;
  if( p > q){
    fprintf(stderr, "Error: Bad expression!\n");
    *success = false;
    return 0;
  }
  else if(p == q){
    return atoi(tokens[p].str);
  }
  else if( check_parentheses(p,q,success) == true){
    if(success==false) return 0;
    return eval(p+1,q-1,success);
  }
  else{
    int op = find_primary_operator(p,q);
    printf("op idx is %d ,  %c\n", op , tokens[op].type);
    int val1 = eval(p, op-1,success);
    int val2 = eval(op+1, q,success);
    switch (tokens[op].type)
    {
    case '+': return val1 + val2;
    case '-': return val1 - val2;
    case '*': return val1 * val2;
    case '/': 
      if(val2 == 0){
        fprintf(stderr, "Error: div-by-zero!\n");
        *success = false;
        return 0;
      }
      return val1 / val2;
    default: 
      fprintf(stderr, "Error: Unexpected operator!\n");
      *success = false;
      return 0;
    }
  }
}

void test(){
  bool success;
  int val = expr("(63*(8)/((92)-42*89/(((48/((45)+((39)))*31)))/73-(4-87-85+1-77)*((45)+82-55-31)-0)+(((30))+59))", &success);
  // printf("cnts of tokens:%d\n", nr_token);
  // for(int i=0; i<nr_token; i++){
  //   printf("token type:%d, str:%s\n", tokens[i].type, tokens[i].str);
  // }
  printf("success is %d, result val = %d\n", success, val);
  // int op = find_primary_operator(0, nr_token-1);
  // printf("primary op index is %d\n", op);
}
//((40-90)*(((40+(18)+41/22))/98))/((5))
//87-(9/41)/(24)-(47)-81/43-37+10+0*40/6
//((97))-(11/((((0)))-17+21-(78)-(74)-((59))))