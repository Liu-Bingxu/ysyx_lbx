#include "pmem.h"
#include "debug.h"
#include "device.h"
#include "regs.h"

extern void sim_exit();

static char pmem[PMEM_SIZE];

void *guest_to_host(paddr_t addr){
    return (pmem + addr - PC_RST);
}

void pmem_read(uint32_t raddr,uint32_t *rdata){
    if(raddr==TIMER_ADDR){(*rdata)=get_timer_reg(0);return;}
    if(raddr==(TIMER_ADDR+4)){(*rdata)=get_timer_reg(1);return;}
    raddr &= (~0x20000000U);
    // assert(raddr < PMEM_SIZE);
    if(raddr>=PMEM_SIZE){
        printf("raddr >= PMEM_SIZE\n");
        npc_state.state = NPC_END;
        npc_state.halt_pc = get_gpr(32);
        npc_state.halt_ret = 0;
        sim_exit();
    }
    (*rdata) = (*((uint32_t *)(pmem + raddr)));
    return;
}

void pmem_write(uint32_t waddr, uint32_t wdata,char wmask){
    if(waddr==SERIAL_ADDR){serial_out(wdata);return;}
    if((waddr==TIMER_ADDR)&&(wdata==0)){get_uptime();return;}
    if((waddr==TIMER_ADDR)&&(wdata==1)){get_rtc();return;}
    waddr &= (~0x80000003U);
    Assert(waddr < PMEM_SIZE,"PC is " FMT_PADDR ", try to access " FMT_PADDR ,get_gpr(32),waddr);
    // switch (wmask){
    // case 0x1:
    //     pmem[waddr] = (wdata & 0xffU);
    //     break;
    // case 0x3:
    //     (*((uint16_t *)(pmem + waddr))) = (wdata & 0xffffU);
    //     break;
    // case 0xf:
    //     (*((uint32_t *)(pmem + waddr))) = wdata;
    // default:
    //     // assert(0);
    //     printf("send wwth %d\n", wmask);
    //     break;
    // }
    if(wmask==0xf)(*((uint32_t *)(pmem + waddr))) = wdata;
    else if(wmask==0x3)(*((uint32_t *)(pmem + waddr))) = ((*((uint32_t *)(pmem + waddr)))&0xffff0000u)|(wdata & 0x0000ffffU);
    // else if(wmask==0x6)(*((uint16_t *)(pmem + waddr + 1))) = (wdata & 0xffffU);
    else if(wmask==0xc)(*((uint32_t *)(pmem + waddr))) = ((*((uint32_t *)(pmem + waddr)))&0x0000ffffu)|(wdata & 0xffff0000U);
    else if(wmask==0x1)(*((uint32_t *)(pmem + waddr))) = ((*((uint32_t *)(pmem + waddr)))&0xffffff00u)|(wdata & 0x000000ffU);
    else if(wmask==0x2)(*((uint32_t *)(pmem + waddr))) = ((*((uint32_t *)(pmem + waddr)))&0xffff00ffu)|(wdata & 0x0000ff00U);
    else if(wmask==0x4)(*((uint32_t *)(pmem + waddr))) = ((*((uint32_t *)(pmem + waddr)))&0xff00ffffu)|(wdata & 0x00ff0000U);
    else if(wmask==0x8)(*((uint32_t *)(pmem + waddr))) = ((*((uint32_t *)(pmem + waddr)))&0x00ffffffu)|(wdata & 0xff000000U);
    else assert(0);
    // else sdb_mainloop();
    return;
}

extern "C" void flash_read(int32_t addr, int32_t *data) { assert(0); }
extern "C" void mrom_read(int32_t addr, int32_t *data) {
    // *data = 0x100073;
    addr &= (~0x3U);
    // assert(raddr < PMEM_SIZE);
    if ((addr - PC_RST) >= PMEM_SIZE)
    {
        printf("raddr >= PMEM_SIZE, addr is " FMT_PADDR "\n", addr);
        npc_state.state = NPC_END;
        npc_state.halt_pc = get_gpr(32);
        npc_state.halt_ret = 0;
        sim_exit();
    }
    (*data) = *(int32_t *)(guest_to_host(addr));
}
