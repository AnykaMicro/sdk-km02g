#!/bin/bash
# SPDX-License-Identifier: Apache-2.0
# Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd.
#
# Build the RAM rootfs image tools/ota/ramfs.img from this repository only:
#   rootfs/skeleton/          init and OTA scripts and /etc files
#   rootfs/build/userland/    BusyBox and mtd_debug, built from source by build_userland.sh
#   rootfs/ai_glass_demo      the app, put here by scripts/build_apps.sh (make app)
# Invoked by scripts/build_rootfs.sh.
#
# Usage: ./create_ramfs.sh all | clean
#   all    Build the userland if needed, assemble the tree and pack it
#   clean  Remove tools/ota/ramfs.img
#

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
# Project root: parent of rootfs
CONFIG_TOP="$(cd "$SCRIPT_DIR/.." && pwd)"

# Name in the image header. The on-device updater may read it, so it keeps the
# V1.00 value until that is checked on a board; override it with RAMFS_NAME.
RAMFS_NAME="${RAMFS_NAME:-AnyCloud39AV200 ramfs}"

# Directories of the ramdisk; usr is where the device mounts its APP partition
DIRS="bin sbin etc/init.d etc/config etc/sysconfig var/run data fast lib usr sys proc mnt dev tmp"

# Pack the ramfs dir and produce ramfs.img
make_image() {
    # Files on the device must be owned by root, not the build user, and only
    # the owner may write them (sticky directories such as a 1777 /tmp keep their mode)
    find ./ramfs ! -type l ! -perm -1000 -exec chmod go-w {} +
    (cd ./ramfs && find . | LC_ALL=C sort | cpio -o -H newc -R 0:0 > ../ramfs.cpio)
    lz4 -l -3 -f ramfs.cpio ramfs.cpio.lz4
    mkimage -A riscv -T ramdisk -C lz4 -O linux -a 0x0 -e 0x0 \
        -n "$RAMFS_NAME" -d ./ramfs.cpio.lz4 "$CONFIG_TOP/tools/ota/ramfs.img"
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
        if [ ! -f ai_glass_demo ]; then
            echo "Error: rootfs/ai_glass_demo not found. Build the app first: make app" >&2
            exit 1
        fi
        ./build_userland.sh
        # Start from a clean tree; an aborted run can leave one behind
        rm -rf ./ramfs ramfs.cpio ramfs.cpio.lz4
        mkdir ramfs
        (cd ramfs && mkdir -p $DIRS)
        cp -a skeleton/. ramfs/
        cp -a build/userland/. ramfs/
        cp ai_glass_demo ramfs/etc/ai_glass_demo
        chmod 755 ramfs/etc/ai_glass_demo
        make_image
        rm -rf ramfs
        ;;
    clean)
        clean_image
        ;;
    *)
        echo "Usage: $0 all | clean"
        echo "  all   Build the userland if needed, assemble the tree and pack it"
        echo "  clean Remove tools/ota/ramfs.img"
        exit 1
        ;;
esac
