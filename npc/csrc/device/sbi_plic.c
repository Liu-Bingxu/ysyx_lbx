#include <utils.h>
#include "stdlib.h"
#include VTOP_H
#include "device.h"
#include "debug.h"

static uint32_t *sbi_plic_base = NULL;

void sbi_plic_io_handler_w(uint64_t waddr, uint64_t wdata, uint8_t wmask){
    assert((waddr - CONFIG_SBI_PLIC_MMIO) <= (2 * 1024 + 4 * CONFIG_SBI_PLIC_CONTEXT_COUNT) * 1024);
    if (wmask == 0x0f)     (*((uint32_t *)(((uint8_t *)sbi_plic_base) + waddr - CONFIG_SBI_PLIC_MMIO))) = (uint32_t)(wdata >> 0);
    else if (wmask == 0xf0)(*((uint32_t *)(((uint8_t *)sbi_plic_base) + waddr - CONFIG_SBI_PLIC_MMIO))) = (uint32_t)(wdata >> 32);
    else if (wmask == 0x03)(*((uint16_t *)(((uint8_t *)sbi_plic_base) + waddr - CONFIG_SBI_PLIC_MMIO))) = (uint16_t)(wdata >> 0);
    else if (wmask == 0x0c)(*((uint16_t *)(((uint8_t *)sbi_plic_base) + waddr - CONFIG_SBI_PLIC_MMIO))) = (uint16_t)(wdata >> 16);
    else if (wmask == 0x30)(*((uint16_t *)(((uint8_t *)sbi_plic_base) + waddr - CONFIG_SBI_PLIC_MMIO))) = (uint16_t)(wdata >> 32);
    else if (wmask == 0xc0)(*((uint16_t *)(((uint8_t *)sbi_plic_base) + waddr - CONFIG_SBI_PLIC_MMIO))) = (uint16_t)(wdata >> 48);
    else if (wmask == 0x01)(*((uint8_t * )(((uint8_t *)sbi_plic_base) + waddr - CONFIG_SBI_PLIC_MMIO))) = (uint8_t)(wdata >> 0);
    else if (wmask == 0x02)(*((uint8_t * )(((uint8_t *)sbi_plic_base) + waddr - CONFIG_SBI_PLIC_MMIO))) = (uint8_t)(wdata >> 8);
    else if (wmask == 0x04)(*((uint8_t * )(((uint8_t *)sbi_plic_base) + waddr - CONFIG_SBI_PLIC_MMIO))) = (uint8_t)(wdata >> 16);
    else if (wmask == 0x08)(*((uint8_t * )(((uint8_t *)sbi_plic_base) + waddr - CONFIG_SBI_PLIC_MMIO))) = (uint8_t)(wdata >> 24);
    else if (wmask == 0x10)(*((uint8_t * )(((uint8_t *)sbi_plic_base) + waddr - CONFIG_SBI_PLIC_MMIO))) = (uint8_t)(wdata >> 32);
    else if (wmask == 0x20)(*((uint8_t * )(((uint8_t *)sbi_plic_base) + waddr - CONFIG_SBI_PLIC_MMIO))) = (uint8_t)(wdata >> 40);
    else if (wmask == 0x40)(*((uint8_t * )(((uint8_t *)sbi_plic_base) + waddr - CONFIG_SBI_PLIC_MMIO))) = (uint8_t)(wdata >> 48);
    else if (wmask == 0x80)(*((uint8_t * )(((uint8_t *)sbi_plic_base) + waddr - CONFIG_SBI_PLIC_MMIO))) = (uint8_t)(wdata >> 56);
    else Assert(0, "now waddr is " FMT_WORD ";wdata is " FMT_WORD ";wmask is 0x%08x", waddr, wdata, wmask);
}

void sbi_plic_io_handler_r(uint64_t raddr, uint64_t *rdata){
    assert((raddr - CONFIG_SBI_PLIC_MMIO) <= (2 * 1024 + 4 * CONFIG_SBI_PLIC_CONTEXT_COUNT) * 1024);
    sbi_plic_base[1024 * 512 + 1] = 0x0;
    (*rdata) = (*((word_t *)(((uint8_t *)sbi_plic_base) + ((raddr - CONFIG_SBI_PLIC_MMIO) / 8))));
}

void init_sbi_plic(){
    sbi_plic_base = (uint32_t *)malloc((2 * 1024 + 64) * 1024);
    memset(sbi_plic_base, '\0', (2 * 1024 + 64) * 1024);
}
