include $(AM_HOME)/scripts/isa/riscv.mk
include $(AM_HOME)/scripts/platform/nemu.mk
CFLAGS  += -DISA_H=\"riscv/riscv.h\"
CROSS_COMPILE := /opt/riscv/riscv-glibc-rv64imac/bin/riscv64-unknown-linux-gnu-
COMMON_CFLAGS := -fno-pic -march=rv64imac_zicsr -mcmodel=medany -mstrict-align # overwrite

AM_SRCS += riscv/nemu/start.S \
           riscv/nemu/cte.c \
           riscv/nemu/trap.S \
           riscv/nemu/vme.c
