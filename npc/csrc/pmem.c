#include "pmem.h"

static char pmem[PMEM_SIZE];

void *guest_to_host(paddr_t addr){
    return (pmem + addr - PC_RST);
}

void pmem_read(uint32_t raddr,uint32_t *rdata){
    raddr &= (~0x80000003U);
    (*rdata) = (*((uint32_t *)(pmem + raddr)));
    return;
}

void pmem_write(uint32_t waddr, uint32_t wdata,char wmask){
    waddr &= (~0x80000003U);
    switch (wmask)
    {
    case 0x1:
        pmem[waddr] = (wdata & 0xfU);
        break;
    case 0x3:
        (*((uint16_t *)(pmem + waddr))) = (wdata & 0xffU);
        break;
    case 0xf:
        (*((uint32_t *)(pmem + waddr))) = wdata;
    default:
        assert(0);
        break;
    }
    return;
}
