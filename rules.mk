# SPDX-License-Identifier: Apache-2.0
# Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd.

# Cross Tools and libs

CROSS_TOOLCHAIN ?= riscv64-anyka-linux-musl
#CROSS_TOOLCHAIN ?= riscv64-anyka-linux-gnu
CROSS_COMPILE    ?= $(CROSS_TOOLCHAIN)-

# Tools
CC           = $(CROSS_COMPILE)gcc
CXX          = $(CROSS_COMPILE)g++
AS           = $(CROSS_COMPILE)as
AR           = $(CROSS_COMPILE)ar
LD           = $(CROSS_COMPILE)ld
RM           = rm -rf
MKDIR        = mkdir
OBJDUMP      = $(CROSS_COMPILE)objdump
OBJCOPY	     = $(CROSS_COMPILE)objcopy
STRIP        = $(CROSS_COMPILE)strip

CFLAGS += -Werror -D_GNU_SOURCE -std=c99 -mlittle-endian  -fno-builtin -nostdlib -O2 $(DEFINE) $(INCLUDE) $(GLB_INCLUDE)
CFLAGS += -march=rv64imafdc_zicsr_zifencei_zba_zbb -mabi=lp64d -mcmodel=medany

#ASFLAGS += -mlittle-endian -x assembler-with-cpp -O2
#ASFLAGS +=


# Rules

# --------------------------- s -> o
%.o:%.s
	@echo ---------------------[build $<]----------------------------------
	$(CC) -c $(ASFLAGS) $(CFLAGS) -o $@ $<

# ----------------------------- c -> o
%.o:%.c
	@echo ---------------------[build $<]----------------------------------
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $@ $<
