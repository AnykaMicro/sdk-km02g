# SPDX-License-Identifier: Apache-2.0
# Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd.
# Cross compilation toolchain
CROSS_COMPILE = riscv64-anyka-linux-musl-
CC		= $(CROSS_COMPILE)gcc
CXX 	= $(CROSS_COMPILE)g++
AR		= $(CROSS_COMPILE)ar
STRIP	= $(CROSS_COMPILE)strip


# Feature switches. Only the defaults below compile: turning EIS, RTSP or
# recording off does not build yet. SOUND_REC_SUPPORT_ENABLE=1 does build.

# EIS image stabilization switch (default off)
EIS_ENABLE ?= 1

# AI denoise switch (default off)
AI_DENOISE_ENABLE ?= 0

# RTSP real-time video switch (default off)
RTSP_SUPPORT_ENABLE ?= 1
# RTSP early start (default off)
RTSP_START_FIRST_ENABLE ?= 1

# Recording switch (default on):
RECORD_SUPPORT_ENABLE ?= 1

# Audio recording switch (default off)
SOUND_REC_SUPPORT_ENABLE ?= 0


# Static compilation
STATIC_ENABLE ?= 1

ifeq ($(EIS_ENABLE),1)
    CFLAGS += -DEIS_ENABLED
    CXXFLAGS += -DEIS_ENABLED
endif

ifeq ($(AI_DENOISE_ENABLE),1)
    CFLAGS += -DAI_DENOISE_ENABLE
    CXXFLAGS += -DAI_DENOISE_ENABLE
endif

ifeq ($(RTSP_SUPPORT_ENABLE),1)
    CFLAGS += -DRTSP_SUPPORT_ENABLE
    CXXFLAGS += -DRTSP_SUPPORT_ENABLE
endif

ifeq ($(RTSP_START_FIRST_ENABLE),1)
    CFLAGS += -DRTSP_START_FIRST_ENABLE
    CXXFLAGS += -DRTSP_START_FIRST_ENABLE
endif

ifeq ($(RECORD_SUPPORT_ENABLE),1)
    CFLAGS += -DRECORD_SUPPORT_ENABLE
    CXXFLAGS += -DRECORD_SUPPORT_ENABLE
endif

ifeq ($(SOUND_REC_SUPPORT_ENABLE),1)
    CFLAGS += -DSOUND_REC_SUPPORT_ENABLE
    CXXFLAGS += -DSOUND_REC_SUPPORT_ENABLE
endif

    CFLAGS += -DLED_ENABLE
    CXXFLAGS += -DLED_ENABLE

ifeq ($(STATIC_ENABLE),1)
    LIBS += -static
endif
