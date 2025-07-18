ENTRY(_start)
PHDRS { text PT_LOAD; data PT_LOAD; }

MEMORY {
    mrom : ORIGIN = 0x20000000, LENGTH = 4K
    sram : ORIGIN = 0x0f000000, LENGTH = 8K
}

SECTIONS {
    /* _pmem_start and _entry_offset are defined in LDFLAGS */
    . = _pmem_start + _entry_offset;
    .text : {
        *(entry)
        *(.text*)
    }> mrom AT> mrom  : text
    etext = .;
    _etext = .;
    .rodata : {
        *(.rodata*)
        *(.srodata*)
        . = ALIGN(0x10);
    }> mrom AT> mrom 
    _erodata = .;
    .data : {
        . = ALIGN(0x10);
        _data_start = .;
        *(.data*)
        *(.sdata*)
        . = ALIGN(0x10);
        _data_end = .;
    }> sram AT> mrom : data 
    edata = .;
    _data = .;
    .bss : {
        . = ALIGN(0x10);
        _bss_start = .;
        *(.bss*)
        *(.sbss*)
        *(.scommon)
        _bss_end = .;
        . = ALIGN(0x10);
    }> sram
    end = .;
    _end = .;
    _stack_top = ALIGN(0x200);
    . = _stack_top + 0x1000;
    _stack_pointer = .;
    _heap_start = ALIGN(0x200);
    . = 0x0f002000;
    _heap_end = .;
}
