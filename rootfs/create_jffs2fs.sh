#!/bin/sh
# SPDX-License-Identifier: Apache-2.0
# Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd.
#
# Build JFFS2 image (config partition) using mtd-utils mkfs.jffs2.
# Packs rootfs/config, with the files of rootfs/config.local/ laid over it,
# into usr.jffs2 under tools/ota/. Requires mkfs.jffs2 (mtd-utils) in PATH.
#
# rootfs/config.local/ is ignored by git. Keep your own shadow (root password
# hash) and app_main_config.ini (debug AP passphrase) there, never in config/.
#
# root must have a password. Set ALLOW_LOCKED_ROOT=1 to build an image with
# root locked instead: it has no console login, and an OTA update with it
# also locks root on devices already in use.
#
# Parameters (do not change unless required):
#   -d  Directory to pack
#   -s  Page size, fixed 256
#   -e  Erase block size 4096. mtd-utils 2.x raises anything below 8 KiB to
#       8 KiB ("Erase size 0x1000 too small"): check on the board that the
#       kernel mounts the image cleanly (see README)
#   -m  Compression none (for AK37 performance/memory, no compression)
#   -U  Every file owned by root
#   -o  Output file name
#
# Mount example: mount -t jffs2 /dev/mtdblock1 /mnt
# Config is stored as JFFS2 on SPI partition and mounted at etc for persistence.
#

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

if [ ! -d "config" ]; then
    echo "Error: config directory not found. Run from rootfs."
    exit 1
fi

if ! command -v mkfs.jffs2 >/dev/null 2>&1; then
    echo "Error: mkfs.jffs2 not found in PATH. On Ubuntu: sudo apt install mtd-utils" >&2
    exit 1
fi

STAGE="$(mktemp -d)"
trap 'rm -rf "$STAGE"' EXIT
cp -R config/. "$STAGE/"
if [ -d config.local ]; then
    cp -R config.local/. "$STAGE/"
    echo "Using your files from rootfs/config.local/: $(ls config.local | tr '\n' ' ')"
fi

if grep -q '^root::' "$STAGE/shadow"; then
    echo "Error: root has an empty password, so anyone on the serial console gets root." >&2
    echo "Put a shadow file with a password hash in rootfs/config.local/ (see README)." >&2
    exit 1
fi
if grep -q '^root:[*!]' "$STAGE/shadow"; then
    if [ "${ALLOW_LOCKED_ROOT:-0}" != 1 ]; then
        echo "Error: root has no password, so the device would have no console login." >&2
        echo "Put a shadow file with a password hash in rootfs/config.local/ (see README)," >&2
        echo "or build with ALLOW_LOCKED_ROOT=1 to accept a locked root." >&2
        exit 1
    fi
    echo "Note: ALLOW_LOCKED_ROOT=1, so root login is disabled in this image." >&2
fi

# /etc/shadow on the device comes from here; only root may read it
chmod 600 "$STAGE/shadow"

mkfs.jffs2 -d "$STAGE" -s 256 -e 4096 -m none -U -o ../tools/ota/usr.jffs2
echo "Generated: $(cd .. && pwd)/tools/ota/usr.jffs2"
