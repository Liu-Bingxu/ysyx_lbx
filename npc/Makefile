# TOPNAME=core_top
TOPNAME?=ysyxsoc_sim
# vsrc_dir=../../ysyx-for-LiuBingxu/vsrc
# vsrc_dir+=../ysyxSoC/perip/uart16550/rtl
# vsrc_dir+=../ysyxSoC/perip/spi/rtl
vsrc_dir?=../../ysyx-for-LiuBingxu
MODULE_SRC = $(shell find $(abspath $(vsrc_dir)) -name "*.v" -or -name "*.sv")
VINC_file=$(sort $(dir $(shell find $(abspath $(vsrc_dir)) -name "*.v" -or -name "*.sv")))
MODULE_SRC+= $(shell find $(abspath /home/kuuga/ysyx-workbench/ysyxSoC/perip) -name "*.v" -or -name "*.sv")
# VINC_file+=/home/kuuga/ysyx-workbench/ysyxSoC/perip/uart16550/rtl
# VINC_file+=/home/kuuga/ysyx-workbench/ysyxSoC/perip/spi/rtl
VINC_file+=$(sort $(dir $(shell find /home/kuuga/ysyx-workbench/ysyxSoC/perip -name "*.v" -or -name "*.sv")))
# MODULE_SRC+=/home/kuuga/ysyx-workbench/ysyxSoC/build/ysyxSoCFull.v
VINC_file+=/home/kuuga/ysyx-workbench/ysyxSoC/build
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
IMAGE?=$(NPC_HOME)/rot13.image

include $(NPC_HOME)/csrc/utils/filelist.mk

CXXFLAGS+=-DVTOP=V$(TOPNAME) -DVTOP_H=\\\"V$(TOPNAME).h\\\" -DVTOP_DPI_H=\\\"V$(TOPNAME)__Dpi.h\\\" -D__GUEST_ISA__=$(ISA) -DTOPNAME=\\\"$(TOPNAME)\\\" -g 
ifeq ($(findstring  ysyxsoc_sim,$(TOPNAME)), )
	CXXFLAGS+=-DPC_RST=0x80000000
	sepc_module=
else
	CXXFLAGS+=-DPC_RST=0x30000000 -DUSE_NVBOARD
	sepc_module="/home/kuuga/ysyx-workbench/ysyxSoC/perip/psram/efabless/EF_PSRAM_CTRL.v"
	include $(NVBOARD_HOME)/scripts/nvboard.mk
	INC_file+=$(NVBOARD_HOME)/usr/include
	LIBS += $(shell sdl2-config --libs) -lSDL2_image -lSDL2_ttf
endif

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
verilate: $(NPC_HOME)/.stamp.verilate-$(TOPNAME)

.PHONY:run
run:sim $(BUILD_DIR)/$(name)wave.fst

.PHONY:waves
waves: $(BUILD_DIR)/$(name)wave.fst
	@echo
	@echo "### WAVES ###"
	@gtkwave $(BUILD_DIR)/$(name)wave.fst

$(BUILD_DIR)/$(name)wave.fst: $(NPC_HOME)/$(TOPNAME)-build/V$(TOPNAME) $(IMG)
	@echo
	@echo "### SIMULATING ###"
	@echo "$(NPC_HOME)/$(TOPNAME)-build/V$(TOPNAME) $(ARGS) $(IMG) +verilator+rand+reset+2"
	$(NPC_HOME)/$(TOPNAME)-build/V$(TOPNAME) $(ARGS) $(IMG) +verilator+rand+reset+2 +image=$(IMAGE)

$(NPC_HOME)/$(TOPNAME)-build/V$(TOPNAME): $(NPC_HOME)/.stamp.verilate-$(TOPNAME)
	@echo
	@echo "### BUILDING SIM ###"
	@echo "make -C $(NPC_HOME)/$(TOPNAME)-build -f V$(TOPNAME).mk V$(TOPNAME) "
	make -C $(NPC_HOME)/$(TOPNAME)-build -f V$(TOPNAME).mk V$(TOPNAME) 

$(NPC_HOME)/.stamp.verilate-$(TOPNAME): $(MODULE_SRC) $(CSRC) $(HEAD_FILE) $(NVBOARD_ARCHIVE)
	@if [ -d $(NPC_HOME)/$(TOPNAME)-build ]; then echo "build dir have build";else mkdir -p $(NPC_HOME)/$(TOPNAME)-build; fi
	@echo
	@echo "### VERILATING ###"
	@echo verilator --Mdir $(NPC_HOME)/$(TOPNAME)-build --unroll-count 1000 --trace-fst --x-assign unique --x-initial unique --timescale "1ns/1ns" --no-timing --top-module $(TOPNAME)  -cc $(TOPMODULE) --exe $(CSRC) $(NVBOARD_ARCHIVE) $(INC) $(VSRC_DIR) $(MY_CXXFLAGS) $(MY_LDFLAGS) $(sepc_module) -Wno-WIDTHTRUNC -Wno-WIDTHEXPAND
	@verilator --Mdir $(NPC_HOME)/$(TOPNAME)-build --autoflush --unroll-count 1000 --trace-fst --x-assign unique --x-initial unique --timescale "1ns/1ns" --no-timing --top-module $(TOPNAME)  -cc $(TOPMODULE) --exe $(CSRC) $(NVBOARD_ARCHIVE) $(INC) $(VSRC_DIR) $(MY_CXXFLAGS) $(MY_LDFLAGS) $(sepc_module) -Wno-WIDTHTRUNC -Wno-WIDTHEXPAND
	@touch $(NPC_HOME)/.stamp.verilate-$(TOPNAME)

gdb: $(NPC_HOME)/$(TOPNAME)-build/V$(TOPNAME) $(IMG)
	$(call git_commit, "gdb NPC")
	@echo
	@echo "### SIMULATING ###"
	@echo "gdb -s $(NPC_HOME)/$(TOPNAME)-build/V$(TOPNAME) --args $(NPC_HOME)/$(TOPNAME)-build/V$(TOPNAME) $(ARGS) $(IMG) +verilator+rand+reset+2 +image=$(IMAGE)"
	gdb -s $(NPC_HOME)/$(TOPNAME)-build/V$(TOPNAME) --args $(NPC_HOME)/$(TOPNAME)-build/V$(TOPNAME) $(ARGS) $(IMG) +verilator+rand+reset+2 +image=$(IMAGE)

.PHONY:lint
lint: $(MODULE_SRC) 
	verilator --lint-only $(MODULE_SRC) $(VSRC_DIR) --top-module $(TOPNAME) -Wall --timescale "1ns/1ns" --no-timing

SDC_FILE = $(shell find $(NPC_HOME) -name "*.sdc")
CLK_FREQ_MHZ=100
.PHONY:sta
sta:
	@rm -rf $(NPC_HOME)/build/result
	@mkdir -p $(NPC_HOME)/build/result/rtl
	@cp $(MODULE_SRC) $(NPC_HOME)/build/result/rtl
# @make sta DESIGN=$(TOPNAME) SDC_FILE=$(SDC_FILE) RTL_FILES="$(shell find $(abspath $(NPC_HOME)/build/result/rtl) -name "*.v" -or -name "*.sv")" \
# 		CLK_FREQ_MHZ=$(CLK_FREQ_MHZ) CLK_PORT_NAME=clk O=$(NPC_HOME)/build/result -C ~/yosys-sta/ -f Makefile

.PHONY: clean
clean:
	rm -rf .stamp.*;
	rm -rf ./*build
	rm -rf wave.fst
	rm -rf *.txt

include ../Makefile
