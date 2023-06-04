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

int get_priority(int type){
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
          case TK_NUM: 
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            break;
          case TK_HEX:
            strncpy(tokens[nr_token].str, substr_start+2, substr_len-2);
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

  /* TODO: Insert codes to evaluate the expression. */
  //TODO();
  for(int i=0; i<nr_token; i++){
    if(tokens[i].type == '*' && (i==0 || (tokens[i-1].type != TK_NUM && tokens[i-1].type != ')'))){
      tokens[i].type = TK_DEREF;
    }
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


bool is_lower(int p,int res){
  //printf("------is_lower------\n");
  //printf("p is %d, res is %d\n", p, res);
  //printf("p type is %c, res type is %c\n\n", tokens[p].type, tokens[res].type);
  //printf("tokens[p]: %c, tokens[res]: %c\n", tokens[p].type, tokens[res].type);

  if( (tokens[p].type=='*'||tokens[p].type=='/') && (tokens[res].type== '+' || tokens[res].type== '-') ) 
    return false;
  return true;
  // return get_priority(tokens[p].type) >= get_priority(tokens[p].type);
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

word_t eval(int p, int q, bool *success){
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
  else if( check_parentheses(p,q) == true){
    return eval(p+1,q-1,success);
  }
  else{
    int op = find_primary_operator(p,q);
    printf("p is %d, q is %d\n", p, q);
    printf("op idx is %d ,  %c\n", op , tokens[op].type);
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
    default: 
      fprintf(stderr, "Error: Unexpected operator!\n");
      *success = false;
      return 0;
    }
  }
}

void test(){
  bool success;
  word_t val = expr(" (((14))+3-(((((22)))+(((71)))/24+((((94))+(((15*37))*91)-(21)/60+7-((90)*((14)))+(34)))*(((26+39+(4*5+56)))*(((((77)*34)/13))))/4/((((((84-(31)))))))/8)-((83)*48*(60)-(((37)))-50-(90)/(49*58*90-14)-26+((86+76/73))*10))/((((((63)-49-99)/(16-27/21*45)/34/(19))-(((53)))))*(43))-(83)*((15+47))*(87)-86+(85-((41)-39+53/(((20-46-((((65)+(85*(63)*37/84-25/2+48*(42)*(41)/47+(79+66*59)/63/95*53+(70/49/(((29+54*(80)))-((52)))/(68)-81*34*(35+74)-61+42-(((59/92)))*26+(23)-97)*99-30*40+78-(41)+(((((((54))+(((85*(57)/((36)-((9)*(((22)))*89/90-93)*(29))+72/((((((10))))))*((80))*(99)+((29)/53)-60+((82/53))+(((80))*76)))*(((((16-((6))))+(5)-71/(20)+95/(((((28)))+34))+(50))*(92)))))-((61-(8)))-36-((43)-(((86-63)))*6-((49))*((71+(13)))-51+(35)*32*(0-((0*(99)))-(((15)))/((((97*9))))*40+37-(14))))*36/(((((88)))))+((23-17)-(74)*64))))*((85/(27)*51-84))-28*(54+55/3+(72)/(((59))+29+7)*40+((45))-(25/(69)+(50)-58/(55)+(35-12+25/(71*93+51)/(38)*29/64)+((((72))-(33))+(75)/12+(((((80))-(44)))*(61)+25+(3)/(44*43)))*(14/60-((55))-(88)+((((78)))))+((58-(12)))-89)/83*((12))-66-((83)/54/33)))+51+11*(12)*50+18/(((((49)/((42*(89)+54*73-((((((((3)/32)))))-(32))*(99)*((16)*95+((97))*(((32*96/((((39))/34)/87)*94)))/71))+(10)/((64)*((25/70-41-19/81*41))*89*(71)-16)/67+(51))))+((((95)-93)*((61))))))-(8)+(16)/((39))/18)/((26)+97)/((75)+94)-((((93)/11)/(46)-(48)-(34)-(70))+9+89))/(((11)-(91+(31-(90)))*6/65)-((40))+31-33+91/(87)+4-19/(88)/(2)*83/(19/60*24)*(17+5)/74*((47+28*((((71)*34/(((((((((98)*80*48-(2/78))+62)))-79+(93)*72*(((93+70*((((42))*(27*(((65)-((64))))*(93)*56-((((((94)+12)-(((((34))*10)-34)*((98/((88*67))+53*64+50*81+(57/93)+54/(55)/23*(39))))+78*((((((((((10/(42))+65*14*(((31))))))))))+89))*(2))))-72)/95-69)/(0))))))+58/40/((((4+8-(92)*44/47+85+(((55-15/(52)))*34/(57)/(98-((((13))))-(((47)))/99-(47*91*((38)/3)+15+(79))))-(82)*((0+(((27-(((88)+(34)))+77-(((((79)))+((((27))/56+3*71)+57*10/89*58-80+(11+24-41+83/(73)))/((16+68)*75*9*(4))/5/95+((((((((21)))+3)))))*37+(11))*16-(((17))))/(64*(0)/25*(12)-((((62))-((((6-83+76+97)*28-43*57))*41+10-94-((34*(66))*50)-(22)/64)+((((25)-((26))))+65)-(39)/(44+99)))/95))*2)/(0/37/54)+((((15)-14)))+(38))/((((6/(56/(71))*38)/(7))/(18))*13/19*46+2/76)*((((((19))))+46-2-23+((96)-47)/71-(((26-26))-(57)+(23)/98*56*((90+85+86))/96/(45)+96/93+77-((((5*21))*55*7*((45))*74))/25/(40))-((((2)))*61-68)/(92-(55)/((((26)*45*(((((28/(8)-((((74))))-45/91+79+60/97/(40)-63)))))+98/11-8+(((75))))))/7/((85))+81)-((54/46))*27)+90/(7)+(72*97-(((51))-78)+2))+92/43-(71)))))*95))+36-47-25-50/(41))+(((96)-67))))))+(99-((((31)/91)))-(((20)/((79)))/57+82)-23/11+((88)/54-9/(67)+80/((((31))-(((0)/((((25)/26+(50*51))*((87)*90)-(5)))/79*((11-(48-((90))*(97)*((49)*41-16+((14)-(11)*(48))-62+(27)-((15)))+(((13)))-36*(((50+98)))*77/((34)/(92)*92-((((((12))*35-(34)/(67))-((78+94/((82))*47))+57/54*30-((49))))))-74*58-27+73/33*(91)+(46)+(6))*((((98)))))))*25))))*(((82)-(38)/99))-28-(((77))))*76))))+(69+((((78*((((85))-2+90)+85-((27)/(22))))))-84))/(10/(85)*((18))))/((93)*73))-60))/36/47*(44)/(19)+((11-(87*(73+((67+((62))-56*89-86*(44)+64/51/(79-((10+4))+38)/3+83)))/((62)*(18*((79-(8)+(((32/51)))/(((41)))-((44)/23)+(17)/(20*(62))*(49*(((((52+67+54)/50+(((81/13/54-(75-23/(16-(66)/(52)+(7))+56)))*20/(21+(32/(50)/(6)+14)*(((80-(30))))-23+((((6-25)/((82))-(((43*(((0)/(((96)-47))*54+(71)+62+(65)+59-9*(8)*((51))-42)))))-((36)))*(((9)/((44))))/17-((99))+((((((68))))))-56/(29)+19/((((84+(36+44-28/89)-(40))-(78)+((((43-44))))-(((((58))/((98))-(66*61)/(36)+41*10+(((88)))/60)/11))+((((75-((26)-19-21)*34-57*49*(7)+62-((51))/(6+3)-26-(94)+((((21))))+36)-52+3))))))-11+15*(35)*81*(((36)-(74-98)-26+60))+47*(80)-81-(((((80)+(48)+(42))/50/82)))-(42)))-(((99)))+85+(8*65/20*29)*((51/(63))/(35)+52+61+(((82))))*(30)-97+(90)+(87))+(97)*37/((98))*(81*(31)*(71)-71/((53))*3-26)/6/38)+(80-(58)/(93)+97/(((22)))+69)*((((((67)/((25+26-53)+(18))))))))))+(95)-(((45)))+18+(1*(74)*24)+(27)-65))/(4))))+88)-89-24-45/(37))))+(87))*(21)+((67)-((((((48-(48*((48)/((68))-16)-0-((72+(((60))+44)*94-33*55/64+95+79*(((8))/14-25)/(42)+7+(20)-42+(((25))/50*((((69+(11/26-(27)))*((39-32+(42/4*83-80*(26)/65+(89)/10*(92))))+80+13-82/13*76/(0)+(((29-74-((11))*(48)))*(45)))-(67)))+(21)*44+(26)*(72-(19/11-24-((62/89*49))-(2))*16/47)-11*(31))))-28*3-18/(44))-(74+11-(81-(75)+30)-19))))))/38/(33)+((54/(60))-77*(((57))-(((((23/19/20)))*(16+55)/((22))+34+(((97-(86)))))))/48))))+27-(73*70)/(((78)))))/(46+((93)/97)+((((22-85-63))-((70)-(41))))))", &success);
  // printf("cnts of tokens:%d\n", nr_token);
  // for(int i=0; i<nr_token; i++){
  //   printf("token type:%d, str:%s\n", tokens[i].type, tokens[i].str);
  // }
  printf("success is %d, result val = %lu\n", success, val);
  // int op = find_primary_operator(0, nr_token-1);
  // printf("primary op index is %d\n", op);
}
//((40-90)*(((40+(18)+41/22))/98))/((5))
//87-(9/41)/(24)-(47)-81/43-37+10+0*40/6
//((97))-(11/((((0)))-17+21-(78)-(74)-((59))))