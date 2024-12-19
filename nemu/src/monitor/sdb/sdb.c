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
#include <cpu/difftest.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
#include <memory/vaddr.h>
#include <memory/paddr.h>

static int is_batch_mode = false;
extern void set_difftest_mode(bool code);

void init_regex();
void init_wp_pool();

extern void delete_symbol_list();

extern int get_func();
extern void set_func(int code);

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
    IFDEF(CONFIG_FTRACE,delete_symbol_list());
    // exit(0);
    bool succes_test = true;
    set_nemu_state(NEMU_QUIT, isa_reg_str2val("pc", &succes_test), 0);
    return -1;
}

static int cmd_si(char *args){
  if(args==NULL){
    cpu_exec(1);
    return 0;
  }
  int n = atoi(args);
  cpu_exec(n);
  return 0;
}

static int cmd_info(char *args){

	if(args==NULL)
		return 0;
	else if (*args == 'r')
		isa_reg_display();
	else if(*args=='w'){
        watchpoint_display();
    }
    else{
        assert(0);
    }
    return 0;

}

static int isnum(char argc){
  return ((argc >= 0x30) && (argc <= 0x39)) ? 1 : 0;
}

long my_atoi(const char *args){
  long res = 0;
  for (int i = 0; i < 18;i++){
    // printf("%-3d: %x\n", i, args[i]);
    if (isnum(args[i]))
    {
      res *= 10;
      res += (args[i] - 0x30);
    }
    else{
      return res;
    }
  }
  return res;
}

static int cmd_x(char *args){
	if(args==NULL){
		return 0;
	}
	else{
        char *vp = strtok(NULL, " ");
        char *mode = strtok(NULL, " ");
        char *N = strtok(NULL, " ");
        char *ADDR = strtok(NULL, " ");
        if (N == NULL || ADDR == NULL || vp == NULL){
            return 0;
        }
        else{
            long n = my_atoi(N);
            long using_virtaddr = my_atoi(vp);
            // uint32_t addr = my_atoi(ADDR);
            bool text = true;
            word_t addr = expr(ADDR, &text,false,0,NULL);

            if (n < 0 || addr < 0||text==false){
            // printf("%ld\n", n);
            // printf("%ld\n", addr);
                return 0;
            }
            else{
                // printf("Now n is %ld\n", n);
                // printf("Now addr is %ld\n", addr);
                if((*mode)=='x'){
                    for (int y = 0; y < n; y++){
                        uint32_t data = (using_virtaddr) ? (uint32_t)vaddr_read(addr, 4) : (uint32_t)paddr_read(addr, 4);
                        printf("0x%08x ", data);
                        addr += 4;
                        if((y+1)%4==0)
                            printf("\n");
                    }
                    if(n%4!=0)printf("\n");
                }
                else if((*mode)=='s'){
                    for (int y = 0; y < n; y++){
                        char data = (using_virtaddr) ? (char)vaddr_read(addr, 1) : (char)paddr_read(addr, 1);
                        printf("%c", data);
                        addr += 1;
                        if((y+1)%100==0)
                            printf("\n");
                    }
                    if(n%100!=0)printf("\n");
                }
                return 0;
            }
        }
	}
}

static int cmd_p(char *args){
    char *ex = strtok(NULL, " ");
    long value = 0;
    if (ex == NULL){
        return 0;
    }
    else{
        bool text = true;
        value = expr(ex, &text,false,0,NULL);
        if(text==true){
            printf("the value of expression is %ld or %#lx\n", value,value);
            return 0;
        }
        else{
            assert(0);
        }
    }
}

static int cmd_w(char *args){
    WP *wp;
    char *ex = strtok(NULL, " ");
    if (ex == NULL){
        return 0;
    }
    else{
        wp = new_wp();
        assert(wp != NULL);
        bool text = true;
        word_t old_value;
        old_value = expr(ex, &text, false, &wp->wp_nr_token, wp->wp_tokens);
        if (text == true){
            wp->old_value = old_value;
            printf("the watchpoint was created\n");
            return 0;
        }
        else{
            assert(0);
        }
    }
}

static int cmd_d(char *args){
    if(args==NULL){
        return 0;
    }
    int NO = my_atoi(args);
    free_wp(NO);
    return 0;
}

