#include <utils.h>
#include "stdlib.h"
#include VTOP_H
#include "device.h"
#include "debug.h"

static uint8_t *sbi_clint_base = NULL;
static VTOP *_top = NULL;

void update_sbi_time(uint64_t us){
    uint64_t *mtime_p = (uint64_t *)&sbi_clint_base[0xbff8];
    (*mtime_p) = us;
    //! one hart
    uint64_t *mtimecmp_p = (uint64_t *)&sbi_clint_base[0x4000];
    if ((*mtime_p) >= (*mtimecmp_p)){
        _top->mtip_asyn = 1;
    }else{
        _top->mtip_asyn = 0;
    }
}

void sbi_clint_io_handler_w(uint64_t waddr, uint64_t wdata, uint8_t wmask){
    assert((waddr - CONFIG_SBI_CLINT_MMIO) <= 0xbff8);
    if(wmask==0xff)        (*((uint64_t *)(sbi_clint_base + waddr - CONFIG_SBI_CLINT_MMIO))) = wdata;
    else if (wmask == 0x0f)(*((uint32_t *)(sbi_clint_base + waddr - CONFIG_SBI_CLINT_MMIO))) = (uint32_t)(wdata >> 0);
    else if (wmask == 0xf0)(*((uint32_t *)(sbi_clint_base + waddr - CONFIG_SBI_CLINT_MMIO))) = (uint32_t)(wdata >> 32);
    else if (wmask == 0x03)(*((uint16_t *)(sbi_clint_base + waddr - CONFIG_SBI_CLINT_MMIO))) = (uint16_t)(wdata >> 0);
    else if (wmask == 0x0c)(*((uint16_t *)(sbi_clint_base + waddr - CONFIG_SBI_CLINT_MMIO))) = (uint16_t)(wdata >> 16);
    else if (wmask == 0x30)(*((uint16_t *)(sbi_clint_base + waddr - CONFIG_SBI_CLINT_MMIO))) = (uint16_t)(wdata >> 32);
    else if (wmask == 0xc0)(*((uint16_t *)(sbi_clint_base + waddr - CONFIG_SBI_CLINT_MMIO))) = (uint16_t)(wdata >> 48);
    else if (wmask == 0x01)(*((uint8_t * )(sbi_clint_base + waddr - CONFIG_SBI_CLINT_MMIO))) = (uint8_t)(wdata >> 0);
    else if (wmask == 0x02)(*((uint8_t * )(sbi_clint_base + waddr - CONFIG_SBI_CLINT_MMIO))) = (uint8_t)(wdata >> 8);
    else if (wmask == 0x04)(*((uint8_t * )(sbi_clint_base + waddr - CONFIG_SBI_CLINT_MMIO))) = (uint8_t)(wdata >> 16);
    else if (wmask == 0x08)(*((uint8_t * )(sbi_clint_base + waddr - CONFIG_SBI_CLINT_MMIO))) = (uint8_t)(wdata >> 24);
    else if (wmask == 0x10)(*((uint8_t * )(sbi_clint_base + waddr - CONFIG_SBI_CLINT_MMIO))) = (uint8_t)(wdata >> 32);
    else if (wmask == 0x20)(*((uint8_t * )(sbi_clint_base + waddr - CONFIG_SBI_CLINT_MMIO))) = (uint8_t)(wdata >> 40);
    else if (wmask == 0x40)(*((uint8_t * )(sbi_clint_base + waddr - CONFIG_SBI_CLINT_MMIO))) = (uint8_t)(wdata >> 48);
    else if (wmask == 0x80)(*((uint8_t * )(sbi_clint_base + waddr - CONFIG_SBI_CLINT_MMIO))) = (uint8_t)(wdata >> 56);
    else
        Assert(0, "now waddr is " FMT_WORD ";wdata is " FMT_WORD ";wmask is 0x%08x", waddr, wdata, wmask);
    if((waddr - CONFIG_SBI_CLINT_MMIO) == 0xbff8){
        update_sbi_time(get_time());
    }
}

void sbi_clint_io_handler_r(uint64_t raddr, uint64_t *rdata){
    assert((raddr - CONFIG_SBI_CLINT_MMIO) <= 0xbff8);
    if((raddr - CONFIG_SBI_CLINT_MMIO) == 0xbff8){
        update_sbi_time(get_time());
    }
    raddr &= (~0x7);
    (*rdata) = (*((word_t *)(sbi_clint_base + raddr - CONFIG_SBI_CLINT_MMIO)));
}

void init_sbi_clint(VTOP *top){
    sbi_clint_base = (uint8_t *)malloc(64 * 1024);
    memset(sbi_clint_base, '\0', 64 * 1024);
    _top = top;
}
