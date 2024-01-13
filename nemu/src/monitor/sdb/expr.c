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

sword_t eval(int p, int q, bool *success);
word_t isa_reg_str2val(const char *s, bool *success);
word_t vaddr_read(vaddr_t addr, int len);

enum {
  TK_NOTYPE = 256, TK_EQ, TK_NE, TK_AND, TK_NUM, TK_HEX, TK_DEREF, TK_REG,

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
  {"\\-", '-'},
  {"\\*", '*'},
  {"/", '/'},
  {"==", TK_EQ},        // equal
  {"\\!=", TK_NE},
  {"&&", TK_AND},
  {"0[xX][0-9a-fA-F]+", TK_HEX},
  {"[0-9]+", TK_NUM},     // number
  {"\\(", '('},
  {"\\)", ')'},
  {"\\$(\\$0|ra|sp|gp|tp|pc|t[0-6]|s[0-9]|s10|s11|a[0-7])", TK_REG},

};

/* operator priority */
static struct priority
{
  int type;
  int level;
} priorities[] = {
  {TK_DEREF, 1},
  {'*', 2},
  {'/', 2},
  {'+', 3},
  {'-', 3},
  {TK_EQ, 4},
  {TK_NE, 4},
  {TK_AND, 5},
};

static int get_priority(int type){
  int n = sizeof(priorities)/sizeof(priorities[0]);
  for(int i=0; i<n; i++){
    if(type == priorities[i].type){
      return priorities[i].level;
    }
  }
  ERROR("Invalid Operator!\n");
  return 0;
}

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

}

typedef struct token {
  int type;
  char str[64];
} Token;

static Token tokens[4096] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

/* Lexical analysis of the expression string e */
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
            ERROR("The number is too large!\n");
            return false;
        }

        if(rules[i].token_type == TK_NOTYPE) break;   //ignore space
        else {
          tokens[nr_token].type = rules[i].token_type;
        }

        switch (rules[i].token_type) {
          case TK_NUM: 
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            tokens[nr_token].str[substr_len] = '\0';
            break;
          case TK_HEX:
            strncpy(tokens[nr_token].str, substr_start+2, substr_len-2);
            tokens[nr_token].str[substr_len-2] = '\0';
            break;
          case TK_REG:
            strncpy(tokens[nr_token].str, substr_start+1, substr_len-1);
            tokens[nr_token].str[substr_len-1] = '\0';
            break;
          default:
            break;
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
/* Check expression`s parentheses validation
 * return value: true is valid, false is invalid.
 */
bool check_parentheses_valid(int p, int q){
  int l=p, r=q;
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
  if(top != -1) return false;
  else return true;
}

/* check whether the expression (index between p and q) 
 * is surrounded by a matched pair of parentheses.
 */
bool check_parentheses(int p, int q){
  if(tokens[p].type!='(' || tokens[q].type!=')')
    return false;
  return check_parentheses_valid(p+1, q-1);
}

word_t expr(char *e, bool *success) {
  if(e == NULL){
    ERROR("The Expression Cannot Be Empty!\n");
    success = false;
    return 0;
  }
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  if(!check_parentheses_valid(0, nr_token-1)){
    *success = false;
    ERROR("Expression`s Parentheses Do not Match!\n");
    return 0;
  }

  for(int i=0; i<nr_token; i++){
    //check TK_DEREF
    if(tokens[i].type == '*' && (i==0 || 
      (tokens[i-1].type != TK_NUM && tokens[i-1].type != ')' 
      && tokens[i-1].type != TK_HEX && tokens[i-1].type != TK_REG))){
      tokens[i].type = TK_DEREF;
    }

    //check register name
    if(i<nr_token-1 && tokens[i].type == TK_REG && tokens[i+1].type == TK_NUM){
      *success = false;
      ERROR("Register Name is Valid!\n");
      return 0;
    }
  }
  return (word_t)eval(0, nr_token-1, success);
}


/* index a and b of tokens is a operator
 * return value: if priority of a is lower b, return false;
 */
bool is_lower(int a,int b){
  return  get_priority(tokens[a].type) >= get_priority(tokens[b].type);
}

/* find primary operator and return index in tokens */
int find_primary_operator(int p, int q){
  int res=-1;
  int l=p;
  int parent_flag = 0; //layer of parentheses
  while(l<q){
    if(tokens[l].type == '('){
      parent_flag++;
    }else if(tokens[l].type == ')'){
      parent_flag--;
    }else if(parent_flag || tokens[l].type == TK_NUM || tokens[l].type == TK_HEX || tokens[l].type == TK_REG){
      //if in parentheses or not a operator, check next token
      l++;
      continue;
    }else if(res==-1 || is_lower(l, res)){
      //current token is a fisrt operator or priority is lower than next token
      res = l;
    }
    l++;
  }
  return res;
}

/* Converts a hexadecimal string to a decimal number */
word_t htod(char str[]){
  word_t ans = 0;
  for(int i=0; str[i]!='\0'; i++){
    if(str[i]>='0' && str[i]<='9') ans = ans*16 + str[i] - '0';
    else if(str[i]>='a' && str[i]<='z') ans = ans*16 + str[i]-'a' + 10;
    else ans = ans*16 + str[i]-'A' + 10;
  }
  return ans;
}

/* Converts a string to a decimal number */
word_t atow(char str[]){
  word_t ans = 0;
  for(int i=0; str[i]!='\0'; i++){
    ans = ans*10+str[i]-'0';
  }
  return ans;
}

sword_t eval(int p, int q, bool *success){
  if((*success) == false) return 0;
  if(p > q){
    fprintf(stderr, "Error: Bad expression!\n");
    *success = false;
    return 0;
  }
  else if(p == q){
    if(tokens[p].type == TK_NUM) {
      return atow(tokens[p].str);
    }
    else if(tokens[p].type == TK_HEX) {
      return htod(tokens[p].str);
    }
    else {
      bool reg_success;
      word_t ret = isa_reg_str2val(tokens[p].str, &reg_success);
      if(!reg_success){
        *success = false;
        fprintf(stderr, "Error: $%s is a invalid register!", tokens[p].str);
        return 0;
      }
      return ret;
    }
  }
  else if( check_parentheses(p,q) == true){
    //if expression is surrounded by a pair of a matched
    // pair of parentheses. evaluate this expression.
    return eval(p+1,q-1,success);
  }
  else{
    int op = find_primary_operator(p,q);
    if(tokens[op].type == TK_DEREF) {
      if(tokens[q].type == TK_NUM) return vaddr_read(atoi(tokens[q].str), 4);
      else return vaddr_read(htod(tokens[q].str), 4);
    }
    sword_t val1 = eval(p, op-1,success);
    sword_t val2 = eval(op+1, q,success);
    switch (tokens[op].type)
    {
    case '+': return val1 + val2;
    case '-': return val1 - val2;
    case '*': return val1 * val2;
    case '/': 
      if(val2 == 0){
        ERROR("div-by-zero!\n");
        *success = false;
        return 0;
      }
      return val1 / val2;
    case TK_EQ: return val1 == val2;
    case TK_NE: return val1 != val2;
    case TK_AND: return val1 && val2;
    default: 
      ERROR("Unexpected Operator!\n");
      *success = false;
      return 0;
    }
  }
}

void expr_test(){
  bool success;
  int res = expr("(0-54)*12", &success);
  printf("%d", res);
}