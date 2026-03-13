#include "verilated.h"
#include "verilated_fst_c.h"
#include VTOP_H
#include <iostream>
// #include VTOP_DPI_H
#include "pmem.h"
#include "common.h"
#include "utils.h"
#include "debug.h"
#include "regs.h"
#include "sdb.h"
#include "remote_bitbang.h"
#ifdef USE_NVBOARD
#include "nvboard.h"
#endif
#include "stdlib.h"
#include "signal.h"
#include "lightsss.h"

using namespace std;

#define MAX_INST_TO_PRINT 10

VerilatedContext* contextp = NULL;
VerilatedFstC* tfp = NULL;
static LightSSS *lightsss = NULL;
#ifdef CONFIG_VCD_GET
static bool enable_fork = true;
#else
static bool enable_fork = false;
#endif
bool skip_ref_flag = false;

void set_skip_ref_flag(void){
    skip_ref_flag = true;
}

extern void sdb_mainloop();

static VTOP *top;
static remote_bitbang_t *remote_bitbang;
uint64_t g_nr_guest_inst = 0;
static uint64_t g_timer = 0; // unit: us
static uint64_t g_timer_rtl = 0; // unit: us
static bool g_print_step = false;
static bool int_in_commit = false;

extern void init_monitor(VTOP *top, remote_bitbang_t **remote_bitbang, int argc, char *argv[]);
extern char *get_wave_name();
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

uint64_t clock_cnt = 0;
void step_and_dump_wave(){
    IFDEF(CONFIG_GET_TIMER, uint64_t timer_start = get_time());

    top->eval();

    IFDEF(CONFIG_GET_TIMER, uint64_t timer_end = get_time());
    IFDEF(CONFIG_GET_TIMER, g_timer_rtl += timer_end - timer_start);
    // printf("Hello\n");
    contextp->timeInc(1);
    if (enable_fork && lightsss->is_child()){
        IFDEF(CONFIG_VCD_GET, tfp->dump(contextp->time()));
    }
}

static void statistic(){
    // IFDEF(CONFIG_ITRACE, irangbuf_printf());
#define NUMBERIC_FMT MUXDEF(CONFIG_TARGET_AM, "%", "%'") PRIu64
    Log("host time spent = " NUMBERIC_FMT " us", g_timer);
    Log("host time spent rtl = " NUMBERIC_FMT " us", g_timer_rtl);
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
    void isa_perform_reg_display(void);
    isa_perform_reg_display();
    delete contextp;
    delete remote_bitbang;
    IFDEF(USE_NVBOARD, nvboard_quit());
    if (enable_fork && lightsss->is_child()) {
        IFDEF(CONFIG_VCD_GET, tfp->close());
        IFDEF(CONFIG_VCD_GET, delete tfp);
    }
    if (enable_fork && !(lightsss->is_child())) {
        if (is_exit_status_bad()) {
            lightsss->wakeup_child(clock_cnt);
        } else {
            lightsss->do_clear();
        }
        delete lightsss;
    }
    exit(is_exit_status_bad());
}

void assert_fail_msg(){
    isa_reg_display();
    IFDEF(CONFIG_ITRACE, irangbuf_printf());
    set_npc_state(NPC_ABORT, get_gpr(32), -1);
    sim_exit();
}

