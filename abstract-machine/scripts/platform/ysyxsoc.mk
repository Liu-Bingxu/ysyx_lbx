AM_SRCS := riscv/ysyxsoc/start.S \
           riscv/ysyxsoc/trm.c \
           riscv/ysyxsoc/ioe.c \
           riscv/ysyxsoc/timer.c \
           riscv/ysyxsoc/input.c \
           riscv/ysyxsoc/cte.c \
           riscv/ysyxsoc/trap.S \
           platform/dummy/vme.c \
           platform/dummy/mpe.c

CFLAGS    += -fdata-sections -ffunction-sections -mstrict-align -I$(AM_HOME)/am/src/riscv/ysyxsoc/include
LDFLAGS   += -T $(AM_HOME)/scripts/linker-ysyxsoc.ld \
						 --defsym=_pmem_start=0x30000000 --defsym=_entry_offset=0x0
LDFLAGS   += --gc-sections -e _start
CFLAGS += -DMAINARGS=\"$(mainargs)\"
# CXXFLAGS+=-mstrict-align -mno-unaligned-access

# LIBS+=am/src/riscv/ysyxsoc

YSYXSOCFLAGS += $(shell dirname $(IMAGE).elf)/ysyxsoc-log.txt
YSYXSOCDIR = $(addsuffix /RTL_build,$(shell dirname $(IMAGE).elf))

.PHONY: $(AM_HOME)/am/src/riscv/ysyxsoc/trm.c

image: $(IMAGE).elf
	@$(OBJDUMP) -d $(IMAGE).elf > $(IMAGE).txt
	@echo + OBJCOPY "->" $(IMAGE_REL).bin
	@$(OBJCOPY) -S --set-section-flags .bss=alloc,contents -O binary $(IMAGE).elf $(IMAGE).bin

run:image
	$(shell if [ ! -d $(YSYXSOCDIR) ]; then mkdir -p $(YSYXSOCDIR) ;fi)
	$(MAKE) -C $(NPC_HOME) run ARGS="-w $(YSYXSOCDIR)/$(NAME)-wave.fst -l $(YSYXSOCFLAGS) $(B) -e $(IMAGE).elf -p 1234" IMG=$(IMAGE).bin ISA=$(ISA) BUILD_DIR=$(YSYXSOCDIR) name=$(NAME)- TOPNAME=ysyxsoc_sim

