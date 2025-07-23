include $(AM_HOME)/scripts/isa/riscv.mk
include $(AM_HOME)/scripts/platform/ysyxsoc.mk
CROSS_COMPILE := /opt/riscv/riscv-glibc-rv64imac/bin/riscv64-unknown-linux-gnu-
COMMON_CFLAGS := -fno-pic -march=rv64imac_zicsr_zifencei -mcmodel=medany -mstrict-align # overwrite

# AM_SRCS += riscv/ysyxsoc/libgcc/div.S \
#            riscv/ysyxsoc/libgcc/muldi3.S \
#            riscv/ysyxsoc/libgcc/multi3.c \
#            riscv/ysyxsoc/libgcc/ashldi3.c \
#            riscv/ysyxsoc/libgcc/unused.c