static int cmd_detach(char *args){
    set_difftest_mode(false);
    printf("back from difftest\n");
    return 0;
}

static int cmd_attch(char *args){
    set_difftest_mode(true);
    printf("enter the difftest mode\n");
    return 0;
}

static int cmd_save(char *args){
    if (!args){
        return 0;
    }
    FILE *save_fp = NULL;
    save_fp = fopen(args, "wb");
    assert(save_fp);
    int len=fwrite(&cpu, sizeof(cpu), 1, save_fp);
    assert(len == 1);
    len = fwrite(guest_to_host(RESET_VECTOR), CONFIG_MSIZE, 1, save_fp);
    assert(len == 1);
    int func = get_func();
    len=fwrite(&func, sizeof(func), 1, save_fp);
    assert(len == 1);
    printf("save the status to %s\n", args);
    return 0;
}

static int cmd_load(char *args){
    if(!args){
        return 0;
    }
    FILE *load_fp = NULL;
    load_fp = fopen(args, "rb");
    assert(load_fp);
    int len = fread(&cpu, sizeof(cpu), 1, load_fp);
    assert(len == 1);
    len = fread(guest_to_host(RESET_VECTOR), CONFIG_MSIZE, 1, load_fp);
    assert(len == 1);
    int func;
    len = fread(&func, sizeof(func), 1, load_fp);
    assert(len == 1);
    set_func(func);
    printf("load the status from %s\n", args);
    set_difftest_mode(false);
    set_difftest_mode(true);
    return 0;
}

static int cmd_eq(char *args){
    nemu_state.state = NEMU_END;
    Log("nemu: %s at pc = " FMT_WORD,
        (nemu_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) : ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED)),
        nemu_state.halt_pc);
    statistic();
    return -1;
}

static int cmd_pc(char *args){
    if(!args){
        return 0;
    }
    long long imm = atoll(args);
    cpu.pc += imm;
    if (ref_difftest_regcpy)
        ref_difftest_regcpy(&cpu, DIFFTEST_TO_REF);
    return 0;
}

static uint32_t inst;
static int cmd_b(char *args){
    char *ex = strtok(NULL, " ");
    if (ex == NULL){
        return 0;
    }
    bool text = true;
    long addr = expr(ex, &text, false, 0, NULL);
    if(text != true){
        assert(0);
    }
    inst = paddr_read(addr, 4);
    paddr_write(addr, 4, 0xfc000073);
    return 0;
}
static int cmd_bq(char *args){
    char *ex = strtok(NULL, " ");
    if (ex == NULL){
        return 0;
    }
    bool text = true;
    long addr = expr(ex, &text, false, 0, NULL);
    if(text != true){
        assert(0);
    }
    paddr_write(addr, 4, inst);
    return 0;
}

static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table[] = {
    {"help", "Display information about all supported commands", cmd_help},
    {"c", "Continue the execution of the program", cmd_c},
    {"q", "Exit NEMU", cmd_q},

    /* TODO: Add more commands */
    {"si", "Step yoour program n times", cmd_si},
    {"info", "Printf register or information of monitor", cmd_info},
    {"x", "Printf the memory in your addr, the number is n", cmd_x},
    {"p", "To calculate the value of expression", cmd_p},
    {"w", "To create a watchpoint, system will stop after the vlaue of expr changing", cmd_w},
    {"d", "To detele the watchpoint by NO", cmd_d},
    {"detach","To back from difftest",cmd_detach},
    {"attch","To enter the difftest mode",cmd_attch},
    {"save","To save the status",cmd_save},
    {"load","To load the status",cmd_load},
    {"eq","To quit when ebreak",cmd_eq},
    {"pc","To add the pc to jump",cmd_pc},
    {"b","To make a ebreak point",cmd_b},
    {"bq","To make a ebreak point quit",cmd_bq},
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
  if (is_batch_mode){
    //   int cnt = 0;
      if (nemu_state.state == NEMU_STOP){
          cmd_c(NULL);
        //   printf("Hello %d\n", cnt);
        //   cnt++;
        if(nemu_state.state == NEMU_STOP){
            nemu_state.state = NEMU_END;
            Log("nemu: %s at pc = " FMT_WORD,
                (nemu_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) : ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED)),
                nemu_state.halt_pc);
            statistic();
        }
      }
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
