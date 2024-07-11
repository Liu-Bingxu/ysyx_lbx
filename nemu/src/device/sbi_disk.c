#include <utils.h>
#include <device/map.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include "isa.h"
#include "memory/paddr.h"
#include "cpu/difftest.h"

static struct _sbi_disk{
    uint64_t *  sbi_disk_base;
    int         disk_img_fd;
    void *      disk_img;
    bool        has_disk;
} sbi_disk;

static void sbi_disk_io_handler(uint32_t offset, int len, bool is_write){
    if(!sbi_disk.has_disk){
        return;
    }
    if ((offset == 24) && is_write){
        // printf("use disk\n");
        paddr_t paddr = (paddr_t)sbi_disk.sbi_disk_base[2];
        vaddr_t vaddr = (vaddr_t)sbi_disk.sbi_disk_base[2];
        uint64_t offset = sbi_disk.sbi_disk_base[0] * 512;
        uint64_t nbytes = sbi_disk.sbi_disk_base[1] * 512;
        // printf("now use the sbi disk\n");
        if(isa_mmu_translate(&paddr, vaddr, 4, MEM_TYPE_READ) == MEM_RET_FAIL){
            Assert(0, "mmu error\n");
        }
        switch (sbi_disk.sbi_disk_base[3])
        {
        case 0:
            memcpy(guest_to_host(paddr), sbi_disk.disk_img + offset, nbytes);
            if(ref_difftest_memcpy)
                ref_difftest_memcpy(vaddr, guest_to_host(paddr), nbytes, DIFFTEST_TO_REF);
            break;
        case 1:
            memcpy(sbi_disk.disk_img + offset, guest_to_host(paddr), nbytes);
            break;
        default:
            Assert(0, "reg error\n");
            break;
        }
    }
}

void init_sbi_disk(){
    sbi_disk.sbi_disk_base = (uint64_t *)new_space(64);
    add_mmio_map("sbi_disk", CONFIG_SBI_DISK_MMIO, sbi_disk.sbi_disk_base, (64), sbi_disk_io_handler);
    memset(sbi_disk.sbi_disk_base, '\0', (64));
    sbi_disk.has_disk = false;
}

void init_sbi_disk_img(const char *disk_img_file){
    sbi_disk.disk_img_fd = open(disk_img_file, O_RDWR);
    if(sbi_disk.disk_img_fd < 0){
        printf("open disk img %s file error with %d\n", disk_img_file, sbi_disk.disk_img_fd);
        printf("errno=%d\n", errno);
        sbi_disk.has_disk = false;
        // return;
        exit(0);
    }
    sbi_disk.disk_img = mmap(NULL, 1 * 1024 * 1024 * 1024, PROT_READ | PROT_WRITE, MAP_SHARED, sbi_disk.disk_img_fd, 0);
    if (sbi_disk.disk_img == MAP_FAILED){
        printf("mmap failed\n");
        close(sbi_disk.disk_img_fd);
        sbi_disk.has_disk = false;
        return;
        // exit(0);
    }
    sbi_disk.has_disk = true;
}
