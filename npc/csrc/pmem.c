#include "pmem.h"
#include "debug.h"
#include "device.h"
#include "regs.h"

extern void assert_fail_msg();

static char pmem[PMEM_SIZE];
static char mrom[MROM_SIZE];
static char flash[FLASH_SIZE];
static uint32_t vmem[VMEM_SIZE] = {0};

static inline bool in_pmem(paddr_t addr){
    return ((addr >= PMEM_START) && (addr < (PMEM_START + PMEM_SIZE)));
}

static inline bool in_mrom(paddr_t addr){
    return ((addr >= MROM_START) && (addr < (MROM_START + MROM_SIZE)));
}

static inline bool in_flash(paddr_t addr){
    return ((addr >= FLASH_START) && (addr < (FLASH_START + FLASH_SIZE)));
}

void *guest_to_host(paddr_t addr){
    if(in_pmem(addr)){
        return (pmem + addr - PMEM_START);
    }else if(in_mrom(addr)){
        return (mrom + addr - MROM_START);
    }else if(in_flash(addr)){
        return (flash + addr - FLASH_START);
    }else{
        Assert(0, "now try to read " FMT_PADDR, addr);
    }
}

void pmem_read(word_t raddr,word_t *rdata){
    raddr &= (~0x80000000U);
    // assert(raddr < PMEM_SIZE);
    if(raddr>=PMEM_SIZE){
        printf("raddr >= PMEM_SIZE\n");
        printf("now try to read " FMT_WORD ":%ld\n", raddr, raddr);
        npc_state.state = NPC_END;
        npc_state.halt_pc = get_gpr(32);
        npc_state.halt_ret = 0;
        assert_fail_msg();
    }
    (*rdata) = (*((word_t *)(pmem + raddr)));
    // printf("now read addr: " FMT_PADDR " now rdata is " FMT_WORD "\n", raddr, *rdata);
    return;
}

void pmem_write(uint64_t waddr, uint64_t wdata,uint8_t wmask){
    waddr &= (~0x80000007U);
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
    if(wmask==0xff)(*((uint64_t *)(pmem + waddr)))          = wdata;
    else if(wmask==0x0f)(*((uint32_t *)(pmem + waddr + 0))) = (uint32_t)(wdata >> 0 );
    else if(wmask==0xf0)(*((uint32_t *)(pmem + waddr + 4))) = (uint32_t)(wdata >> 32);
    else if(wmask==0x03)(*((uint16_t *)(pmem + waddr + 0))) = (uint16_t)(wdata >> 0 );
    else if(wmask==0x0c)(*((uint16_t *)(pmem + waddr + 2))) = (uint16_t)(wdata >> 16);
    else if(wmask==0x30)(*((uint16_t *)(pmem + waddr + 4))) = (uint16_t)(wdata >> 32);
    else if(wmask==0xc0)(*((uint16_t *)(pmem + waddr + 6))) = (uint16_t)(wdata >> 48);
    else if(wmask==0x01)(*((uint8_t  *)(pmem + waddr + 0))) = (uint8_t )(wdata >> 0 );
    else if(wmask==0x02)(*((uint8_t  *)(pmem + waddr + 1))) = (uint8_t )(wdata >> 8 );
    else if(wmask==0x04)(*((uint8_t  *)(pmem + waddr + 2))) = (uint8_t )(wdata >> 16);
    else if(wmask==0x08)(*((uint8_t  *)(pmem + waddr + 3))) = (uint8_t )(wdata >> 24);
    else if(wmask==0x10)(*((uint8_t  *)(pmem + waddr + 4))) = (uint8_t )(wdata >> 32);
    else if(wmask==0x20)(*((uint8_t  *)(pmem + waddr + 5))) = (uint8_t )(wdata >> 40);
    else if(wmask==0x40)(*((uint8_t  *)(pmem + waddr + 6))) = (uint8_t )(wdata >> 48);
    else if(wmask==0x80)(*((uint8_t  *)(pmem + waddr + 7))) = (uint8_t )(wdata >> 56);
    else assert(0);
    // else sdb_mainloop();
    return;
}

extern "C" void sim_vmem_read(uint32_t haddr, uint32_t vaddr, uint32_t *rdata){
    uint32_t addr = vaddr * 640 + haddr;
    if(unlikely((vaddr > 480) || (haddr > 640))){
        assert(0);
    }
    *(rdata) = vmem[addr];
}

