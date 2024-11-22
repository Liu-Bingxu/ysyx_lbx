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
#include <memory/vaddr.h>
#include <memory/paddr.h>

#define SATP_CHECK (cpu.satp & MUXDEF(CONFIG_RV64, (0x1UL << 63), (0x1UL << 31)))

#ifdef CONFIG_RV64
static paddr_t mmu_riscv_translate(paddr_t *paddr, vaddr_t vaddr, int len, int type){
    //! RV39
    int level;
    word_t pte, index;
    word_t pte_t = (BITS(cpu.satp, 43, 0) << 12);
    vaddr_t test_vaddr_0 = vaddr & (~((0x1UL << 39) - 1));
    vaddr_t test_vaddr_1 = vaddr | ((0x1UL << 39) - 1);
    privileged privilege = (cpu.mstatus & (0x1UL << 17)) ? ((cpu.mstatus >> 11) & 0x3) : cpu.privilege;
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
        pte   = paddr_read(pte_t + 8 * index, 8);
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
        if((!(pte & (0x1UL << 1))) && (!((pte & (0x1UL << 3)) && ((cpu.mstatus >> 19) & 0x1))) && (type == MEM_TYPE_READ)){
            debug_info("%s_%d:level is %d, pte is 0x%08lx this page can not read and not Exculte when MXR = 1\n", __func__, __LINE__, level, pte);
            return MEM_RET_FAIL;
        }
        //! this page can not write
        if((!(pte & (0x1UL << 2))) && (type == MEM_TYPE_WRITE)){
            debug_info("%s_%d:level is %d, pte is 0x%08lx this page can not write\n", __func__, __LINE__, level, pte);
            return MEM_RET_FAIL;
        }
        //! Smode don't access the Umode page data when SUM = 0
        if ((privilege == PRV_S) && (pte & (0x1UL << 4)) && (!((cpu.mstatus >> 18) & 0x1))){
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

#else
static paddr_t mmu_riscv_translate(paddr_t *paddr, vaddr_t vaddr, int len, int type);
#endif

int isa_mmu_check(vaddr_t vaddr, int len, int type){
    if((!SATP_CHECK) || ((cpu.mstatus & (0x1UL << 17)) && (((cpu.mstatus >> 11) & 0x3) == 0x3))){
        return MMU_DIRECT;
    }

    if (cpu.privilege <= PRV_S){
        return MMU_TRANSLATE;
    }

    if ((type != MEM_TYPE_IFETCH) && (cpu.mstatus & (0x1UL << 17)) && (((cpu.mstatus >> 11) & 0x3) != 0x3)){
        return MMU_TRANSLATE;
    }

    return MMU_DIRECT;
}

int isa_mmu_translate(paddr_t* paddr, vaddr_t vaddr, int len, int type) {
    return mmu_riscv_translate(paddr, vaddr, len, type);
}
