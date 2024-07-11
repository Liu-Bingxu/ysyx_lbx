#include <utils.h>
#include <device/map.h>

static uint32_t *sbi_plic_base = NULL;

static void sbi_plic_io_handler(uint32_t offset, int len, bool is_write)
{
    uint32_t local_offset = offset;
    assert(len <= 4);
    assert(offset <= (2 * 1024 + 4 * CONFIG_SBI_PLIC_CONTEXT_COUNT) * 1024);
    if (offset > (2 * 1024 * 1024))
    {
        offset -= (2 * 1024 * 1024);
        offset &= 0xfff;
        assert(offset <= 4);
        if ((offset == 4) & (!is_write)){
            sbi_plic_base[local_offset/4] = 0x0;
        }
    }
}

void init_sbi_plic()
{
    sbi_plic_base = (uint32_t *)new_space((2 * 1024 + 64) * 1024);
    add_mmio_map("sbi_plic", CONFIG_SBI_PLIC_MMIO, sbi_plic_base, (2 * 1024 + 64) * 1024, sbi_plic_io_handler);
    memset(sbi_plic_base, '\0', (2 * 1024 + 64) * 1024);
}
