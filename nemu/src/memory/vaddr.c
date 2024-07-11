/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <memory/paddr.h>

word_t vaddr_ifetch(vaddr_t addr, int len){
    paddr_t paddr = addr;
#ifdef CONFIG_RV64
    if((addr >> 12) != ((addr + len) >> 12)){
        if (isa_mmu_check(addr, 2, MEM_TYPE_IFETCH) == MMU_TRANSLATE){
            word_t inst;
            if (isa_mmu_translate(&paddr, addr, 2, MEM_TYPE_IFETCH) == MEM_RET_FAIL){
                isa_page_fault(MEM_TYPE_IFETCH, addr);
                return 0;
            }
            inst = paddr_read(paddr, 2);
            if (isa_mmu_translate(&paddr, addr + 2, 2, MEM_TYPE_IFETCH) == MEM_RET_FAIL){
                isa_page_fault(MEM_TYPE_IFETCH, addr);
                return 0;
            }
            // printf("%s-%d:0x%08lx get a virtaddr 0x%08x\n", __func__, __LINE__, addr, paddr);
            return (inst | (paddr_read(paddr, 2) << 16));
        }
    }else
#endif
    {
        if (isa_mmu_check(addr, len, MEM_TYPE_IFETCH) == MMU_TRANSLATE){
            if (isa_mmu_translate(&paddr, addr, len, MEM_TYPE_IFETCH) == MEM_RET_FAIL){
                isa_page_fault(MEM_TYPE_IFETCH, addr);
                return 0;
            }
            // printf("%s-%d:0x%08lx get a virtaddr 0x%08x\n", __func__, __LINE__, addr, paddr);
        }
    }
    return paddr_read(paddr, len);
}

word_t vaddr_read(vaddr_t addr, int len) {
    // Assert((addr % len) == 0, "virtaddr is "FMT_WORD", len is %d\n",addr, len);
    if((addr % len) != 0){
        isa_misalign_fault(MEM_TYPE_READ, addr);
        return 0;
    }
    paddr_t paddr = addr;
    if (isa_mmu_check(addr, len, MEM_TYPE_READ) == MMU_TRANSLATE){
        if (isa_mmu_translate(&paddr, addr, len, MEM_TYPE_READ) == MEM_RET_FAIL){
            isa_page_fault(MEM_TYPE_READ, addr);
            return 0;
        }
    }
    return paddr_read(paddr, len);
}

void vaddr_write(vaddr_t addr, int len, word_t data) {
    // assert((addr % len) == 0);
    if((addr % len) != 0){
        isa_misalign_fault(MEM_TYPE_WRITE, addr);
        return;
    }
    paddr_t paddr = addr;
    if (isa_mmu_check(addr, len, MEM_TYPE_WRITE) == MMU_TRANSLATE){
        if (isa_mmu_translate(&paddr, addr, len, MEM_TYPE_WRITE) == MEM_RET_FAIL){
            isa_page_fault(MEM_TYPE_WRITE, addr);
            return;
        }
    }
    paddr_write(paddr, len, data);
}
