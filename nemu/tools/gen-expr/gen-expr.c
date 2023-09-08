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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

int piont = 0;

static void gen_num(){
  if(piont==65535)
    return;
  uint32_t val = rand();
  int now_piont = piont;
  int temp_piont;
  while (val != 0)
  {
    char inttochar = (val % 10) + 0x30;
    buf[now_piont] = inttochar;
    now_piont++;
    val /= 10;
  }
  temp_piont = now_piont - 1;
  for (int i = piont; i < temp_piont; i++,temp_piont--)
  {
    char temp;
    temp = buf[i];
    buf[i] = buf[temp_piont];
    buf[temp_piont] = temp;
  }
  piont = now_piont;
  // buf[piont] = 'U';
  // piont++;
}

static void gen_rand_op(){
  if(piont==65535)
    return;
  switch (rand()%4)
  {
  case 0:
    buf[piont] = '+';
    piont++;
    break;
  case 1:
    buf[piont] = '-';
    piont++;
    break;
  case 2:
    buf[piont] = '*';
    piont++;
    break;
  case 3:
    buf[piont] = '/';
    piont++;
    break;
  default:
    break;
  }
}

static void gen_back(){
  if(piont==65535)
    return;
  int y = rand()%5;
  for (int i = 0; i < y;i++){
    buf[piont] = ' ';
    piont++;
  }
}

static void gen_rand_expr() {
  if(piont==65535)
    return;
  switch (rand() % 3)
  {
  case 0:
    gen_num();
    break;
  case 1:
    buf[piont] = '(';
    piont++;
    gen_back();
    gen_rand_expr();
    gen_back();
    buf[piont] = ')';
    piont++;
    break;
  default:
    gen_rand_expr();
    gen_back();
    gen_rand_op();
    gen_back();
    gen_rand_expr();
    break;
  }
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    do{
      piont = 0;
      gen_rand_expr();
      buf[piont] = '\0';
    }while(piont==65535);

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    ret = fscanf(fp, "%d", &result);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
