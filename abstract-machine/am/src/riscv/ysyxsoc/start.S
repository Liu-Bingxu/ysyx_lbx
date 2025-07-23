.section entry, "ax"
.globl _start
.type _start, @function

#if __riscv_xlen == 64
#define RISCV_PTR		.dword
#define RISCV_SZPTR		8
#define RISCV_LGPTR		3
#define REG_L           ld
#define REG_S           sd
#elif __riscv_xlen == 32
#define RISCV_PTR		.word
#define RISCV_SZPTR		4
#define RISCV_LGPTR		2
#define REG_L           lw
#define REG_S           sw
#else
#error "Unexpected __SIZEOF_POINTER__"
#endif

.macro load_data,_src_start,_dst_start,_dst_end
	bgeu	\_dst_start, \_dst_end, 2f
1:
	REG_L   t0, (\_src_start)
	REG_S   t0, (\_dst_start)
	addi    \_src_start, \_src_start, RISCV_SZPTR
	addi    \_dst_start, \_dst_start, RISCV_SZPTR
	bltu    \_dst_start, \_dst_end, 1b
2:
.endm

.macro clear_bss,_dst_start,_dst_end
	bgeu	\_dst_start, \_dst_end, 2f
1:
	REG_S   zero, (\_dst_start)
	addi    \_dst_start, \_dst_start, RISCV_SZPTR
	bltu    \_dst_start, \_dst_end, 1b
2:
.endm

_start:
    //load ssbl
	//[0x30000000] --> [0x0f000000:_data_end]
    .globl _init_start_flash_addr
    REG_L	a0,	_init_start_flash_addr
    li     t3,0xa0000000
    sub    a0,a0,t3
    li     t3,0x30000000
    add    a0,a0,t3
    .globl _init_start_addr
    REG_L	a1,	_init_start_addr
    .globl _init_end_addr
	REG_L	a2,	_init_end_addr

    .globl _init
    la     t1,_init
    sub    t1,t1,a0
    add    t1,t1,a1

	load_data a0,a1,a2

    li		a0,	0x30000000
    li		a1,	0xa0000000
    .globl _data_end_addr
	REG_L	a2,	_data_end_addr

    la     t2,reloc
    sub    t2,t2,a0
    add    t2,t2,a1

    fence.i
    jr     t1

    //load app
	//[0x30000000] --> [0x0f000000:_data_end]
    // li		a0,	0x30000000
    // li		a1,	0x80000000
    // .globl _data_end_addr
	// REG_L	a2,	_data_end_addr

	// load_data a0,a1,a2

reloc:
    .globl _bss_start_addr
	REG_L	a0,	_bss_start_addr
    .globl _bss_end_addr
	REG_L	a1,	_bss_end_addr
    clear_bss a0,a1

    // jr     t1

    mv s0, zero
    la sp, _stack_pointer
    jal _trm_init

.align 3
_init_start_flash_addr:
    RISCV_PTR _init_start_flash

_init_start_addr:
    RISCV_PTR _init_start

_init_end_addr:
    RISCV_PTR _init_end

_data_end_addr:
    RISCV_PTR _data_end

_bss_start_addr:
    RISCV_PTR _bss_start

_bss_end_addr:
    RISCV_PTR _bss_end

.section ssbl, "ax"
.globl _init
.type _init, @function

_init:
    load_data a0,a1,a2

    fence.i
    jr t2
