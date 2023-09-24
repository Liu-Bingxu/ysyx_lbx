AM_SRCS := riscv/npc/start.S \
           riscv/npc/trm.c \
           riscv/npc/ioe.c \
           riscv/npc/timer.c \
           riscv/npc/input.c \
           riscv/npc/cte.c \
           riscv/npc/trap.S \
           platform/dummy/vme.c \
           platform/dummy/mpe.c

CFLAGS    += -fdata-sections -ffunction-sections -mstrict-align
LDFLAGS   += -T $(AM_HOME)/scripts/linker.ld \
						 --defsym=_pmem_start=0x80000000 --defsym=_entry_offset=0x0
LDFLAGS   += --gc-sections -e _start
CFLAGS += -DMAINARGS=\"$(mainargs)\"
CXXFLAGS+=-mstrict-align -mno-unaligned-access

NPCFLAGS += $(shell dirname $(IMAGE).elf)/npc-log.txt
NPCDIR = $(addsuffix $(NAME)-RTL_build,$(dir $(abspath NPCFLAGS)))

.PHONY: $(AM_HOME)/am/src/riscv/npc/trm.c

image: $(IMAGE).elf
	@$(OBJDUMP) -d $(IMAGE).elf > $(IMAGE).txt
	@echo + OBJCOPY "->" $(IMAGE_REL).bin
	@$(OBJCOPY) -S --set-section-flags .bss=alloc,contents -O binary $(IMAGE).elf $(IMAGE).bin

run:image
	$(shell if [ ! -d $(NPCDIR)/build ]; then mkdir -p $(NPCDIR)/build ;fi)
	$(MAKE) -C $(NPC_HOME) run ARGS="-w $(NPCDIR)/$(NAME)-wave.vcd -l $(NPCFLAGS) $(B) -e $(IMAGE).elf" IMG=$(IMAGE).bin ISA=$(ISA) BUILD_DIR=$(NPCDIR) name=$(NAME)-