#ifdef USE_NVBOARD
void nvboard_bind_all_pins(VTOP* top) {
    nvboard_bind_pin(&top->VGA_VSYNC, 1, VGA_VSYNC);
    nvboard_bind_pin(&top->VGA_HSYNC, 1, VGA_HSYNC);
    nvboard_bind_pin(&top->VGA_BLANK_N, 1, VGA_BLANK_N);
    nvboard_bind_pin(&top->VGA_R, 8, VGA_R7, VGA_R6, VGA_R5, VGA_R4, VGA_R3, VGA_R2, VGA_R1, VGA_R0);
    nvboard_bind_pin(&top->VGA_G, 8, VGA_G7, VGA_G6, VGA_G5, VGA_G4, VGA_G3, VGA_G2, VGA_G1, VGA_G0);
    nvboard_bind_pin(&top->VGA_B, 8, VGA_B7, VGA_B6, VGA_B5, VGA_B4, VGA_B3, VGA_B2, VGA_B1, VGA_B0);
    nvboard_bind_pin(&top->ledr, 16, LD15, LD14, LD13, LD12, LD11, LD10, LD9, LD8, LD7, LD6, LD5, LD4, LD3, LD2, LD1, LD0);
    nvboard_bind_pin(&top->sw, 16, SW15, SW14, SW13, SW12, SW11, SW10, SW9, SW8, SW7, SW6, SW5, SW4, SW3, SW2, SW1, SW0);
    // nvboard_bind_pin(&top->btn, 5, BTNL, BTNU, BTNC, BTND, BTNR);
    nvboard_bind_pin(&top->seg7, 8, SEG0A, SEG0B, SEG0C, SEG0D, SEG0E, SEG0F, SEG0G, DEC0P);
    nvboard_bind_pin(&top->seg6, 8, SEG1A, SEG1B, SEG1C, SEG1D, SEG1E, SEG1F, SEG1G, DEC1P);
    nvboard_bind_pin(&top->seg5, 8, SEG2A, SEG2B, SEG2C, SEG2D, SEG2E, SEG2F, SEG2G, DEC2P);
    nvboard_bind_pin(&top->seg4, 8, SEG3A, SEG3B, SEG3C, SEG3D, SEG3E, SEG3F, SEG3G, DEC3P);
    nvboard_bind_pin(&top->seg3, 8, SEG4A, SEG4B, SEG4C, SEG4D, SEG4E, SEG4F, SEG4G, DEC4P);
    nvboard_bind_pin(&top->seg2, 8, SEG5A, SEG5B, SEG5C, SEG5D, SEG5E, SEG5F, SEG5G, DEC5P);
    nvboard_bind_pin(&top->seg1, 8, SEG6A, SEG6B, SEG6C, SEG6D, SEG6E, SEG6F, SEG6G, DEC6P);
    nvboard_bind_pin(&top->seg0, 8, SEG7A, SEG7B, SEG7C, SEG7D, SEG7E, SEG7F, SEG7G, DEC7P);
    nvboard_bind_pin(&top->ps2_clk, 1, PS2_CLK);
    nvboard_bind_pin(&top->ps2_data, 1, PS2_DAT);
    nvboard_bind_pin(&top->uart_tx, 1, UART_TX);
    nvboard_bind_pin(&top->uart_rx, 1, UART_RX);
}
#endif

void my_handler(int param){
    set_npc_state(NPC_END, get_gpr(32), get_gpr(10));
}

void sim_init(int argc, char *argv[]){
    atexit(statistic);
    if (enable_fork) {
        lightsss = new LightSSS;
    }
    contextp = new VerilatedContext;
    top = new VTOP;
    IFDEF(USE_NVBOARD, nvboard_bind_all_pins(top));
    IFDEF(USE_NVBOARD, nvboard_init());
    contextp->commandArgs(argc, argv);
    signal(SIGINT, my_handler);
    init_gpr(top);
    top->clock = 0;
    top->rst_n = 0;
    // pmem_read(top->PC_out, &top->inst_in);
    init_monitor(top, &remote_bitbang, argc, argv);
    if (enable_fork && !(lightsss->is_child())){
        switch (lightsss->do_fork()) {
            case FORK_ERROR: set_npc_state(NPC_ABORT, get_gpr(32), 1); break;
            case FORK_CHILD:
                top->atClone(); // dump wave
                Verilated::traceEverOn(true);
#ifdef ENABLE_VCD
                tfp = new VerilatedVcdC;
#else
                tfp = new VerilatedFstC;
#endif
                top->trace(tfp, 99);
                tfp->open(get_wave_name());
            default: break;
        }
    }
    void sim_rst();
    sim_rst();
}

typedef struct
{
    uint8_t valid;
    uint8_t skip;
    uint8_t isRVC;
    uint8_t rfwen;
    uint8_t fpwen;
    uint8_t vecwen;
    uint8_t wpdest;
    uint8_t wdest;
    uint64_t pc;
    uint32_t instr;
    uint16_t robIdx;
    uint8_t lqIdx;
    uint8_t sqIdx;
    uint8_t isLoad;
    uint8_t isStore;
    uint8_t nFused;
    uint8_t special;
} DifftestInstrCommit;

