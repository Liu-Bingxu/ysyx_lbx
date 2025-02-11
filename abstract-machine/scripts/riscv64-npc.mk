include $(AM_HOME)/scripts/isa/riscv.mk
include $(AM_HOME)/scripts/platform/npc.mk
CROSS_COMPILE := /opt/riscv/riscv-glibc-rv64imac/bin/riscv64-unknown-linux-gnu-
COMMON_CFLAGS := -fno-pic -march=rv64imac_zicsr -mcmodel=medany -mstrict-align # overwrite

# AM_SRCS += riscv/npc/libgcc/div.S \
#            riscv/npc/libgcc/muldi3.S \
#            riscv/npc/libgcc/multi3.c \
#            riscv/npc/libgcc/ashldi3.c \
#            riscv/npc/libgcc/unused.c
