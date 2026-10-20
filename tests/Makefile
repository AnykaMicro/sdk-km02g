# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2026 Guangzhou Anyka Microelectronics Co., Ltd.
#
# Host-side checks of ak_list.h and the UVC request handling in usb_webcam.c.
# Run on the Linux build host (needs gcc and the kernel UAPI headers from
# linux-libc-dev):  make -C tests

APP     = ../apps/ai_glass_demo
HOSTCC ?= cc
TESTS   = list_test uvc_test
FLAGS   = -std=gnu99 -Wall -Wextra -g -fsanitize=address,undefined

all: $(TESTS)
	@for t in $(TESTS); do ./$$t || exit 1; done

list_test: list_test.c $(APP)/common/inc/ak_list.h
	$(HOSTCC) $(FLAGS) -I$(APP)/common/inc -o $@ $<

uvc_test: uvc_test.c $(APP)/libTakePhotoCode/usb_webcam.c $(APP)/libTakePhotoCode/usb_webcam.h
	$(HOSTCC) $(FLAGS) -I$(APP)/libTakePhotoCode -o $@ $<

clean:
	rm -f $(TESTS)

.PHONY: all clean
