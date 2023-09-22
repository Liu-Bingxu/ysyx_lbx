#include "verilated.h"
#include "verilated_vcd_c.h"
#include "Vtop.h"
#include <iostream>
#include "svdpi.h"
#include "Vtop__Dpi.h"
#include "pmem.h"
#include "common.h"
#include "utils.h"
#include "debug.h"
#include "regs.h"
#include "sdb.h"

using namespace std;

#define MAX_INST_TO_PRINT 10

VerilatedContext* contextp = NULL;
VerilatedVcdC* tfp = NULL;

extern void sdb_mainloop();

static Vtop *top;
uint64_t g_nr_guest_inst = 0;
static uint64_t g_timer = 0; // unit: us
static bool g_print_step = false;

extern char *init_monitor(int argc, char *argv[]);
extern void irangbuf_printf();
extern void irangbuf_write(const char *buf);
extern void device_update();

void step_and_dump_wave(){
    top->eval();
    contextp->timeInc(1);
    tfp->dump(contextp->time());
}

static void statistic(){
    // IFDEF(CONFIG_ITRACE, irangbuf_printf());
#define NUMBERIC_FMT MUXDEF(CONFIG_TARGET_AM, "%", "%'") PRIu64
    Log("host time spent = " NUMBERIC_FMT " us", g_timer);
    Log("total guest instructions = " NUMBERIC_FMT, g_nr_guest_inst);
    if (g_timer > 0)
        Log("simulation frequency = " NUMBERIC_FMT " inst/s", g_nr_guest_inst * 1000000 / g_timer);
    else
        Log("Finish running in less than 1 us and can not calculate the simulation frequency");
}

void sim_exit(int code)
{
    step_and_dump_wave();
    top->final();
    tfp->close();
    delete contextp;
    delete tfp;
    exit(code);
}

void assert_fail_msg(){
    isa_reg_display();
    statistic();
    IFDEF(CONFIG_ITRACE, irangbuf_printf());
    sim_exit(-1);
}

void sim_init(int argc, char *argv[]){
    contextp = new VerilatedContext;
    tfp = new VerilatedVcdC;
    top = new Vtop;
    contextp->traceEverOn(true);
    contextp->commandArgs(argc, argv);
    top->trace(tfp, 0);
    init_gpr(top);

    char *wave_file=init_monitor(argc, argv);
    tfp->open(wave_file);
    top->sys_clk = 0;
    top->sys_rst_n = 1;
}


void sim_rst(){
    top->sys_rst_n = 0;
    for (int i = 0; i < 10;i++){
        top->sys_clk = !top->sys_clk;
        if(i==9)
            top->sys_rst_n = 1;
        step_and_dump_wave();
    }
    top->sys_clk = !top->sys_clk;
    step_and_dump_wave();
    top->sys_clk = !top->sys_clk;
    step_and_dump_wave();
    top->sys_clk = !top->sys_clk;
    step_and_dump_wave();
    pmem_read(top->PC_out, &top->inst_in);
}

void halt(int code,int pc){
	// Log("npc: %s at pc = " FMT_WORD,((code==0)?ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) :ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED)),pc); 
    // sim_exit(code);
    Assert(-1);
    set_npc_state(NPC_END, pc, 0);
}

static void exec_once(char *p,paddr_t pc){
    pmem_read(top->PC_out, &top->inst_in);
    top->sys_clk = !top->sys_clk;
    step_and_dump_wave();
    pmem_read(top->PC_out, &top->inst_in);
    top->sys_clk = !top->sys_clk;
    step_and_dump_wave();
#ifdef CONFIG_ITRACE
    p += snprintf(p, 128, FMT_WORD ":", (pc));
    int ilen = 4;
    int i;
    word_t val;
    pmem_read(pc, &val);
    uint8_t *inst = (uint8_t *)&val;
    for (i = ilen - 1; i >= 0; i--){
        p += snprintf(p, 4, " %02x", inst[i]);
    }
    int ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4);
    int space_len = ilen_max - ilen;
    if (space_len < 0)
        space_len = 0;
    space_len = space_len * 3 + 1;
    memset(p, ' ', space_len);
    p += space_len;

    disassemble(p, p + 128 - p,pc, (uint8_t *)&val, ilen);
#endif
}

#ifdef CONFIG_WATCHPOINT
static void cpu_check_watchpoint()
{
    if ((check_watchpoint() == false) && (npc_state.state != NPC_END))
        npc_state.state = NPC_STOP;
}
#endif

static void trace_and_difftest(const char *buf,paddr_t pc, paddr_t dnpc) {
#ifdef CONFIG_ITRACE_COND
    if (ITRACE_COND) { log_write("%s\n", buf); }
#endif
    if (g_print_step) { IFDEF(CONFIG_ITRACE, printf("%s\n", buf));}
    IFDEF(CONFIG_ITRACE, irangbuf_write(buf));
    IFDEF(CONFIG_DIFFTEST, difftest_step((pc), dnpc));
    IFDEF(CONFIG_WATCHPOINT, cpu_check_watchpoint());
}

static void execute(uint64_t n)
{
    for (; n > 0; n--)
    {
        char *p = (char *)malloc(128);
        paddr_t pc = top->PC_out;
        exec_once(p, top->PC_out);
        g_nr_guest_inst++;
        trace_and_difftest(p,pc,top->PC_out);
        if (npc_state.state != NPC_RUNNING)
            break;
        IFDEF(CONFIG_DEVICE, device_update());
    }
}

void cpu_exec(uint64_t n)
{
    g_print_step = (n < MAX_INST_TO_PRINT);
    switch (npc_state.state){
    case NPC_END:
    case NPC_ABORT:
        printf("Program execution has ended. To restart the program, exit NPC and run again.\n");
        return;
    default:
        npc_state.state = NPC_RUNNING;
    }

    uint64_t timer_start = get_time();

    execute(n);

    uint64_t timer_end = get_time();
    g_timer += timer_end - timer_start;

    switch (npc_state.state){
    case NPC_RUNNING:
        npc_state.state = NPC_STOP;
        break;

    case NPC_END:
    case NPC_ABORT:
        Log("NPC: %s at pc = " FMT_WORD,
            (npc_state.state == NPC_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED) : (npc_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) : ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
            npc_state.halt_pc);
        // fall through
    case NPC_QUIT:
        statistic();
    }
}

int main(int argc, char *argv[]){

    sim_init(argc,argv);
    sim_rst();
    // int count = 0;
    // while (!contextp->gotFinish()){
    //     pmem_read(top->PC_out,&top->inst_in);
    //     top->sys_clk = !top->sys_clk;
    //     step_and_dump_wave();
    //     top->sys_clk = !top->sys_clk;
    //     step_and_dump_wave();
    //     count++;
    //     if(count==1000000)break;
    // }
    // top->sys_clk = !top->sys_clk;
    sdb_mainloop();
    sim_exit(0);
}