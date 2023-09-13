#include "verilated.h"
#include "verilated_vcd_c.h"
#include "Vtop.h"
#include <iostream>

using namespace std;

VerilatedContext* contextp = NULL;
VerilatedVcdC* tfp = NULL;

uint32_t inst[] = {
    0x12300093,0x23400113,0x34500193,0x45600213,0x32108093,
};

static Vtop* top;

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
    tfp->open("wave.vcd");
}

void sim_exit(){
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
    exit(0);
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

int main(int argc, char *argv[]){

    sim_init(argc,argv);
    top->sys_clk = 0;
    top->sys_rst_n = 1;
    top->inst_in = inst[0];
    // 初始化信号
    sim_rst();
    // sim_exit();
    int count = 0;
    while (!contextp->gotFinish()){
        top->inst_in = inst[count];
        top->sys_clk = !top->sys_clk;
        step_and_dump_wave();
        top->sys_clk = !top->sys_clk;
        step_and_dump_wave();
        count++;
        if(count==6)break;
    }
    top->sys_clk = !top->sys_clk;
    sim_exit();
}