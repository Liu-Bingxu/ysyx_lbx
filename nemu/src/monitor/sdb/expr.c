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
#include "string.h"
#include <memory/vaddr.h>

enum {
  TK_NOTYPE = 256, TK_EQ,

  /* TODO: Add more token types */
  TK_NUM,TK_ADD,TK_SUB,TK_MUL,TK_DIV,TK_LP,TK_RP,TK_NEQ,TK_AND,TK_HEXNUM,TK_REG,TK_PIONT,TK_NEG
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +", TK_NOTYPE},			// spaces
	{"\\+", TK_ADD},			// plus
	{"-", TK_SUB},				// sub
	{"\\*", TK_MUL},			// multiplication
	{"/", TK_DIV},				// division
	{"0x[0-9a-f]+", TK_HEXNUM}, // hex number
	{"[0-9]+", TK_NUM},			// number
	{"\\(", TK_LP},				// left (
	{"\\)", TK_RP},				// right )
	{"==", TK_EQ},				// equal
	{"!=", TK_NEQ},				// no equal
	{"&&", TK_AND},				// logic and
	{"\\$(s11|s10|..)",TK_REG},	//register
};

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
  char str[32];
} Token;

static Token tokens[320] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static void strcp(char *target,const char *source,int num){
  	assert(num > 0);
	assert(target != NULL);
	if(source==NULL){
		for (int i = 0; i < 32;i++){
			target[i] = '\0';
		}
		return;
	}
	for (int i = 0; i < num; i++){
		if(source[i]=='\0'){
			assert(0);
		}
		else{
			target[i] = source[i];
		}
	}
	target[num] = '\0';
	return;
}

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

//   nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);


        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        tokens[nr_token].type = rules[i].token_type;
		strcp(tokens[nr_token].str, NULL, substr_len);
		strcp(tokens[nr_token].str, e + position, substr_len);
		assert(nr_token < 320);
		nr_token++;

		position += substr_len;

        // switch (rules[i].token_type) {
        //   default: TODO();
        // }

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
typedef enum
{
	check_expr_true,
	check_expr_error
} check_expr_type;

check_expr_type check_parentheses(int p,int q){
	int count = 1;
	if ((tokens[p].type != TK_LP) || (tokens[q].type != TK_RP)){
		return check_expr_error;
	}
	else if(q==p+1){
		assert(0);
	}
	else{
		for (int i = p+1; i < q;i++){
			if(tokens[i].type==TK_LP){
				count++;
			}
			else if(tokens[i].type==TK_RP){
				count--;
			}
			if(count==0)
				return check_expr_error;
			assert(count > 0);
		}
	}
	if(count==1)
		return check_expr_true;
	else
		return check_expr_error;
}

static long my_atoi_hex(char *args){
	long res = 0;
	for (int i = 2; args[i] != '\0';i++){
		if((args[i] & 0xF0)==0x30){
			res *= 16;
			res += (args[i] - 0x30);
		}
		else if((args[i] & 0xF0)==0x40){
			res *= 16;
			res += (args[i] - 0x37);
		}
		else if((args[i] & 0xF0)==0x60){
			res *= 16;
			res += (args[i] - 0x57);
		}
		else{
			assert(0);
		}
	}
	return res;
}

static int check_is_OP(int i){
	if((tokens[i].type==TK_ADD)||(tokens[i].type==TK_SUB)||(tokens[i].type==TK_MUL)||(tokens[i].type==TK_DIV)||(tokens[i].type==TK_EQ)||(tokens[i].type==TK_NEQ)||(tokens[i].type==TK_AND))
		return 1;
	return 0;
}

