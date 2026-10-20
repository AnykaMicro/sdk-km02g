# SPDX-License-Identifier: Apache-2.0
# Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd.
# Project name
include common.mk

PROJ = ai_glass_demo



LIBS = $(addprefix -l, $(LIB_DEP))

TARGET = ai_glass_demo

LIB_DEP = plat_common  plat_mem plat_thread pthread  plat_log plat_osal plat_isp_sdk plat_vi plat_vpss mpi_venc plat_dbg mpi_ipu m
LIB_DEP += plat_ai plat_vqe mpi_aenc akmp3 akaac akamr
LIB_DEP += akaudiofilter akaudiocodec
LIB_DEP += mpi_mux akmedia ak_eis
LIB_DEP += rtsp_server app_video
LIB_DEP += mpi_svp ak_nne ak_nna plat_npu ak_object_filter
LIB_DEP += plat_timer app_its

TOPDIR 	?= $(shell pwd)/../..

PLATFORM_INSTALL = $(TOPDIR)/platform/lib

INCLUDE    =	-I. \
		-I$(TOPDIR)/platform/include/ \
		-I./common/inc

#Locations


SRCDIR += .

CSRCS = $(foreach d, $(SRCDIR), $(wildcard $d/*.c))
CSRCS += $(wildcard common/src/*.c)
CPPSRCS = $(wildcard common/src/*.cpp)



LIB_DIR = $(TOPDIR)/platform/lib


ifeq ($(RTSP_SUPPORT_ENABLE),1)
# LiveStream library path
INCLUDE += -IlibLiveStreamCode/
CSRCS += $(wildcard libLiveStreamCode/*.c)
endif

# TakePhoto library path
INCLUDE += -IlibTakePhotoCode/
CSRCS += $(wildcard libTakePhotoCode/*.c)

ifeq ($(RECORD_SUPPORT_ENABLE),1)
# VideoRecord library path
INCLUDE += -IlibVideoRecordCode/
CSRCS += $(wildcard libVideoRecordCode/*.c)
endif

ifeq ($(SOUND_REC_SUPPORT_ENABLE),1)
# AudioRecord library path
INCLUDE += -IlibAudioRecordCode/
CSRCS += $(wildcard libAudioRecordCode/*.c)
endif

# VideoEis library path
ifeq ($(EIS_ENABLE),1)
INCLUDE += -IlibVideoEis/
CSRCS += $(wildcard libVideoEis/*.c)
CPPSRCS += $(wildcard libVideoEis/*.cpp)
endif

LD_FLAGS =  $(addprefix -L,$(LIB_DIR)) -static -lstdc++ -lm


COBJS += $(patsubst %.c, %.o, $(CSRCS))
CPPOBJS += $(patsubst %.c, %.o, $(CPPSRCS))

#build rule
.PHONY: all target clean build-deps

all: $(TARGET)

$(TARGET):$(COBJS) $(CPPOBJS)
	$(CC)  $(COBJS) $(CPPOBJS) -Xlinker "-(" -L$(PLATFORM_INSTALL)/ $(LIBS) -Xlinker "-)" $(LD_FLAGS) -o $(TARGET)
	$(STRIP) $(TARGET)

$(COBJS):%.o:%.c
	$(CC) $(CFLAGS) $(ALLHEADDIR) -c $< -o $@

#$(CPPOBJS):%.o:%.cpp
#	$(CC) $(CFLAGS-CPP) $(ALLHEADDIR) -c $< -o $@

clean :
	$(RM) $(COBJS) $(CPPOBJS) $(TARGET)
	@echo "Clean completed."

ifndef RULES_MK_FILE
RULES_MK_FILE=$(TOPDIR)/rules.mk
endif


# Rules
include $(RULES_MK_FILE)
