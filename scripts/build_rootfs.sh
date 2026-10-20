#!/bin/bash
# SPDX-License-Identifier: Apache-2.0
# Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd.
#
# Build rootfs images: ramfs + jffs2 (config).
#
# Usage: ./scripts/build_rootfs.sh all | clean
#   all    Build ramfs.img and usr.jffs2 under tools/ota
#   clean  Print hint only (actual cleanup is via top-level make clean)
#

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
CONFIG_MK="$SCRIPT_DIR/../config.mk"
ROOTFS_DIR="$SCRIPT_DIR/../rootfs"

if [ ! -f "$CONFIG_MK" ]; then
    echo "Error: config.mk not found."
    exit 1
fi
. "$CONFIG_MK"

ROOTFS_DIR="$(cd "$SCRIPT_DIR/../rootfs" && pwd)"

make_image() {
    cd "$ROOTFS_DIR"
    ./create_ramfs.sh all
    ./create_jffs2fs.sh
    cd - >/dev/null
}

clean_image() {
    echo "Use top-level 'make clean' to clean rootfs artifacts."
}

case "${1:-}" in
    all)
        make_image
        ;;
    clean)
        clean_image
        ;;
    *)
        echo "Usage: $0 all | clean"
        echo "  all   Build ramfs and jffs2 images"
        echo "  clean Hint to use make clean"
        exit 1
        ;;
esac
