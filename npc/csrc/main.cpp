#include "verilated.h"
#include "verilated_vcd_c.h"
#include "Vtop.h"
#include <iostream>
#include "svdpi.h"
#include "Vtop__Dpi.h"
#include "pmem.h"

using namespace std;

VerilatedContext* contextp = NULL;
VerilatedVcdC* tfp = NULL;

static Vtop* top;

extern char *init_monitor(int argc, char *argv[]);

void step_and_dump_wave(){
    // cout << "Hello Wrold" << top->sys_clk << "ByeBye" << endl;
    // printf("%d\n", top->sys_clk);
    top->eval();
    contextp->timeInc(1);
    tfp->dump(contextp->time());
}

void sim_init(int argc, char *argv[]){
    contextp = new VerilatedContext;
    tfp = new VerilatedVcdC;
    top = new Vtop;
    contextp->traceEverOn(true);
    contextp->commandArgs(argc, argv);
    top->trace(tfp, 0);

    char *wave_file=init_monitor(argc, argv);
    tfp->open(wave_file);
}

void sim_exit(int code){
    step_and_dump_wave();
    top->final();
    tfp->close();
    // printf("Hello\n");
    delete contextp;
    // printf("Hello\n");
    delete tfp;
    // printf("Hello\n");
    // delete top;
    // printf("Hello\n");
    exit(code);
    // printf("Hello\n");
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
}

void halt(int code){
    sim_exit(code);
}

int main(int argc, char *argv[]){

    sim_init(argc,argv);
    top->sys_clk = 0;
    top->sys_rst_n = 1;
    pmem_read(top->PC_out, &top->inst_in);
    // 初始化信号
    sim_rst();
    // sim_exit();
    int count = 0;
    while (!contextp->gotFinish()){
        pmem_read(top->PC_out,&top->inst_in);
        top->sys_clk = !top->sys_clk;
        step_and_dump_wave();
        top->sys_clk = !top->sys_clk;
        step_and_dump_wave();
        count++;
        if(count==1000000)break;
    }
    top->sys_clk = !top->sys_clk;
    sim_exit(0);
}