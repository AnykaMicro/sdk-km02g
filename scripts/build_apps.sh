#!/bin/bash
# SPDX-License-Identifier: Apache-2.0
# Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd.
#
# Build AI glasses app (ai_glass_demo).
#
# Usage: ./scripts/build_apps.sh all | clean
#   all    Build app and copy to rootfs
#   clean  Remove app build artifacts
#

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
CONFIG_MK="$SCRIPT_DIR/../config.mk"

if [ ! -f "$CONFIG_MK" ]; then
    echo "Error: config.mk not found (expected: $CONFIG_MK)"
    exit 1
fi
. "$CONFIG_MK"

# Use project root and rules.mk path (same as top-level Makefile)
CONFIG_TOP_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
RULES_MK_FILE="${CONFIG_TOP_DIR}/rules.mk"
INSTALL_DIR="$CONFIG_TOP_DIR"
PLATFORM_INC_DIR="$INSTALL_DIR/platform/include"
PLATFORM_LIB_DIR="$INSTALL_DIR/platform/lib"
EXT_LIB_DIR="$INSTALL_DIR/platform/lib"

export RULES_MK_FILE INSTALL_DIR \
       PLATFORM_INC_DIR PLATFORM_LIB_DIR EXT_LIB_DIR

build_ai_glass() {
    echo ">>> Building ai_glass_demo ..."
    local app_dir="$CONFIG_TOP_DIR/apps/ai_glass_demo"
    cd "$app_dir"
    # A failed build must not leave the previous binary for the ramdisk step
    rm -f "$CONFIG_TOP_DIR/rootfs/ai_glass_demo"
    make clean

    if ! make; then
        echo "Error: ai_glass_demo build failed."
        cd - >/dev/null
        exit 1
    fi

    cp ai_glass_demo "$CONFIG_TOP_DIR/rootfs/"
    cp common/config/app_main_config.ini "$CONFIG_TOP_DIR/rootfs/config/"
    cp common/config/camera_gc13b0.ini "$CONFIG_TOP_DIR/rootfs/config/camera.ini"
    cp common/config/eis_gc13b0.ini "$CONFIG_TOP_DIR/rootfs/config/eis_config.ini"

    cd - >/dev/null
}

clean_ai_glass() {
    local app_dir="$CONFIG_TOP_DIR/apps/ai_glass_demo"
    [ -d "$app_dir" ] || return 0
    cd "$app_dir"
    make clean
    cd - >/dev/null
}

case "${1:-}" in
    all)
        build_ai_glass
        ;;
    clean)
        clean_ai_glass
        ;;
    *)
        echo "Usage: $0 all | clean"
        echo "  all   Build app and install to rootfs"
        echo "  clean Remove app build artifacts"
        exit 1
        ;;
esac
