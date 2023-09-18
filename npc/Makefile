TOPNAME=top
MODULE = $(shell find $(abspath ./vsrc) -name "*.v" -or -name "*.sv")
CSRC = $(shell find $(abspath ./csrc) -name "*.c" -or -name "*.cpp" -or -name "cc")
INC_file=$(shell find $(abspath ./) -name "*.h")
INC=$(addprefix -CFLAGS -I,$(sort $(dir $(INC_file))))

BUILD_DIR=$(addsuffix /build,$(shell pwd))

override ARGS ?= --log=$(BUILD_DIR)/npc-log.txt
override ARGS+=

all:
	@echo "Write this Makefile by your self."

sim:waveform.vcd
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by your self."

.PHONY:verilate
verilate: .stamp.verilate

.PHONY:build
build: build/Valu

.PHONY:run
run:waves

.PHONY:waves
waves: wave.vcd
	@echo
	@echo "### WAVES ###"
	gtkwave wave.vcd

wave.vcd: ./build/V$(TOPNAME)
	@echo
	@echo "### SIMULATING ###"
	@./build/V$(TOPNAME) $(ARGS) $(IMG) +verilator+rand+reset+2

./build/V$(TOPNAME): .stamp.verilate
	@echo
	@echo "### BUILDING SIM ###"
	make -C build -f V$(TOPNAME).mk V$(TOPNAME) 

.stamp.verilate: $(MODULE) $(CSRC)
	@echo $(INC)
	@echo
	@echo "### VERILATING ###"
	verilator --Mdir ./build -Wall --trace --x-assign unique --x-initial unique --top-module $(TOPNAME)  -cc $(MODULE) --exe $(CSRC) $(INC)
	@touch .stamp.verilate

.PHONY:lint
lint: $(MODULE).v
	verilator --lint-only $(MODULE).v

.PHONY: clean
clean:
	rm -rf .stamp.*;
	rm -rf ./build
	rm -rf wave.vcd

include ../Makefile
