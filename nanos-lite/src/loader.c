#include <proc.h>
#include <elf.h>
#include "fs.h"

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

static uintptr_t loader(PCB *pcb, const char *filename) {
    int fd = fs_open(filename, 0, 0);
    assert(fd > 0);
    int offset = fs_lseek(fd, 0, SEEK_SET);
    assert(offset == 0);
    Elf_Ehdr ehdr;
    int len = fs_read(fd, &ehdr, sizeof(Elf_Ehdr));
    assert(len == sizeof(Elf_Ehdr));
    assert(*(uint32_t *)ehdr.e_ident == 0x464c457f);
    for (int i = 0; i < ehdr.e_phnum;i++){
        Elf_Phdr phdr;
        offset = fs_lseek(fd, ehdr.e_phoff + (i * ehdr.e_phentsize), SEEK_SET);
        assert(offset == (ehdr.e_phoff + (i * ehdr.e_phentsize)));
        // len = ramdisk_read(&phdr, ehdr.e_phoff + (i * ehdr.e_phentsize), sizeof(Elf_Phdr));
        len = fs_read(fd, &phdr, sizeof(Elf_Phdr));
        assert(len == sizeof(Elf_Phdr));
        if(phdr.p_type==PT_LOAD){
            offset = fs_lseek(fd, phdr.p_offset, SEEK_SET);
            assert(offset == phdr.p_offset);
            len = fs_read(fd, (void *)(phdr.p_vaddr), phdr.p_memsz);
            // len = ramdisk_read((void *)(phdr.p_vaddr), phdr.p_offset, phdr.p_memsz);
            // assert(len == phdr.p_memsz);
            memset((void *)(phdr.p_vaddr + phdr.p_filesz), 0, phdr.p_memsz - phdr.p_filesz);
        }
    }
    //   TODO();
    return ehdr.e_entry;
}
#ifdef __riscv_e
#define MYGPR0 "a5"
#else
#define MYGPR0 "a7"
#endif

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  register intptr_t _gpr1 asm(MYGPR0) = -2;
  register intptr_t _gpr2 asm("t0") = (intptr_t)filename;
  asm volatile (
        "ecall" : : "r"(_gpr2),"r"(_gpr1)
    );
  pcb->cp = ucontext(NULL, (Area){.start = pcb, .end = (pcb + 1)}, (void *)entry);
  //   ((void (*)())entry)();
}

