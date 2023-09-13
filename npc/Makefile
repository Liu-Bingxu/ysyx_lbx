TOPNAME=top
MODULE = $(shell find $(abspath ./vsrc) -name "*.v" -or -name "*.sv")

all:
	@echo "Write this Makefile by your self."

sim:waveform.vcd
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by your self."

.PHONY:verilate
verilate: .stamp.verilate

.PHONY:build
build: build/Valu

.PHONY:waves
waves: waveform.vcd
	@echo
	@echo "### WAVES ###"
	gtkwave wave.vcd

waveform.vcd: ./build/V$(MODULE)
	@echo
	@echo "### SIMULATING ###"
	@./build/V$(TOPNAME) +verilator+rand+reset+2

./build/V$(MODULE): .stamp.verilate
	@echo
	@echo "### BUILDING SIM ###"
	make -C build -f V$(TOPNAME).mk V$(TOPNAME)

.stamp.verilate: $(MODULE) ./csrc/main.cpp
	@echo
	@echo "### VERILATING ###"
	verilator --Mdir ./build -Wall --trace --x-assign unique --x-initial unique --top-module $(TOPNAME)  -cc $(MODULE) --exe ./csrc/main.cpp
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
