#include "verilated.h"
#include "verilated_vcd_c.h"
#include "VysyxSoCFull.h"
#include <iostream>
#include "VysyxSoCFull__Dpi.h"
#include "pmem.h"
#include "common.h"
#include "utils.h"
#include "debug.h"
#include "regs.h"
#include "sdb.h"

using namespace std;

#define MAX_INST_TO_PRINT 10

// #define PC_now top->rootp->
// #define DUT_inst top->rootp->top__DOT__u_ifu__DOT__inst_reg
// #define npc_ifu_status top->rootp->top__DOT__u_ifu__DOT__IFU_FSM_STATUS_ADDR
#define ls_valid_flag top->rootp->ysyxSoCFull__DOT__asic__DOT__cpu__DOT__cpu__DOT__u_core_top__DOT__u_lsu__DOT__u_ls_valid__DOT__data_out_reg

VerilatedContext* contextp = NULL;
VerilatedVcdC* tfp = NULL;

bool skip_ref_flag=false;

void set_skip_ref_flag(void){
    skip_ref_flag = true;
}

extern void sdb_mainloop();

static VysyxSoCFull *top;
uint64_t g_nr_guest_inst = 0;
static uint64_t g_timer = 0; // unit: us
static bool g_print_step = false;

extern void init_monitor(VysyxSoCFull *top, VerilatedVcdC *tfp, int argc, char *argv[]);
extern void irangbuf_printf();
extern void irangbuf_write(const char *buf);
extern void ftrace_watch(paddr_t pc,paddr_t pc_jump);
extern void device_update();
extern int  is_exit_status_bad();

//icache hit rate
static uint64_t icache_hit_num = 0;
static uint64_t icache_access_num = 0;
void icache_access(void){
    icache_access_num++;
}
void icache_hit(void){
    icache_hit_num++;
}
//icache hit rate

// dcache hit rate
static uint64_t dcache_hit_num = 0;
static uint64_t dcache_access_num = 0;
static uint64_t mmio_access_num = 0;
void dcache_access(void){
    dcache_access_num++;
}
void dcache_hit(void){
    dcache_hit_num++;
}
void mmio_access(void){
    mmio_access_num++;
}
// dcache hit rate

static uint64_t clock_cnt = 0;
void step_and_dump_wave(){
    // static int cnt = 0;
    // cnt++;
    // if(cnt==2){
    //     cnt = 0;
    //     clock_cnt++;
    // }
    top->eval();
    // printf("Hello\n");
    contextp->timeInc(1);
    IFDEF(CONFIG_VCD_GET, tfp->dump(contextp->time()));
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
    Log("the clock_num is %ld, the ipc is %f", clock_cnt , 1.0 * g_nr_guest_inst / clock_cnt);
    Log("the icache hit num is %ld, the icache access num is %ld", icache_hit_num, icache_access_num);
    Log("the icache hit rate is %lf", icache_hit_num * (double)1.0 / icache_access_num);
    Log("the dcache hit num is %ld, the dcache access num is %ld", dcache_hit_num, dcache_access_num);
    Log("the dcache hit rate is %lf", dcache_hit_num * (double)1.0 / dcache_access_num);
    Log("the mmio access num is %ld", mmio_access_num);
}

void sim_exit(){
    step_and_dump_wave();
    top->final();
    IFDEF(CONFIG_VCD_GET, tfp->close());
    delete contextp;
    IFDEF(CONFIG_VCD_GET, delete tfp);
    statistic();
    exit(is_exit_status_bad());
}

void assert_fail_msg(){
    isa_reg_display();
    statistic();
    IFDEF(CONFIG_ITRACE, irangbuf_printf());
    set_npc_state(NPC_ABORT, get_gpr(32), -1);
    sim_exit();
}

void sim_init(int argc, char *argv[]){
    contextp = new VerilatedContext;
    IFDEF(CONFIG_VCD_GET, tfp = new VerilatedVcdC);
    top = new VysyxSoCFull;
    contextp->traceEverOn(true);
    contextp->commandArgs(argc, argv);
    IFDEF(CONFIG_VCD_GET, top->trace(tfp, 0));
    init_gpr(top);
    top->clock = 0;
    top->reset = 0;
    // pmem_read(top->PC_out, &top->inst_in);
    init_monitor(top,tfp,argc, argv);
}


void sim_rst(){
    top->reset = 1;
    for (int i = 0; i < 10;i++){
        top->clock = !top->clock;
        if(i==9)
            top->reset = 0;
        step_and_dump_wave();
    }
    // top->sys_clk = !top->sys_clk;
    // step_and_dump_wave();
    // top->sys_clk = !top->sys_clk;
    // step_and_dump_wave();
    // while(npc_ifu_status==0){
    //     top->sys_clk = !top->sys_clk;
    //     step_and_dump_wave();
    // }
    while (ls_valid_flag == 0){
        top->clock = !top->clock;
        step_and_dump_wave();
        top->clock = !top->clock;
        step_and_dump_wave();
        clock_cnt++;
        if(clock_cnt == 40){
            isa_reg_display();
            IFDEF(CONFIG_ITRACE, irangbuf_printf());
            set_npc_state(NPC_END, get_gpr(32), 0);
            sim_exit();
        }
    }
    // top->sys_clk = !top->sys_clk;
    // step_and_dump_wave();
    // top->sys_clk = !top->sys_clk;
    // step_and_dump_wave();
}

