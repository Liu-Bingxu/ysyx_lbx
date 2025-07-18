#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include "device.h"
#include "difftest.h"
#include "debug.h"
#include "stdlib.h"
#include "pmem.h"
#include "regs.h"

#ifdef MY_DEBUG
#define debug_info printf
#else
#define debug_info(fmt, ...)
#endif

enum { MMU_DIRECT, MMU_TRANSLATE, MMU_FAIL };
enum { MEM_TYPE_IFETCH, MEM_TYPE_READ, MEM_TYPE_WRITE };
enum { MEM_RET_OK, MEM_RET_FAIL, MEM_RET_CROSS_PAGE };

extern REGS reg;

static paddr_t mmu_riscv_translate(paddr_t *paddr, vaddr_t vaddr, int len, int type){
    //! RV39
    update_reg();
    int level;
    word_t pte, index;
    word_t pte_t = (BITS(reg.satp, 43, 0) << 12);
    vaddr_t test_vaddr_0 = vaddr & (~((0x1UL << 39) - 1));
    vaddr_t test_vaddr_1 = vaddr | ((0x1UL << 39) - 1);
    privileged privilege = (reg.mstatus & (0x1UL << 17)) ? (privileged)((reg.mstatus >> 11) & 0x3) : reg.privilege;
    assert(privilege != PRV_M);
    if(paddr == NULL){
        // return MEM_RET_FAIL;
        assert(0);
    }
    if((test_vaddr_0 != 0) && (test_vaddr_1 != (vaddr_t)(-1))){
        return MEM_RET_FAIL;
    }
    for (level = 2; level >= 0;level--){
        index = ((vaddr >> ((9 * level) + 12)) & ((0x1UL << 9) - 1));
        // pte   = paddr_read(pte_t + 8 * index, 8);
        pmem_read(pte_t + 8 * index, &pte);
        //! high 10 bit is not zero
        if(pte & (((0x1UL << 11) -1) << 54)){
            debug_info("%s_%d:level is %d, pte is 0x%08lx high 10 bit is not zero\n", __func__, __LINE__, level, pte);
            return MEM_RET_FAIL;
        }
        //! pte invalid
        if(!(pte & 0x1)){
            debug_info("%s_%d:level is %d, pte is " FMT_WORD " pte invalid\n", __func__, __LINE__, level, pte);
            debug_info("pte_t is "FMT_WORD", index is %ld, vaddr is " FMT_WORD "\n", pte_t, index, vaddr);
            return MEM_RET_FAIL;
        }
        //! when r=0 but write=1
        if((!(pte & (0x1UL << 1))) && (pte & (0x1UL << 2))){
            debug_info("%s_%d:level is %d, pte is 0x%08lx when r=0 but write=1\n", __func__, __LINE__, level, pte);
            return MEM_RET_FAIL;
        }
        //! rwx == 0
        if(!(pte & (0x7 << 1))){
            // debug_info("%s_%d:level is %d, pte is 0x%08lx rwx == 0\n", __func__, __LINE__, level, pte);
            pte_t = (BITS(pte, 53, 10) << 12);
            continue;
        }
        //! no align super page
        if ((level != 0) && ((pte >> 10) & ((0x1UL << (9 * level)) - 1))){
            debug_info("%s_%d:level is %d, pte is 0x%08lx no align super page\n", __func__, __LINE__, level, pte);
            return MEM_RET_FAIL;
        }
        //? ifetch
        if (type == MEM_TYPE_IFETCH){
            //! this page can not Excute
            if(!(pte & (0x1UL << 3))){
                debug_info("%s_%d:level is %d, pte is 0x%08lx this page can not Excute\n", __func__, __LINE__, level, pte);
                return MEM_RET_FAIL;
            }
            //! Smode don't fetch the Umode page instrument
            if ((privilege == PRV_S) && (pte & (0x1UL << 4))){
                debug_info("%s_%d:level is %d, pte is 0x%08lx Smode don't fetch the Umode page instrument\n", __func__, __LINE__, level, pte);
                return MEM_RET_FAIL;
            }
            //! Umode don't fetch the Smode page instrument
            if ((privilege == PRV_U) && (!(pte & (0x1UL << 4)))){
                debug_info("%s_%d:level is %d, pte is 0x%08lx Umode don't fetch the Smode page instrument\n", __func__, __LINE__, level, pte);
                return MEM_RET_FAIL;
            }
            //! A control while A is zero
            if (!(pte & (0x1UL << 6))){
                debug_info("%s_%d:level is %d, pte is 0x%08lx A control while A is zero\n", __func__, __LINE__, level, pte);
                return MEM_RET_FAIL;
            }
            *paddr = (((pte << 2) & (~((0x1L << 12) - 1))) | (vaddr & ((0x1UL << (9 * level + 12)) - 1)));
            return MEM_RET_OK;
        }
        //? load and store
        //! this page can not read and not Exculte when MXR = 1
        if ((!(pte & (0x1UL << 1))) && (!((pte & (0x1UL << 3)) && ((reg.mstatus >> 19) & 0x1))) && (type == MEM_TYPE_READ))
        {
            debug_info("%s_%d:level is %d, pte is 0x%08lx this page can not read and not Exculte when MXR = 1\n", __func__, __LINE__, level, pte);
            return MEM_RET_FAIL;
        }
        //! this page can not write
        if((!(pte & (0x1UL << 2))) && (type == MEM_TYPE_WRITE)){
            debug_info("%s_%d:level is %d, pte is 0x%08lx this page can not write\n", __func__, __LINE__, level, pte);
            return MEM_RET_FAIL;
        }
        //! Smode don't access the Umode page data when SUM = 0
        if ((privilege == PRV_S) && (pte & (0x1UL << 4)) && (!((reg.mstatus >> 18) & 0x1))){
            debug_info("%s_%d:level is %d, pte is 0x%08lx Smode don't access the Umode page data when SUM = 0\n", __func__, __LINE__, level, pte);
            return MEM_RET_FAIL;
        }
        //! Umode don't access the Smode page data
        if ((privilege == PRV_U) && (!(pte & (0x1UL << 4)))){
            debug_info("%s_%d:level is %d, pte is 0x%08lx Umode don't access the Smode page data\n", __func__, __LINE__, level, pte);
            return MEM_RET_FAIL;
        }
        //! A/D control while A is zero or type is write but D is zero
        if ((!(pte & (0x1UL << 6))) || ((type == MEM_TYPE_WRITE) && (!(pte & (0x1UL << 7))))){
            debug_info("%s_%d:level is %d, pte is 0x%08lx A/D control while A is zero or type is write but D is zero\n", __func__, __LINE__, level, pte);
            return MEM_RET_FAIL;
        }
        *paddr = (((pte << 2) & (~((0x1L << 12) - 1))) | (vaddr & ((0x1UL << (9 * level + 12)) - 1)));
        return MEM_RET_OK;
    }
    return MEM_RET_FAIL;
}