extern "C" void sim_vmem_write(uint32_t waddr, uint32_t wdata, uint8_t wmask){
    uint32_t addr = ((waddr - VMEM_START) / 4);
    if(unlikely(addr > VMEM_SIZE)){
        assert(0);
    }
         if(wmask==0x0f)(*((uint32_t *)((uint8_t *)(vmem + addr) + 0))) = (uint32_t)(wdata >> 0 );
    else if(wmask==0x03)(*((uint16_t *)((uint8_t *)(vmem + addr) + 0))) = (uint16_t)(wdata >> 0 );
    else if(wmask==0x0c)(*((uint16_t *)((uint8_t *)(vmem + addr) + 2))) = (uint16_t)(wdata >> 16);
    else if(wmask==0x01)(*((uint8_t  *)((uint8_t *)(vmem + addr) + 0))) = (uint8_t )(wdata >> 0 );
    else if(wmask==0x02)(*((uint8_t  *)((uint8_t *)(vmem + addr) + 1))) = (uint8_t )(wdata >> 8 );
    else if(wmask==0x04)(*((uint8_t  *)((uint8_t *)(vmem + addr) + 2))) = (uint8_t )(wdata >> 16);
    else if(wmask==0x08)(*((uint8_t  *)((uint8_t *)(vmem + addr) + 3))) = (uint8_t )(wdata >> 24);
    else assert(0);
}

extern "C" void sim_sram_read(uint64_t raddr, uint64_t *rdata){
    raddr &= (~0x7U);
    pmem_read(raddr, rdata);
}

extern "C" void sim_sram_write(uint64_t waddr, uint64_t wdata, uint8_t wmask){
    pmem_write(waddr, wdata, wmask);
}

extern "C" void sim_periph_read(uint64_t raddr, uint64_t *rdata){
    if((raddr==TIMER_ADDR) || (raddr==(TIMER_ADDR + 4))){
        (*rdata) = ((uint64_t)get_timer_reg(0) | ((uint64_t)get_timer_reg(1) << 32));
        set_skip_ref_flag();
        return;
    }
    if ((raddr >= CONFIG_SBI_DISK_MMIO) && (raddr < CONFIG_SBI_DISK_MMIO + 64)){
        sbi_disk_io_handler_r(raddr, rdata);
        set_skip_ref_flag();
        return;
    }
    if ((raddr >= CONFIG_SBI_SERIAL_MMIO) && (raddr < CONFIG_SBI_SERIAL_MMIO + 8)){
        sbi_serial_io_handler_r(raddr, rdata);
        set_skip_ref_flag();
        return;
    }
    if ((raddr >= CONFIG_SBI_CLINT_MMIO) && (raddr < CONFIG_SBI_CLINT_MMIO + 64 * 1024)){
        sbi_clint_io_handler_r(raddr, rdata);
        set_skip_ref_flag();
        return;
    }
    if ((raddr >= CONFIG_SBI_PLIC_MMIO) && (raddr < CONFIG_SBI_PLIC_MMIO + (2 * 1024 + 64) * 1024)){
        sbi_plic_io_handler_r(raddr, rdata);
        set_skip_ref_flag();
        return;
    }
}

extern "C" void sim_periph_write(uint64_t waddr, uint64_t wdata, uint8_t wmask){
    if(waddr==SERIAL_ADDR){
        serial_out((char)wdata);
        set_skip_ref_flag();
        return;
    }
    if((waddr==TIMER_ADDR)&&(wdata==0)){
        get_uptime();
        set_skip_ref_flag();
        return;
    }
    if((waddr==TIMER_ADDR)&&(wdata==1)){
        get_rtc();
        set_skip_ref_flag();
        return;
    }
    if ((waddr >= CONFIG_SBI_DISK_MMIO) && (waddr < CONFIG_SBI_DISK_MMIO + 64)){
        sbi_disk_io_handler_w(waddr, wdata, wmask);
        set_skip_ref_flag();
        return;
    }
    if ((waddr >= CONFIG_SBI_SERIAL_MMIO) && (waddr < CONFIG_SBI_SERIAL_MMIO + 8)){
        sbi_serial_io_handler_w(waddr, wdata, wmask);
        set_skip_ref_flag();
        return;
    }
    if ((waddr >= CONFIG_SBI_CLINT_MMIO) && (waddr < CONFIG_SBI_CLINT_MMIO + 64 * 1024))
    {
        sbi_clint_io_handler_w(waddr, wdata, wmask);
        set_skip_ref_flag();
        return;
    }
    if ((waddr >= CONFIG_SBI_PLIC_MMIO) && (waddr < CONFIG_SBI_PLIC_MMIO + (2 * 1024 + 64) * 1024)){
        sbi_plic_io_handler_w(waddr, wdata, wmask);
        set_skip_ref_flag();
        return;
    }
}

extern "C" void flash_read(int32_t addr, int32_t *data) { 
    // assert(0);
    assert(addr < FLASH_SIZE);
    addr &= (~0x3U);
    (*data) = *(int32_t *)(guest_to_host(addr + FLASH_START));
}
extern "C" void mrom_read(int32_t addr, int32_t *data) {
    // (*data) = 0xfc000073;
    // (*data) = 0x100073;
    assert(addr < (MROM_START + MROM_SIZE));
    addr &= (~0x3U);
    // if ((addr - PC_RST) >= PMEM_SIZE)
    // {
    //     printf("raddr >= PMEM_SIZE, addr is " FMT_PADDR "\n", addr);
    //     npc_state.state = NPC_END;
    //     npc_state.halt_pc = get_gpr(32);
    //     npc_state.halt_ret = 0;
    //     sim_exit();
    // }
    (*data) = *(int32_t *)(guest_to_host(addr));
}