static long eval(int p,int q){
	int count = 0;
	int flag = 0;
	int op = -1;
	if (p > q){
		// printf("%s and %s, %d %d", tokens[p].str, tokens[q].str,p,q);
		assert(0);
	}
	else if(p==q){
		// printf("%s\n", tokens[p].str);
		if (tokens[p].type == TK_NUM){
			return my_atoi(tokens[p].str);
		}
		else if(tokens[p].type==TK_REG){
			bool text = true;
			word_t reg = isa_reg_str2val((tokens[p].str + 1), &text);
			if (text == true)
				return reg;
			else
				assert(0);
		}
		else if(tokens[p].type==TK_HEXNUM){
			return my_atoi_hex(tokens[p].str);
		}
		else{
			assert(0);
		}
	}
	else if(q == p+1){
		if(tokens[p].type==TK_NEG){
			return 0 - eval(q, q);
		}
		else if(tokens[p].type==TK_PIONT){
			if(tokens[p+1].type==TK_NUM){
				return vaddr_read(my_atoi(tokens[p+1].str),4);
			}
			else if(tokens[p+1].type==TK_HEXNUM){
				// printf("%ld\n",my_atoi_hex(tokens[p+1].str));
				return vaddr_read(my_atoi_hex(tokens[p + 1].str), 4);
			}
			else if(tokens[p+1].type==TK_REG){
				bool point_reg_text = true;
				word_t point_reg_addr = isa_reg_str2val((tokens[p + 1].str + 1), &point_reg_text);
				if(point_reg_text==true)
					return vaddr_read(point_reg_addr, 4);
				else
					assert(0);
			}
			else{
				assert(0);
			}
		}
		else{
			assert(0);
		}
	}
	else if((tokens[p+1].type==TK_LP)&&(check_parentheses(p + 1, q) == check_expr_true)){	
		if(tokens[p].type==TK_NEG){
			return 0 - eval(p + 1, q);
		}
		else if(tokens[p].type==TK_PIONT){
			return vaddr_read(eval(p + 1, q), 4);
		}
		else{
			assert(0);
		}
	}
	else if(check_parentheses(p,q) == check_expr_true){
		// printf("()is true\n");
		return eval(p + 1, q - 1);
	}
	// else if()
	else{
		for (int i = p; i <= q;i++){
			if(tokens[i].type==TK_LP){
				count++;
				flag = 1;
			}
			else if(tokens[i].type==TK_RP){
				count--;
				if (count == 0)
					flag = 0;
			}
			else if((check_is_OP(i)==1)&&(flag==0)){
				// printf("YES\n");
				if (op >= 0){
					if((tokens[i].type==TK_ADD)||(tokens[i].type==TK_SUB)||(tokens[op].type==TK_DIV)||(tokens[op].type==TK_MUL)){
						op = i;
					}
				}
				else{
					op = i;
				}
			}
			// printf("now i is %d, op is %d, count is %d, flag is %d\n", i, op, count,flag);
			// if(op>0)
			// 	printf("the op is %d\n", tokens[op].type);
			assert(count >= 0);
		}
		long val1 = eval(p, op - 1);
		long val2 = eval(op + 1, q);
		switch (tokens[op].type){
			case TK_ADD:
				return val1 + val2;
			case TK_SUB:
				return val1 - val2;
			case TK_MUL:
				return val1 * val2;
			case TK_DIV:
				return val1 / val2;
			default:
				assert(0);
		}
	}
}


word_t expr(char *e, bool *success) {
  while(e!=NULL){
  
    if (!make_token(e)) {
      *success = false;
      return 0;
    }
    e = strtok(NULL, " ");
  }

  	for(int i=0;i<nr_token;i++){
		if(tokens[i].type==TK_MUL){
			if(i!=0){
				if((check_is_OP(i-1)!=1)&&(tokens[i-1].type!=TK_LP)){
					break;
				}
			}
			tokens[i].type=TK_PIONT;
			if((tokens[i+1].type!=TK_NUM)&&(tokens[i+1].type!=TK_HEXNUM)&&(tokens[i+1].type!=TK_LP)&&(tokens[i+1].type!=TK_REG))
				assert(0);
		}
	}

	for(int i=0;i<nr_token;i++){
		if(tokens[i].type==TK_SUB){
			if(i!=0){
				if((check_is_OP(i-1)!=1)&&(tokens[i-1].type!=TK_LP)){
					break;
				}
			}
			tokens[i].type=TK_NEG;
			if((tokens[i+1].type!=TK_NUM)&&(tokens[i+1].type!=TK_HEXNUM)&&(tokens[i+1].type!=TK_LP)&&(tokens[i+1].type!=TK_REG))
				assert(0);
		}
	}

//   for (int i = 0; i < 32;i++){
//     printf("%3d: %-20s\n", tokens[i].type, tokens[i].str);
//   }

  word_t val = eval(0, (nr_token - 1));

  //   printf("now is %ld\n", eval(0, (nr_token - 1)));
  /* TODO: Insert codes to evaluate the expression. */
//   TODO();

  nr_token = 0;

  return val;
}