static struct _sbi_disk{
    uint64_t *  sbi_disk_base;
    int         disk_img_fd;
    void *      disk_img;
    bool        has_disk;
} sbi_disk;

void sbi_disk_io_handler_r(uint64_t raddr, uint64_t *rdata){
    if(raddr >= (CONFIG_SBI_DISK_MMIO + 64)){
        Assert(0, "raddr error\n");
        return;
    }
    else{
        *rdata = sbi_disk.sbi_disk_base[(raddr - CONFIG_SBI_DISK_MMIO) / 8];
    }
}

void sbi_disk_io_handler_w(uint64_t waddr, uint64_t wdata, uint8_t wmask){
    if(waddr >= (CONFIG_SBI_DISK_MMIO + 64)){
        Assert(0, "raddr error\n");
        return;
    }
    else{
        uint8_t *disk_reg = (uint8_t *)sbi_disk.sbi_disk_base;
        if(wmask==0xff)     (*((uint64_t *)(disk_reg + waddr - CONFIG_SBI_DISK_MMIO))) = wdata;
        else if(wmask==0x0f)(*((uint32_t *)(disk_reg + waddr - CONFIG_SBI_DISK_MMIO))) = (uint32_t)(wdata >> 0 );
        else if(wmask==0xf0)(*((uint32_t *)(disk_reg + waddr - CONFIG_SBI_DISK_MMIO))) = (uint32_t)(wdata >> 32);
        else if(wmask==0x03)(*((uint16_t *)(disk_reg + waddr - CONFIG_SBI_DISK_MMIO))) = (uint16_t)(wdata >> 0 );
        else if(wmask==0x0c)(*((uint16_t *)(disk_reg + waddr - CONFIG_SBI_DISK_MMIO))) = (uint16_t)(wdata >> 16);
        else if(wmask==0x30)(*((uint16_t *)(disk_reg + waddr - CONFIG_SBI_DISK_MMIO))) = (uint16_t)(wdata >> 32);
        else if(wmask==0xc0)(*((uint16_t *)(disk_reg + waddr - CONFIG_SBI_DISK_MMIO))) = (uint16_t)(wdata >> 48);
        else if(wmask==0x01)(*((uint8_t  *)(disk_reg + waddr - CONFIG_SBI_DISK_MMIO))) = (uint8_t )(wdata >> 0 );
        else if(wmask==0x02)(*((uint8_t  *)(disk_reg + waddr - CONFIG_SBI_DISK_MMIO))) = (uint8_t )(wdata >> 8 );
        else if(wmask==0x04)(*((uint8_t  *)(disk_reg + waddr - CONFIG_SBI_DISK_MMIO))) = (uint8_t )(wdata >> 16);
        else if(wmask==0x08)(*((uint8_t  *)(disk_reg + waddr - CONFIG_SBI_DISK_MMIO))) = (uint8_t )(wdata >> 24);
        else if(wmask==0x10)(*((uint8_t  *)(disk_reg + waddr - CONFIG_SBI_DISK_MMIO))) = (uint8_t )(wdata >> 32);
        else if(wmask==0x20)(*((uint8_t  *)(disk_reg + waddr - CONFIG_SBI_DISK_MMIO))) = (uint8_t )(wdata >> 40);
        else if(wmask==0x40)(*((uint8_t  *)(disk_reg + waddr - CONFIG_SBI_DISK_MMIO))) = (uint8_t )(wdata >> 48);
        else if(wmask==0x80)(*((uint8_t  *)(disk_reg + waddr - CONFIG_SBI_DISK_MMIO))) = (uint8_t )(wdata >> 56);
        else assert(0);
    }
    if(!sbi_disk.has_disk){
        return;
    }
    if ((waddr - CONFIG_SBI_DISK_MMIO) == 24){
        // printf("use disk\n");
        paddr_t paddr = (paddr_t)sbi_disk.sbi_disk_base[2];
        vaddr_t vaddr = (vaddr_t)sbi_disk.sbi_disk_base[2];
        uint64_t offset = sbi_disk.sbi_disk_base[0] * 512;
        uint64_t nbytes = sbi_disk.sbi_disk_base[1] * 512;
        // printf("now use the sbi disk\n");
        printf("disk offset = %lx, nbytes = %lx\n", offset, nbytes);
        if (mmu_riscv_translate(&paddr, vaddr, 4, MEM_TYPE_READ) == MEM_RET_FAIL)
        {
            Assert(0, "mmu error\n");
        }
        switch (sbi_disk.sbi_disk_base[3])
        {
        case 0:
            memcpy(guest_to_host(paddr), sbi_disk.disk_img + offset, nbytes);
            if(ref_difftest_memcpy)
                ref_difftest_memcpy(paddr, guest_to_host(paddr), nbytes, DIFFTEST_TO_REF);
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
    sbi_disk.sbi_disk_base = (uint64_t *)malloc(64);
    memset(sbi_disk.sbi_disk_base, '\0', (64));
    sbi_disk.has_disk = false;
}

void init_sbi_disk_img(const char *disk_img_file){
    sbi_disk.disk_img_fd = open(disk_img_file, O_RDWR);
    if(sbi_disk.disk_img_fd < 0){
        printf("open disk img %s file error with %d\n", disk_img_file, sbi_disk.disk_img_fd);
        printf("errno=%d\n", errno);
        sbi_disk.has_disk = false;
        return;
        // exit(0);
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