void halt(char code){
	// Log("npc: %s at pc = " FMT_WORD,((code==0)?ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) :ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED)),pc); 
    // sim_exit(code);
    // assert_fail_msg();
    set_npc_state(NPC_END, (code==1)?1:get_gpr(32), get_gpr(10));
}

static void exec_once(char *p, char *p2,paddr_t pc){
    // printf("H\n");
#ifdef CONFIG_ITRACE
    char *inst_asm = p;
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
    printf("ASM1: %s\n", inst_asm);

    char *inst_asm2 = p2;
    p2 += snprintf(p2, 128, FMT_WORD ":", (pc));
    int ilen2 = 4;
    int i2;
    word_t val2 = top->rootp->ysyxSoCFull__DOT__asic__DOT__cpu__DOT__cpu__DOT__u_core_top__DOT__u_lsu__DOT__u_inst__DOT__data_out_reg;
    uint8_t *inst2 = (uint8_t *)&val2;
    for (i2 = ilen2 - 1; i2 >= 0; i2--){
        p2 += snprintf(p2, 4, " %02x", inst2[i2]);
    }
    int ilen_max2 = MUXDEF(CONFIG_ISA_x86, 8, 4);
    int space_len2 = ilen_max2 - ilen2;
    if (space_len2 < 0)
        space_len2 = 0;
    space_len2 = space_len2 * 3 + 1;
    memset(p2, ' ', space_len2);
    p2 += space_len2;
    disassemble(p2, p2 + 128 - p2, pc, (uint8_t *)&val2, ilen2);
    printf("ASM2: %s\n", inst_asm2);
    // if(strcmp(inst_asm,inst_asm2)!=0){
    //     npc_state.state = NPC_END;
    //     npc_state.halt_pc = get_gpr(32);
    //     printf("%s\n", inst_asm2);
    //     npc_state.halt_ret = 0;
    //     Log(ANSI_FMT("inst error\n", ANSI_FG_RED));
    //     sim_exit();
    // }
#endif
    // pmem_read(top->PC_out, &top->inst_in);
    // printf("%d\n", g_nr_guest_inst);
    // top->sys_clk = !top->sys_clk;
    // step_and_dump_wave();
    // printf("%d\n", g_nr_guest_inst);
    // pmem_read(top->PC_out, &top->inst_in);
    // top->sys_clk = !top->sys_clk;
    // step_and_dump_wave();

    // while(npc_ifu_status!=3){
    //     top->sys_clk = !top->sys_clk;
    //     step_and_dump_wave();
    //     top->sys_clk = !top->sys_clk;
    //     step_and_dump_wave();
    //     clock_cnt++;
    // }
    // while(npc_ifu_status!=1){
    //     top->sys_clk = !top->sys_clk;
    //     step_and_dump_wave();
    //     top->sys_clk = !top->sys_clk;
    //     step_and_dump_wave();
    //     clock_cnt++;
    // }

    if ((top->rootp->ysyxSoCFull__DOT__asic__DOT__cpu__DOT__cpu__DOT__u_core_top__DOT__u_lsu__DOT__u_inst__DOT__data_out_reg)==0x100073){
        set_npc_state(NPC_END, get_gpr(32), get_gpr(10));
        return;
    }
    // fflush(stdout);

    top->clock = !top->clock;
    step_and_dump_wave();
    top->clock = !top->clock;
    step_and_dump_wave();
    clock_cnt++;
    int cnt_now = 0;
    while (ls_valid_flag == 0){
        // if (get_time() >= 800000){
        //     npc_state.state = NPC_END;
        //     npc_state.halt_pc = get_gpr(32);
        //     npc_state.halt_ret = 0;
        //     sim_exit();
        // }
        top->clock = !top->clock;
        step_and_dump_wave();
        top->clock = !top->clock;
        step_and_dump_wave();
        clock_cnt++;
        cnt_now++;
        if (cnt_now == 100000){
            isa_reg_display();
            IFDEF(CONFIG_ITRACE, irangbuf_printf());
            set_npc_state(NPC_END, get_gpr(32), 0);
            sim_exit();
        }
    }

    //debug icache mutil sram in a way 2023.10.26
    // uint32_t _inst;
    // pmem_read(pc, &_inst);
    // Assert(DUT_inst == _inst, "error: PC is " FMT_PADDR ", read data is " FMT_WORD ", but true data is " FMT_WORD, pc, DUT_inst, _inst);
    // if (DUT_inst != _inst){
    //     Log(ANSI_FMT("error: PC is " FMT_PADDR ", read data is " FMT_WORD ", but true data is " FMT_WORD "\n", ANSI_FG_RED), pc, DUT_inst, _inst);
    //     npc_state.state = NPC_END;
    //     npc_state.halt_pc = get_gpr(32);
    //     npc_state.halt_ret = 0;
    //     sim_exit();
    // }

    // debug dcache mutil sram in a way 2023.10.27
    //  uint32_t _inst;
    //  pmem_read(pc, &_inst);
    //  Assert(DUT_inst == _inst, "error: PC is " FMT_PADDR ", read data is " FMT_WORD ", but true data is " FMT_WORD, pc, DUT_inst, _inst);
    //  if (DUT_inst != _inst){
    //      Log(ANSI_FMT("error: PC is " FMT_PADDR ", read data is " FMT_WORD ", but true data is " FMT_WORD "\n", ANSI_FG_RED), pc, DUT_inst, _inst);
    //      npc_state.state = NPC_END;
    //      npc_state.halt_pc = get_gpr(32);
    //      npc_state.halt_ret = 0;
    //      sim_exit();
    //  }

    // debug dcache mutil sram in a way 2023.10.27
    //  uint32_t _inst;
    //  pmem_read(pc, &_inst);
    //  Assert(DUT_inst == _inst, "error: PC is " FMT_PADDR ", read data is " FMT_WORD ", but true data is " FMT_WORD, pc, DUT_inst, _inst);
    //  if (DUT_inst != _inst){
    //      Log(ANSI_FMT("error: PC is " FMT_PADDR ", read data is " FMT_WORD ", but true data is " FMT_WORD "\n", ANSI_FG_RED), pc, DUT_inst, _inst);
    //      npc_state.state = NPC_END;
    //      npc_state.halt_pc = get_gpr(32);
    //      npc_state.halt_ret = 0;
    //      sim_exit();
    //  }

    // #ifdef CONFIG_ITRACE
    // p += snprintf(p, 128, FMT_WORD ":", (pc));
    // int ilen = 4;
    // int i;
    // word_t val;
    // pmem_read(pc, &val);
    // uint8_t *inst = (uint8_t *)&val;
    // for (i = ilen - 1; i >= 0; i--){
    // p += snprintf(p, 4, " %02x", inst[i]);
    // }
    // int ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4);
    // int space_len = ilen_max - ilen;
    // if (space_len < 0)
    // space_len = 0;
    // space_len = space_len * 3 + 1;
    // memset(p, ' ', space_len);
    // p += space_len;
// 
    // disassemble(p, p + 128 - p,pc, (uint8_t *)&val, ilen);
// #endif
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
    // if (ITRACE_COND) { log_write(1,"%s\n", buf); }
#endif
    // if (g_print_step) { IFDEF(CONFIG_ITRACE, printf("%s\n", buf));}
    IFDEF(CONFIG_ITRACE, irangbuf_write(buf));
    IFDEF(CONFIG_DIFFTEST, difftest_step((pc), dnpc));
    IFDEF(CONFIG_WATCHPOINT, cpu_check_watchpoint());
    IFDEF(CONFIG_FTRACE, ftrace_watch(pc, dnpc));
}

