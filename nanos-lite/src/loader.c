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
    Elf_Ehdr ehdr;
    int len = fs_read(fd, &ehdr, sizeof(Elf_Ehdr));
    assert(len == sizeof(Elf_Ehdr));
    assert(*(uint32_t *)ehdr.e_ident == 0x464c457f);
    for (int i = 0; i < ehdr.e_phnum;i++){
        Elf_Phdr phdr;
        int offset = fs_lseek(fd, ehdr.e_phoff + (i * ehdr.e_phentsize), SEEK_SET);
        assert(offset == (ehdr.e_phoff + (i * ehdr.e_phentsize)));
        // len = ramdisk_read(&phdr, ehdr.e_phoff + (i * ehdr.e_phentsize), sizeof(Elf_Phdr));
        len = fs_read(fd, &phdr, sizeof(Elf_Phdr));
        assert(len == sizeof(Elf_Phdr));
        if(phdr.p_type==PT_LOAD){
            offset = fs_lseek(fd, phdr.p_offset, SEEK_SET);
            assert(offset == phdr.p_offset);
            len = fs_read(fd, (void *)(phdr.p_vaddr), phdr.p_memsz);
            // len = ramdisk_read((void *)(phdr.p_vaddr), phdr.p_offset, phdr.p_memsz);
            assert(len == phdr.p_memsz);
            memset((void *)(phdr.p_vaddr + phdr.p_filesz), 0, phdr.p_memsz - phdr.p_filesz);
        }
    }
    //   TODO();
    return ehdr.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

