TOPNAME=top
MODULE_SRC = $(shell find $(abspath ./vsrc) -name "*.v" -or -name "*.sv")
TOPMODULE = $(shell find $(abspath ./vsrc) -name "top.v")
CSRC = $(shell find $(abspath ./csrc) -name "*.c" -or -name "*.cpp" -or -name "*.cc")
HEAD_FILE=$(shell find $(abspath ./) -name "*.h")
INC_file=$(sort $(dir $(shell find $(abspath ./) -name "*.h")))
LIBS+=-lreadline

override ARGS ?= --log=$(BUILD_DIR)/npc-log.txt --wave=$(BUILD_DIR)/$(name)wave.vcd
override ARGS += -d $(NEMU_HOME)/build/riscv32-nemu-interpreter-so

CONFIG_ITRACE=true

CFLAGS_add=$(addprefix -CFLAGS ,$(1))
CFLAGS_inc_add=$(addprefix -CFLAGS -I,$(1))
CFLAGS_lib_add=$(addprefix -CFLAGS -l,$(1))
LD_add=$(addprefix -LDFLAGS ,$(1))
module_add=$(addprefix -v ,$(1))

BUILD_DIR?=./build
name?=

include $(NPC_HOME)/csrc/utils/filelist.mk

MODULE=$(call module_add,$(MODULE_SRC))
INC=$(call CFLAGS_inc_add,$(INC_file))
MY_CXXFLAGS=$(call CFLAGS_add,$(CXXFLAGS))
MY_LDFLAGS=$(call LD_add,$(LIBS))

all:run
	@echo "Write this Makefile by your self."

sim:$(BUILD_DIR)/$(name)wave.vcd
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
# @echo "Write this Makefile by your self."

.PHONY:verilate
verilate: $(BUILD_DIR)/.stamp.verilate

.PHONY:run
run:sim $(BUILD_DIR)/$(name)wave.vcd

.PHONY:waves
waves: $(BUILD_DIR)/$(name)wave.vcd
	@echo
	@echo "### WAVES ###"
	@gtkwave $(BUILD_DIR)/$(name)wave.vcd

$(BUILD_DIR)/$(name)wave.vcd: $(BUILD_DIR)/build/V$(TOPNAME) $(IMG)
	@echo
	@echo "### SIMULATING ###"
	@$(BUILD_DIR)/build/V$(TOPNAME) $(ARGS) $(IMG) +verilator+rand+reset+2

$(BUILD_DIR)/build/V$(TOPNAME): $(BUILD_DIR)/.stamp.verilate
	@echo
	@echo "### BUILDING SIM ###"
	make -C $(BUILD_DIR)/build -f V$(TOPNAME).mk V$(TOPNAME) 

$(BUILD_DIR)/.stamp.verilate: $(MODULE_SRC) $(CSRC) $(HEAD_FILE)
	@if [ -d $(BUILD_DIR)/build ]; then echo "build dir have build";else mkdir -p $(BUILD_DIR)/build; fi
	@echo
	@echo "### VERILATING ###"
	@echo verilator --Mdir $(BUILD_DIR)/build -Wall --unroll-count 1000 --trace --x-assign unique --x-initial unique --timing --top-module $(TOPNAME)  -cc $(TOPMODULE) --exe $(CSRC) $(INC) -CFLAGS -D__GUEST_ISA__=$(ISA) -Ivsrc $(MY_CXXFLAGS) $(MY_LDFLAGS)
	verilator --Mdir $(BUILD_DIR)/build -Wall --unroll-count 1000 --trace --x-assign unique --x-initial unique --timing --top-module $(TOPNAME)  -cc $(TOPMODULE) --exe $(CSRC) $(INC) -CFLAGS -D__GUEST_ISA__=$(ISA) -Ivsrc $(MY_CXXFLAGS) $(MY_LDFLAGS)
	@touch $(BUILD_DIR)/.stamp.verilate

.PHONY:lint
lint: $(MODULE_SRC) 
	verilator --lint-only $(MODULE_SRC) -Ivsrc --top-module $(TOPNAME)

.PHONY: clean
clean:
	rm -rf .stamp.*;
	rm -rf ./build
	rm -rf wave.vcd
	rm -rf *.txt

include ../Makefile