static void execute(uint64_t n)
{
    for (; n > 0; n--)
    {
        char *p = (char *)malloc(128);
        char *p2 = (char *)malloc(128);
        // paddr_t pc = get_gpr(32);
        // paddr_t pc = top->rootp->top__DOT__u_ifu__DOT__PC_to_sram_reg;
        if(skip_ref_flag==true){
            skip_ref_flag = false;
            difftest_skip_ref();
        }
        paddr_t pc = top->rootp->ysyxSoCFull__DOT__asic__DOT__cpu__DOT__cpu__DOT__u_core_top__DOT__u_lsu__DOT__u_PC__DOT__data_out_reg;
        exec_once(p, p2, pc);
        g_nr_guest_inst++;
        paddr_t dnpc = top->rootp->ysyxSoCFull__DOT__asic__DOT__cpu__DOT__cpu__DOT__u_core_top__DOT__u_lsu__DOT__u_PC__DOT__data_out_reg;
        set_pc(dnpc);
        // Log("dnpc: 0x%08x\n", dnpc);
        // if((g_nr_guest_inst%1000)==0){
        // log_write(1, "now program runing %d inst, PC is" FMT_WORD "\n", g_nr_guest_inst,get_gpr(32));
        // }
        trace_and_difftest(p, pc, dnpc);
        if (npc_state.state != NPC_RUNNING)
            break;
        // if (get_time() >= 1900000){
        //     npc_state.state = NPC_END;
        //     npc_state.halt_pc = get_gpr(32);
        //     npc_state.halt_ret = 0;
        //     printf("\n");
        // }
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

    IFDEF(CONFIG_GET_TIMER, uint64_t timer_start = get_time());

    execute(n);

    IFDEF(CONFIG_GET_TIMER, uint64_t timer_end = get_time());
    IFDEF(CONFIG_GET_TIMER, g_timer += timer_end - timer_start);

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
    // case NPC_QUIT:
        // statistic();
    }
}

int main(int argc, char *argv[]){

    sim_init(argc,argv);
    // sim_rst();
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
    IFDEF(CONFIG_ITRACE, irangbuf_printf());
    sim_exit();
}