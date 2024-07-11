include $(AM_HOME)/scripts/isa/riscv.mk
include $(AM_HOME)/scripts/platform/ysyxsoc.mk
COMMON_CFLAGS := -fno-pic -march=rv64gc -mcmodel=medany -mstrict-align # overwrite

# AM_SRCS += riscv/ysyxsoc/libgcc/div.S \
#            riscv/ysyxsoc/libgcc/muldi3.S \
#            riscv/ysyxsoc/libgcc/multi3.c \
#            riscv/ysyxsoc/libgcc/ashldi3.c \
#            riscv/ysyxsoc/libgcc/unused.c
