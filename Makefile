# SPDX-License-Identifier: Apache-2.0
# Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd.
# Makefile for AnyCloud39AV200 SDK

# Include configuration
include config.mk

# Default target
.PHONY: all app install clean test
# all = clean, app, install in that order; never run them in parallel
.NOTPARALLEL:

all:  clean app install

app:
	echo "========================================";
	echo "Building Apps...";
	echo "========================================";
	+./scripts/build_apps.sh all;

# Install Images
install:
	@echo "========================================"
	@echo "Installing Images..."
	@echo "========================================"
	@# A failed step must never leave older images to be packaged
	@rm -f tools/ota/update.bin tools/ota/ramfs.img tools/ota/usr.jffs2 tools/ota/part1.bin
	# Build file system
	@./scripts/build_rootfs.sh all
	@# Run signing script

	# ota package
	@echo "ota"
	@pwd
	@cd tools/ota && ./ota_ramfs_only.sh
	@echo "Images generated in tools/ota"

# Clean
clean:
	@echo "========================================"
	@echo "Cleaning apps object files..."
	@echo "========================================"
	@find apps -name "*.o" -type f -delete || echo "No .o files found or cleanup failed"
	@rm -rf apps/ai_glass_demo/ai_glass_demo
	@rm -rf rootfs/ai_glass_demo rootfs/ramfs rootfs/ramfs.cpio rootfs/ramfs.cpio.lz4
	@rm -rf tools/ota/update.bin
	@rm -rf tools/ota/ramfs.img
	@rm -rf tools/ota/usr.jffs2 tools/ota/part1.bin
	@rm -f tests/list_test tests/uvc_test
	@echo "Clean complete."

# Host-side unit tests (needs gcc and linux-libc-dev on the build host)
test:
	+$(MAKE) -C tests
