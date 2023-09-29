ifeq ($(wildcard include/am-origin.h),)
ifeq ($(wildcard $(AM_HOME)/am/include/am.h),)
  $(error $$AM_HOME/am/include/amdev.h will be used. Please set $$AM_HOME to an AbstractMachine repo)
else
  $(info Setup link to header files)
  $(shell ln -sf -T $(AM_HOME)/am/include/am.h include/am-origin.h)
  $(shell ln -sf -T $(AM_HOME)/am/include/amdev.h include/amdev.h)
  $(shell ln -sf -T $(AM_HOME)/klib/include/klib.h include/klib.h)
#   $(shell cp $(AM_HOME)/am/src/platform/nemu/ioe/gpu.c   src)
#   $(shell cp $(AM_HOME)/am/src/platform/nemu/ioe/timer.c src)
#   $(shell cp $(AM_HOME)/am/src/platform/nemu/ioe/input.c src)
#   $(shell cp $(AM_HOME)/am/src/platform/nemu/ioe/disk.c  src)
#   $(shell cp $(AM_HOME)/am/src/platform/nemu/ioe/audio.c src)
#   $(shell ln -sf -T $(AM_HOME)/am/src/platfrom/nemu/ioe/ include/klib-macros.h)
endif
endif

NAME = libam
SRCS = $(shell find ./src -name "*.c" -o -name "*.cpp")
LIB_DEP = libndl
include $(NAVY_HOME)/Makefile
