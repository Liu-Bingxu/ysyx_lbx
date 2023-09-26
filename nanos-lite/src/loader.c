#include <proc.h>
#include <elf.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif

static uintptr_t loader(PCB *pcb, const char *filename) {
    Elf_Ehdr ehdr;
    int len = ramdisk_read(&ehdr, 0, sizeof(Elf_Ehdr));
    assert(len == sizeof(Elf_Ehdr));
    assert(*(uint32_t *)ehdr.e_ident == 0x464c457f);
    for (int i = 0; i < ehdr.e_phnum;i++){
        Elf_Phdr phdr;
        len = ramdisk_read(&phdr, ehdr.e_phoff + (i * ehdr.e_phentsize), sizeof(Elf_Phdr));
        assert(len == sizeof(Elf_Phdr));
        if(phdr.p_type==PT_LOAD){
            len = ramdisk_read((void *)(phdr.p_vaddr), phdr.p_offset, phdr.p_memsz);
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

