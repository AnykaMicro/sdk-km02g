#!/bin/bash
# SPDX-License-Identifier: Apache-2.0
# Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd.
#
# Build RAM rootfs image (ramfs.img).
# Unpacks ramfs.img in rootfs, merges ai_glass_demo, repacks; output to tools/ota/ramfs.img.
# Must be run from rootfs directory (invoked by build_rootfs.sh).
#
# Usage: ./create_ramfs.sh all | clean
#   all    Unpack ramfs.img -> merge app -> repack
#   clean  Remove tools/ota/ramfs.img
#

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
# Project root: parent of rootfs
CONFIG_TOP="$(cd "$SCRIPT_DIR/.." && pwd)"

# Unpack ramfs image into given directory
# Args: image file, output dir (under current dir)
extract_image() {
    local img_file="$1"
    local output_dir="$2"
    mkdir -p "$output_dir"
    dd if="$img_file" bs=64 skip=1 of=ramfs.cpio.lz4 status=none
    lz4 -d -f ramfs.cpio.lz4 ramfs.cpio
    (cd "$output_dir" && cpio -idmv < ../ramfs.cpio)
    rm -f ramfs.cpio ramfs.cpio.lz4
    echo "Extracted to: $output_dir"
}

# Pack ramfs dir and produce ramfs.img
make_image() {
    # Files on the device must be owned by root, not the build user, and only
    # the owner may write them (sticky directories such as a 1777 /tmp keep their mode)
    find ./ramfs ! -type l ! -perm -1000 -exec chmod go-w {} +
    (cd ./ramfs && find . | cpio -o -H newc -R 0:0 > ../ramfs.cpio)
    lz4 -l -3 -f ramfs.cpio ramfs.cpio.lz4
    mkimage -A riscv -T ramdisk -C lz4 -O linux -a 0x0 -e 0x0 \
        -n "AnyCloud39AV200 ramfs" -d ./ramfs.cpio.lz4 "$CONFIG_TOP/tools/ota/ramfs.img"
    rm -f ramfs.cpio ramfs.cpio.lz4
}

clean_image() {
    echo "Cleaning ramfs image..."
    rm -f "$CONFIG_TOP/tools/ota/ramfs.img"
}

# Current dir must be rootfs when running
cd "$SCRIPT_DIR"

case "${1:-}" in
    all)
        # Host tools come from PATH
        for tool in lz4 mkimage cpio; do
            if ! command -v "$tool" >/dev/null 2>&1; then
                echo "Error: '$tool' not found in PATH. On Ubuntu: sudo apt install lz4 u-boot-tools cpio" >&2
                exit 1
            fi
        done
        if [ ! -f ramfs.img ]; then
            echo "Error: rootfs/ramfs.img not found. Download the base ramdisk from the GitHub Release (see README)." >&2
            exit 1
        fi
        # Never fall back to the prebuilt demo inside the base ramdisk
        if [ ! -f ai_glass_demo ]; then
            echo "Error: rootfs/ai_glass_demo not found. Build the app first: make app" >&2
            exit 1
        fi
        # Start from a clean tree; an aborted run can leave one behind
        rm -rf ./ramfs ramfs.cpio ramfs.cpio.lz4
        extract_image ramfs.img ./ramfs
        cp ./ai_glass_demo ./ramfs/etc/ai_glass_demo
        make_image
        rm -rf ramfs
        ;;
    clean)
        clean_image
        ;;
    *)
        echo "Usage: $0 all | clean"
        echo "  all   Unpack, merge app and repack ramfs"
        echo "  clean Remove tools/ota/ramfs.img"
        exit 1
        ;;
esac