static DifftestInstrCommit packet0 = {
    .valid = false
};

static DifftestInstrCommit packet1 = {
    .valid = false
};

extern "C" void difftest_InstrCommit(
    uint8_t io_skip,
    uint8_t io_isRVC,
    uint8_t io_rfwen,
    uint8_t io_fpwen,
    uint8_t io_vecwen,
    uint8_t io_wpdest,
    uint8_t io_wdest,
    uint64_t io_pc,
    uint32_t io_instr,
    uint32_t io_robIdx,
    uint8_t io_lqIdx,
    uint8_t io_sqIdx,
    uint8_t io_isLoad,
    uint8_t io_isStore,
    uint8_t io_nFused,
    uint8_t io_special,
    uint8_t io_coreid,
    uint8_t io_index)
{
    if(io_index == 0){
        packet0.valid = true;
        packet0.skip = io_skip;
        packet0.isRVC = io_isRVC;
        packet0.rfwen = io_rfwen;
        packet0.fpwen = io_fpwen;
        packet0.vecwen = io_vecwen;
        packet0.wpdest = io_wpdest;
        packet0.wdest = io_wdest;
        packet0.pc = io_pc;
        packet0.instr = io_instr;
        packet0.robIdx = io_robIdx;
        packet0.lqIdx = io_lqIdx;
        packet0.sqIdx = io_sqIdx;
        packet0.isLoad = io_isLoad;
        packet0.isStore = io_isStore;
        packet0.nFused = io_nFused;
        packet0.special = io_special;
    }else if(io_index == 1){
        packet1.valid = true;
        packet1.skip = io_skip;
        packet1.isRVC = io_isRVC;
        packet1.rfwen = io_rfwen;
        packet1.fpwen = io_fpwen;
        packet1.vecwen = io_vecwen;
        packet1.wpdest = io_wpdest;
        packet1.wdest = io_wdest;
        packet1.pc = io_pc;
        packet1.instr = io_instr;
        packet1.robIdx = io_robIdx;
        packet1.lqIdx = io_lqIdx;
        packet1.sqIdx = io_sqIdx;
        packet1.isLoad = io_isLoad;
        packet1.isStore = io_isStore;
        packet1.nFused = io_nFused;
        packet1.special = io_special;
    }else{
        Assert(0, "unkown index");
    }
}

uint32_t get_commit_inst(void){
    return packet1.valid ? packet1.instr : packet0.instr;
}

extern "C" void difftest_TrapEvent(
    uint8_t io_hasTrap,
    uint64_t io_cycleCnt,
    uint64_t io_instrCnt,
    uint8_t io_hasWFI,
    uint64_t io_code,
    uint64_t io_pc,
    uint8_t io_coreid)
{
    if (ref_difftest_raise_intr)
        ref_difftest_raise_intr(io_code);
    IFDEF(CONFIG_ETRACE, Log("Now have a tarp happen cycle is " NUMBERIC_FMT ", inst is " NUMBERIC_FMT ",code is 0x%016lx,pc is 0x%016lx", io_cycleCnt, io_instrCnt, io_code, io_pc));
    int_in_commit = true;
}

void sim_rst(){
    top->rst_n = 0;
    for (int i = 0; i < 30;i++){
        top->clock = !top->clock;
        if(i==29)
            top->rst_n = 1;
        step_and_dump_wave();
    }
    update_reg();
    set_pc(PC_RST);
}

extern "C" void halt(char code){
    set_npc_state(NPC_END, (code==1)?1:get_gpr(32), get_gpr(10));
}

