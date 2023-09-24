#include "sdb.h"
#include <readline/readline.h>
#include <readline/history.h>

static bool Batch_mode = false;

void sdb_set_batch_mode(){
    Batch_mode = true;
    printf("Enter the Batch mode\n");
    return;
}

void init_regex();
void init_wp_pool();

extern void sim_exit();

extern void delete_symbol_list();
extern void cpu_exec(uint64_t n);
extern void isa_reg_display();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char *rl_gets(){
    static char *line_read = NULL;

    if (line_read){
        free(line_read);
        line_read = NULL;
    }

    line_read = readline("(npc) ");

    if (line_read && *line_read){
        add_history(line_read);
    }

    return line_read;
}

static int cmd_c(char *args){
    while (npc_state.state!=NPC_END){
        cpu_exec(-1);
    }
    return 0;
}

static int cmd_q(char *args){
    IFDEF(CONFIG_FTRACE, delete_symbol_list());
    set_npc_state(NPC_QUIT, 0, 0);
    sim_exit();
    return 0;
}

static int cmd_si(char *args){
    if (args == NULL){
        cpu_exec(1);
        return 0;
    }
    int n = atoi(args);
    cpu_exec(n);
    return 0;
}

static int cmd_info(char *args){
    if (args == NULL)
        return 0;
    else if (*args == 'r')
        isa_reg_display();
    else if (*args == 'w'){
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
        char *mode = strtok(NULL, " ");
        char *N = strtok(NULL, " ");
        char *ADDR = strtok(NULL, " ");
        if (N == NULL || ADDR == NULL){
            return 0;
        }
        else{
            long n = my_atoi(N);
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
                        word_t val;
                        pmem_read(addr, &val);
                        printf("0x%08x ", val);
                        addr += 4;
                        if((y+1)%4==0)
                            printf("\n");
                    }
                    if(n%4!=0)printf("\n");
                }
                else if((*mode)=='s'){
                    for (int y = 0; y < n; y++){
                        printf_char val;
                        pmem_read(addr, &(val.val));
                        printf("%c%c%c%c",val.x1,val.x2,val.x3,val.x4);
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

static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  { "si","Step yoour program n times",cmd_si},
  {"info","Printf register or information of monitor",cmd_info},
  {"x","Printf the memory in your addr, the number is n",cmd_x},
  {"p","To calculate the value of expression",cmd_p},
  {"w","To create a watchpoint, system will stop after the vlaue of expr changing",cmd_w},
  {"d","To detele the watchpoint by NO",cmd_d},
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

void sdb_mainloop() {
  if (Batch_mode) {
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

