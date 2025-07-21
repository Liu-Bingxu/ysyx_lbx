ENTRY(_start)
PHDRS { text PT_LOAD; data PT_LOAD; }

MEMORY {
    mrom  : ORIGIN = 0x20000000, LENGTH = 4K
    flash : ORIGIN = 0x30000000, LENGTH = 16M
    sram  : ORIGIN = 0x0f000000, LENGTH = 8K
    psram : ORIGIN = 0x80000000, LENGTH = 4M
}

SECTIONS {
    /* _pmem_start and _entry_offset are defined in LDFLAGS */
    /* . = _pmem_start + _entry_offset; */
    .text : {
        _text_start = .;
        . = ALIGN(0x10);
        *(entry)
        *(.text*)
        . = ALIGN(0x10);
    }> psram AT> flash  : text
    etext = .;
    _etext = .;
    .rodata : {
        . = ALIGN(0x10);
        *(.rodata*)
        *(.srodata*)
        . = ALIGN(0x10);
    }> psram AT> flash 
    _erodata = .;
    .data : {
        . = ALIGN(0x10);
        _data_start = .;
        *(.data*)
        *(.sdata*)
        . = ALIGN(0x10);
        _data_end = .;
    }> psram AT> flash : data 
    edata = .;
    _data = .;
    . = ALIGN(0x10);
    _init_start_flash = .;
    .ssbl : {
        . = ALIGN(0x10);
        _init_start = .;
        *(ssbl)
        . = ALIGN(0x10);
        _init_end = .;
    }> psram AT> flash
    .bss : {
        . = ALIGN(0x10);
        _bss_start = .;
        *(.bss*)
        *(.sbss*)
        *(.scommon)
        . = ALIGN(0x10);
        _bss_end = .;
    }> psram
    end = .;
    _end = .;
    _stack_top = ALIGN(0x10);
    . = _stack_top + 0xC00;
    _stack_pointer = .;
    _heap_start = ALIGN(0x10);
    . = _heap_start + 0x800000;
    _heap_end = .;
}
