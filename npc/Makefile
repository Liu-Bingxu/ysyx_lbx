TOPNAME=top
MODULE = $(shell find $(abspath ./vsrc) -name "*.v" -or -name "*.sv")
CSRC = $(shell find $(abspath ./csrc) -name "*.c" -or -name "*.cpp" -or -name "*.cc")
INC_file=$(sort $(dir $(shell find $(abspath ./) -name "*.h")))
LIBS+=-lreadline

override ARGS ?= --log=$(BUILD_DIR)/npc-log.txt
override ARGS+=

CONFIG_ITRACE=true

CFLAGS_add=$(addprefix -CFLAGS ,$(1))
CFLAGS_inc_add=$(addprefix -CFLAGS -I,$(1))
CFLAGS_lib_add=$(addprefix -CFLAGS -l,$(1))
LD_add=$(addprefix -LDFLAGS ,$(1))

BUILD_DIR?=.

include $(NPC_HOME)/csrc/utils/filelist.mk

INC=$(call CFLAGS_inc_add,$(INC_file))
MY_CXXFLAGS=$(call CFLAGS_add,$(CXXFLAGS))
MY_LDFLAGS=$(call LD_add,$(LIBS))

all:
	@echo "Write this Makefile by your self."

sim:$(BUILD_DIR)/wave.vcd
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
# @echo "Write this Makefile by your self."

.PHONY:verilate
verilate: $(BUILD_DIR)/.stamp.verilate

.PHONY:run
run:sim $(BUILD_DIR)/wave.vcd

.PHONY:waves
waves: $(BUILD_DIR)/wave.vcd
	@echo
	@echo "### WAVES ###"
	gtkwave $(BUILD_DIR)/wave.vcd

$(BUILD_DIR)/wave.vcd: $(BUILD_DIR)/build/V$(TOPNAME) $(IMG)
	@echo
	@echo "### SIMULATING ###"
	@$(BUILD_DIR)/build/V$(TOPNAME) $(ARGS) $(IMG) +verilator+rand+reset+2

$(BUILD_DIR)/build/V$(TOPNAME): $(BUILD_DIR)/.stamp.verilate
	@echo
	@echo "### BUILDING SIM ###"
	make -C $(BUILD_DIR)/build -f V$(TOPNAME).mk V$(TOPNAME) 

$(BUILD_DIR)/.stamp.verilate: $(MODULE) $(CSRC)
	@echo
	@echo "### VERILATING ###"
	@echo verilator --Mdir $(BUILD_DIR)/build -Wall --trace --x-assign unique --x-initial unique --top-module $(TOPNAME)  -cc $(MODULE) --exe $(CSRC) $(INC) -CFLAGS -D__GUEST_ISA__=$(ISA) -Ivsrc $(MY_CXXFLAGS) $(MY_LDFLAGS)
	verilator --Mdir $(BUILD_DIR)/build -Wall --trace --x-assign unique --x-initial unique --top-module $(TOPNAME)  -cc $(MODULE) --exe $(CSRC) $(INC) -CFLAGS -D__GUEST_ISA__=$(ISA) -Ivsrc $(MY_CXXFLAGS) $(MY_LDFLAGS)
	@touch $(BUILD_DIR)/.stamp.verilate

.PHONY:lint
lint: $(MODULE).v
	verilator --lint-only $(MODULE).v

.PHONY: clean
clean:
	rm -rf .stamp.*;
	rm -rf ./build
	rm -rf wave.vcd
	rm -rf *.txt

include ../Makefile
