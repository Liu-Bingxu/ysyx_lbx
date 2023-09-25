#include "pmem.h"
#include "device.h"

static char pmem[PMEM_SIZE];

void *guest_to_host(paddr_t addr){
    return (pmem + addr - PC_RST);
}

void pmem_read(uint32_t raddr,uint32_t *rdata){
    if(raddr==TIMER_ADDR){(*rdata)=get_timer_reg(0);return;}
    if(raddr==(TIMER_ADDR+4)){(*rdata)=get_timer_reg(1);return;}
    raddr &= (~0x80000003U);
    assert(raddr < PMEM_SIZE);
    (*rdata) = (*((uint32_t *)(pmem + raddr)));
    return;
}

void pmem_write(uint32_t waddr, uint32_t wdata,char wmask){
    if(waddr==SERIAL_ADDR){serial_out(wdata);return;}
    if((waddr==TIMER_ADDR)&&(wdata==0)){get_uptime();return;}
    if((waddr==TIMER_ADDR)&&(wdata==1)){get_rtc();return;}
    waddr &= (~0x80000003U);
    assert(waddr < PMEM_SIZE);
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
    else if(wmask==0x3)(*((uint16_t *)(pmem + waddr))) = (wdata & 0xffffU);
    // else if(wmask==0x6)(*((uint16_t *)(pmem + waddr + 1))) = (wdata & 0xffffU);
    else if(wmask==0xc)(*((uint16_t *)(pmem + waddr + 2))) = (wdata & 0xffffU);
    else if(wmask==0x1)pmem[waddr] = (wdata & 0xffU);
    else if(wmask==0x2)pmem[waddr+1] = (wdata & 0xffU);
    else if(wmask==0x4)pmem[waddr+2] = (wdata & 0xffU);
    else if(wmask==0x8)pmem[waddr+3] = (wdata & 0xffU);
    else assert(0);
    // else sdb_mainloop();
    return;
}