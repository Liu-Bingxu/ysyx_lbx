# TOPNAME=core_top
TOPNAME?=ysyxSoCFull
# vsrc_dir=../../ysyx-for-LiuBingxu/vsrc
# vsrc_dir+=../ysyxSoC/perip/uart16550/rtl
# vsrc_dir+=../ysyxSoC/perip/spi/rtl
vsrc_dir?=../../ysyx-for-LiuBingxu
MODULE_SRC = $(shell find $(abspath $(vsrc_dir)) -name "*.v" -or -name "*.sv")
VINC_file=$(sort $(dir $(shell find $(abspath $(vsrc_dir)) -name "*.v" -or -name "*.sv")))
# MODULE_SRC+= $(shell find $(abspath ../ysyxSoC/perip) -name "*.v" -or -name "*.sv")
# MODULE_SRC+=./vsrc/ysyx_23060081.v
TOPMODULE = $(shell find $(abspath $(vsrc_dir)) -name "$(TOPNAME).v")
CSRC = $(shell find $(abspath ./csrc) -name "*.c" -or -name "*.cpp" -or -name "*.cc")#
HEAD_FILE=$(shell find $(abspath ./) -name "*.h")
INC_file=$(sort $(dir $(shell find $(abspath ./) -name "*.h")))
LIBS?=
LIBS+=-lreadline

override ARGS ?= --log=$(BUILD_DIR)/npc-log.txt --wave=$(BUILD_DIR)/$(name)wave.fst -b -p 9825 
override ARGS += -d $(NEMU_HOME)/build/riscv64-nemu-interpreter-so

IMG?=$(NPC_HOME)/rot13.bin 

CONFIG_ITRACE=true

CFLAGS_add=$(addprefix -CFLAGS ,$(1))
CFLAGS_inc_add=$(addprefix -CFLAGS -I,$(1))
CFLAGS_lib_add=$(addprefix -CFLAGS -l,$(1))
LD_add=$(addprefix -LDFLAGS ,$(1))
module_add=$(addprefix -v ,$(1))
vsrc_dir_add=$(addprefix -I,$(1))

BUILD_DIR?=$(NPC_HOME)/build
name?=

include $(NPC_HOME)/csrc/utils/filelist.mk

CXXFLAGS+=-DVTOP=V$(TOPNAME) -DVTOP_H=\\\"V$(TOPNAME).h\\\" -DVTOP_DPI_H=\\\"V$(TOPNAME)__Dpi.h\\\" -D__GUEST_ISA__=$(ISA)

VSRC_DIR=$(call vsrc_dir_add,$(VINC_file))
MODULE=$(call module_add,$(MODULE_SRC))
INC=$(call CFLAGS_inc_add,$(INC_file))
MY_CXXFLAGS=$(call CFLAGS_add,$(CXXFLAGS))
MY_LDFLAGS=$(call LD_add,$(LIBS))

all:run
	@echo "Write this Makefile by your self."

sim:$(BUILD_DIR)/$(name)wave.fst
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
# @echo "Write this Makefile by your self."

.PHONY:verilate
verilate: $(NPC_HOME)/.stamp.verilate

.PHONY:run
run:sim $(BUILD_DIR)/$(name)wave.fst

.PHONY:waves
waves: $(BUILD_DIR)/$(name)wave.fst
	@echo
	@echo "### WAVES ###"
	@gtkwave $(BUILD_DIR)/$(name)wave.fst

$(BUILD_DIR)/$(name)wave.fst: $(NPC_HOME)/build/V$(TOPNAME) $(IMG)
	@echo
	@echo "### SIMULATING ###"
	@echo "$(NPC_HOME)/build/V$(TOPNAME) $(ARGS) $(IMG) +verilator+rand+reset+2"
	$(NPC_HOME)/build/V$(TOPNAME) $(ARGS) $(IMG) +verilator+rand+reset+2

$(NPC_HOME)/build/V$(TOPNAME): $(NPC_HOME)/.stamp.verilate
	@echo
	@echo "### BUILDING SIM ###"
	@echo "make -C $(NPC_HOME)/build -f V$(TOPNAME).mk V$(TOPNAME) "
	make -C $(NPC_HOME)/build -f V$(TOPNAME).mk V$(TOPNAME) 

$(NPC_HOME)/.stamp.verilate: $(MODULE_SRC) $(CSRC) $(HEAD_FILE)
	@if [ -d $(NPC_HOME)/build ]; then echo "build dir have build";else mkdir -p $(NPC_HOME)/build; fi
	@echo
	@echo "### VERILATING ###"
	@echo verilator --Mdir $(NPC_HOME)/build --unroll-count 1000 --trace-fst --x-assign unique --x-initial unique --timing --top-module $(TOPNAME)  -cc $(TOPMODULE) --exe $(CSRC) $(INC) $(VSRC_DIR) $(MY_CXXFLAGS) $(MY_LDFLAGS) -Wno-WIDTHTRUNC -Wno-WIDTHEXPAND
	@verilator --Mdir $(NPC_HOME)/build --autoflush --unroll-count 1000 --trace-fst --x-assign unique --x-initial unique --timescale "1ns/1ns" --no-timing --top-module $(TOPNAME)  -cc $(TOPMODULE) --exe $(CSRC) $(INC) $(VSRC_DIR) $(MY_CXXFLAGS) $(MY_LDFLAGS) -Wno-WIDTHTRUNC -Wno-WIDTHEXPAND
	@touch $(NPC_HOME)/.stamp.verilate

.PHONY:lint
lint: $(MODULE_SRC) 
	verilator --lint-only $(MODULE_SRC) $(VSRC_DIR) --top-module $(TOPNAME) -Wall --timescale "1ns/1ns" --no-timing

# WORK_PATH=$(shell echo $(PWD))
# SDC_FILE = $(shell find $(abspath ./) -name "*.sdc")
# .PHONY:sta
# sta:
# 	@rm -rf $(WORK_PATH)/result
# 	make clean -C ~/yosys-sta -f Makefile
# 	make sta DESIGN=core SDC_FILE=$(SDC_FILE) RTL_FILES="$(MODULE_SRC)" CLK_FREQ_MHZ=100 -C ~/yosys-sta/ -f Makefile
# 	@cp -r ~/yosys-sta/result/ $(WORK_PATH)/

.PHONY: clean
clean:
	rm -rf .stamp.*;
	rm -rf ./build
	rm -rf wave.fst
	rm -rf *.txt

include ../Makefile
