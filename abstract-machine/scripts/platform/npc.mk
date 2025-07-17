AM_SRCS := riscv/npc/start.S \
           riscv/npc/trm.c \
           riscv/npc/ioe.c \
           riscv/npc/timer.c \
           riscv/npc/input.c \
           riscv/npc/cte.c \
           riscv/npc/trap.S \
           platform/dummy/vme.c \
           platform/dummy/mpe.c

CFLAGS    += -fdata-sections -ffunction-sections -mstrict-align -I$(AM_HOME)/am/src/riscv/npc/include
LDFLAGS   += -T $(AM_HOME)/scripts/linker.ld \
						 --defsym=_pmem_start=0x80000000 --defsym=_entry_offset=0x0
LDFLAGS   += --gc-sections -e _start
CFLAGS += -DMAINARGS=\"$(mainargs)\"
# CXXFLAGS+=-mstrict-align -mno-unaligned-access

# LIBS+=am/src/riscv/npc

NPCFLAGS += $(shell dirname $(IMAGE).elf)/npc-log.txt
NPCDIR = $(addsuffix /RTL_build,$(shell dirname $(IMAGE).elf))

# TOPNAME?=core_debugger_top
TOPNAME?=core_debugger_top_with_idcache

.PHONY: $(AM_HOME)/am/src/riscv/npc/trm.c

image: $(IMAGE).elf
	@$(OBJDUMP) -d $(IMAGE).elf > $(IMAGE).txt
	@echo + OBJCOPY "->" $(IMAGE_REL).bin
	@$(OBJCOPY) -S --set-section-flags .bss=alloc,contents -O binary $(IMAGE).elf $(IMAGE).bin
	@$(OBJCOPY) -S --set-section-flags .bss=alloc,contents -O ihex   $(IMAGE).elf $(IMAGE).hex
	@~/tools/hex2image.py --hex_file $(IMAGE).hex --image $(IMAGE).image --width 64

run:image
	$(shell if [ ! -d $(NPCDIR) ]; then mkdir -p $(NPCDIR) ;fi)
	$(MAKE) -C $(NPC_HOME) run ARGS="-w $(NPCDIR)/$(NAME)-wave.fst -l $(NPCFLAGS) $(B) -e $(IMAGE).elf -p 1234" IMG=$(IMAGE).bin IMAGE=$(IMAGE).image ISA=$(ISA) BUILD_DIR=$(NPCDIR) name=$(NAME)- TOPNAME=$(TOPNAME)