static void exec_once(char *p, char *p2,paddr_t pc){
    packet0.valid = false;
    packet1.valid = false;
    int_in_commit = false;
    int cnt_now = 0;
    while ((packet0.valid == false) && (packet1.valid == false)){
        top->clock = !top->clock;
        step_and_dump_wave();
        top->clock = !top->clock;
        step_and_dump_wave();
        IFDEF(USE_NVBOARD, nvboard_update());
        clock_cnt++;
        if ((clock_cnt % 50) == 0)
            remote_bitbang->tick();
        cnt_now++;
        if (cnt_now == 4096 * 4){
            Log("now cnt_now is too big\n");
            isa_reg_display();
            IFDEF(CONFIG_ITRACE, irangbuf_printf());
            set_npc_state(NPC_ABORT, get_gpr(32), 1);
            sim_exit();
        }
    }
    set_pc(packet1.valid ? packet1.pc : packet0.pc);
    update_reg();

    //! end instr 
    if (get_commit_inst() == 0xfc000073){
        set_npc_state(NPC_END, pc, get_gpr(10));
        return;
    }

    //! skip
    // if (((packet.skip) == true) || (int_in_commit == true)){
    //     difftest_skip_ref();
    // }

    if (g_nr_guest_inst % 2500 == 0){
        void update_sbi_time(uint64_t us);
        update_sbi_time(get_time());
    }
    g_nr_guest_inst++;
    if (enable_fork && (g_nr_guest_inst % 2000 == 0) && !(lightsss->is_child())){
        switch (lightsss->do_fork()) {
            case FORK_ERROR: set_npc_state(NPC_ABORT, get_gpr(32), 1); break;
            case FORK_CHILD:
                top->atClone(); // dump wave
                Verilated::traceEverOn(true);
#ifdef ENABLE_VCD
                tfp = new VerilatedVcdC;
#else
                tfp = new VerilatedFstC;
#endif
                top->trace(tfp, 99);
                tfp->open(get_wave_name());
            default: break;
        }
    }
#ifdef CONFIG_ITRACE
    char *inst_asm = p;
    p += snprintf(p, 128, FMT_WORD ":", (pc));
    int ilen = 4;
    int i;
    word_t val;
    val = packet.instr;
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
#endif
}

#ifdef CONFIG_WATCHPOINT
static void cpu_check_watchpoint()
{
    if ((check_watchpoint() == false) && (npc_state.state != NPC_END))
        npc_state.state = NPC_STOP;
}
#endif

static void trace_and_difftest(const char *buf, paddr_t pc, paddr_t dnpc) {
#ifdef CONFIG_ITRACE_COND
    if (ITRACE_COND) { log_write(1,"%s\n", buf); }
#endif
    if (g_print_step) { IFDEF(CONFIG_ITRACE, printf("%s\n", buf));}
    IFDEF(CONFIG_ITRACE, irangbuf_write(buf));
    IFDEF(CONFIG_DIFFTEST, if((ref_difftest_exec) && packet1.valid & ( packet0.skip)){difftest_skip_ref(); difftest_step((pc), dnpc);});
    IFDEF(CONFIG_DIFFTEST, if((ref_difftest_exec) && packet1.valid & (!packet0.skip))ref_difftest_exec(1););
    IFDEF(CONFIG_DIFFTEST, if(packet1.valid ? packet1.skip : packet0.skip)difftest_skip_ref(););
    IFDEF(CONFIG_DIFFTEST, difftest_step((pc), dnpc));
    IFDEF(CONFIG_WATCHPOINT, cpu_check_watchpoint());
    IFDEF(CONFIG_FTRACE, ftrace_watch(pc, dnpc));
}

static void execute(uint64_t n)
{
    char *p = (char *)malloc(128);
    char *p2 = (char *)malloc(128);
    for (; n > 0; n--){
        paddr_t pc = get_gpr(32);
        exec_once(p, p2, pc);
        if(skip_ref_flag==true){
            skip_ref_flag = false;
        }
        paddr_t dnpc = get_gpr(32);
        if (g_print_step) {
            IFDEF(CONFIG_ITRACE, printf("ASM1: %s\n", p););
            IFDEF(CONFIG_ITRACE, printf("ASM2: %s\n", p2););
        }
        trace_and_difftest(p, pc, dnpc);
        if (npc_state.state != NPC_RUNNING)
            break;
        IFDEF(CONFIG_DEVICE, device_update());
    }
    free(p);
    free(p2);
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
    }
}

int main(int argc, char *argv[]){

    sim_init(argc,argv);
    sdb_mainloop();
    IFDEF(CONFIG_ITRACE, irangbuf_printf());
    Log(ANSI_FMT("now is normal exit", ANSI_FG_GREEN));
    sim_exit();
}