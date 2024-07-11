include $(AM_HOME)/scripts/isa/riscv.mk
include $(AM_HOME)/scripts/platform/nemu.mk
COMMON_CFLAGS := -fno-pic -march=rv64gc -mcmodel=medany -mstrict-align # overwrite

CFLAGS  += -DISA_H=\"riscv/riscv.h\" -DRV64_LBX

OBJDUMP += -S
CFLAGS  += -g -M no-aliases,numeric

AM_SRCS += riscv/nemu/start.S \
           riscv/nemu/cte.c \
           riscv/nemu/trap.S \
           riscv/nemu/vme.c
