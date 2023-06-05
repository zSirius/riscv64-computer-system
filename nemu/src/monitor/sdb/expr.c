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

word_t eval(int p, int q, bool *success);
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
  {"\\$(\\$0|ra|sp|gp|tp|t[0-6]|s[0-9]|s10|s11|a[0-7])", TK_REG},

};

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
  fprintf(stderr, "Error: Invalid operator!\n");
  return 0;
}



//\\$(\\$0|ra|sp|gp|tp|t[0-6]|s[0-9]|s10|s11|a[0-7])(?![0-9a-z])

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
  char str[64];
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
          case TK_NUM: 
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            break;
          case TK_HEX:
            strncpy(tokens[nr_token].str, substr_start+2, substr_len-2);
            break;
          case TK_REG:
            strncpy(tokens[nr_token].str, substr_start+1, substr_len-1);
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

bool check_parentheses(int p, int q){
  if(tokens[p].type!='(' || tokens[q].type!=')')
    return false;
  return check_parentheses_valid(p+1, q-1);
}

word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  if(!check_parentheses_valid(0, nr_token-1)){
    *success = false;
    fprintf(stderr, "Error: The parentheses do not match!\n");
    return 0;
  }

  for(int i=0; i<nr_token; i++){
    //check TK_DEREF
    if(tokens[i].type == '*' && (i==0 || 
      (tokens[i-1].type != TK_NUM && tokens[i-1].type != ')' && tokens[i-1].type != TK_HEX && tokens[i-1].type != TK_REG))){
      tokens[i].type = TK_DEREF;
    }
    //check register name
    if(i<nr_token-1 && tokens[i].type == TK_REG && tokens[i+1].type == TK_NUM){
      *success = false;
      fprintf(stderr, "Error: Register name is error!\n");
      return 0;
    }
  }

  *success = true;
  return eval(0, nr_token-1, success);
  //return 1;
}


//return a is lower b
bool is_lower(int a,int b){
  //printf("------is_lower------\n");
  //printf("p is %d, res is %d\n", p, res);
  //printf("p type is %c, res type is %c\n\n", tokens[p].type, tokens[res].type);
  //printf("tokens[p]: %c, tokens[res]: %c\n", tokens[p].type, tokens[res].type);

  // if( (tokens[a].type=='*'||tokens[a].type=='/') && (tokens[b].type== '+' || tokens[b].type== '-') ) 
  //   return false;
  // return true;
  //printf("first type is %d , second type is %d\n", tokens[a].type, tokens[b].type);
  return  get_priority(tokens[a].type) >= get_priority(tokens[b].type);

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
    }else if(parent_flag || tokens[l].type == TK_NUM || tokens[l].type == TK_HEX || tokens[l].type == TK_REG){
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

word_t htod(char str[]){
  word_t ans = 0;
  for(int i=0; str[i]!='\0'; i++){
    if(str[i]>='0' && str[i]<='9') ans = ans*16 + str[i] - '0';
    else if(str[i]>='a' && str[i]<='z') ans = ans*16 + str[i]-'a' + 10;
    else ans = ans*16 + str[i]-'A' + 10;
  }
  return ans;
}

word_t eval(int p, int q, bool *success){
  //printf("p is %d, q is %d\n", p ,q);
  if((*success)==false) return 0;
  if( p > q){
    fprintf(stderr, "Error: Bad expression!\n");
    *success = false;
    return 0;
  }
  else if(p == q){
    if(tokens[p].type == TK_NUM) return atoi(tokens[p].str);
    else if(tokens[p].type == TK_HEX) return htod(tokens[p].str);
    else {
      bool reg_success;
      int ret = isa_reg_str2val(tokens[p].str, &reg_success);
      if(!reg_success){
        *success = false;
        fprintf(stderr, "Error: $%s is a invalid register!", tokens[p].str);
        return 0;
      }
      return ret;
    }
  }
  else if( check_parentheses(p,q) == true){
    return eval(p+1,q-1,success);
  }
  else{
    int op = find_primary_operator(p,q);
    // printf("p is %d, q is %d\n", p, q);
    // printf("op idx is %d ,  %c\n", op , tokens[op].type);
    if(op == TK_DEREF) return vaddr_read(atoi(tokens[q].str), 4);
    word_t val1 = eval(p, op-1,success);
    word_t val2 = eval(op+1, q,success);
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
    case TK_EQ: return val1 == val2;
    case TK_NE: return val1 != val2;
    case TK_AND: return val1 && val2;
    default: 
      fprintf(stderr, "Error: Unexpected operator!\n");
      *success = false;
      return 0;
    }
  }
}

void test(){
  bool success;
  word_t val = expr("*0x80000000", &success);
  // printf("---cnts of tokens:%d---\n", nr_token);
  // for(int i=0; i<nr_token; i++){
  //   printf("idx is %d, token type:%d, str:%s\n", i, tokens[i].type, tokens[i].str);
  // }
  printf("success is %d, result val = %lu\n", success, val);
  // int op = find_primary_operator(0, nr_token-1);
  // printf("primary op index is %d\n", op);
  //printf("%d\n", get_priority('+') >= get_priority(TK_AND));  


  //test htod()
  //printf("res = %lu\n", htod("ef12"));



}
//((40-90)*(((40+(18)+41/22))/98))/((5))
//12 87-(9/41)/(24)-(47)-81/43-37+10+0*40/6
//((97))-(11/((((0)))-17+21-(78)-(74)-((59))))
//324809 65+61-37+37/31-((23)+2*96+(0*36*(26)/(((39)))+87*12-(59)-(((89)))*((37)/(36))+35+24+(96)-23*((22/35))+(2))/23+(33)*3+(81)/70/((((42)))-14)-42/((13)+(5/(9))-(65)-(3)+(97)*34-99-91*97+55-82*((((((((56)+(61))))))/72-9*14-29)*13))/(89-97-(79))/15-48+(71)/(41)-39*25+90+(60+((49/40)-46)-29*20/(96)*9*30/(10)*33+(((((66/69))))-(((86-(13))))))*60+45-(((87))/5)*74/41+79-(((45)/((61)*(26*(24)/((38))-(97)))/87)))
// 4088 (((6)+58*((77))*65)/(71))    (((0x6)+0x3a*((0x4d))*65)/(71))
//(((0x6)+0x3a*((0x4d))*0x41) / 0x47)