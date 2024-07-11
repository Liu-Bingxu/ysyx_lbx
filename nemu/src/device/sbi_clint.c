#include <utils.h>
#include <device/map.h>
#include "isa.h"

static uint8_t *sbi_clint_base = NULL;

void update_sbi_time(uint64_t us){
    uint64_t *mtime_p = (uint64_t *)&sbi_clint_base[0xbff8];
    (*mtime_p) = us * 10;
    //! one hart
    uint64_t *mtimecmp_p = (uint64_t *)&sbi_clint_base[0x4000];
    if ((*mtime_p) >= (*mtimecmp_p)){
        cpu.mip |= (1 << 7);
    }else{
        cpu.mip &= (~(1 << 7));
    }
}

static void sbi_clint_io_handler(uint32_t offset, int len, bool is_write){
    assert(len <= 8);
    assert(offset <= 0xbff8);
    if (offset == 0xbff8){
        update_sbi_time(get_time());
        return;
    }
    if (offset >= 0x4000){
        offset -= 0x4000;
        assert(offset <= 8 * (CONFIG_SBI_CLINT_HART_COUNT));
        assert((offset + len) <= 8 * (CONFIG_SBI_CLINT_HART_COUNT));
        return;
    }
    assert(offset <= 4 * (CONFIG_SBI_CLINT_HART_COUNT));
    assert((offset + len) <= 4 * (CONFIG_SBI_CLINT_HART_COUNT));
    if (is_write & (sbi_clint_base[offset] == 0x1)){
        //? todo raise a soft interrupt
    }
    return;
}

void init_sbi_clint(){
    sbi_clint_base = new_space(64 * 1024);
    add_mmio_map("sbi_clint", CONFIG_SBI_CLINT_MMIO, sbi_clint_base, 64 * 1024, sbi_clint_io_handler);
    memset(sbi_clint_base, '\0', 64 * 1024);
}
