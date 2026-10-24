#!/bin/sh
# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2026 Guangzhou Anyka Microelectronics Co., Ltd.
#
# Build the ramdisk's userland from upstream source, statically:
#   BusyBox 1.35.0 with rootfs/busybox-1.35.0.config
#   mtd_debug from mtd-utils 2.3.1
# Output: rootfs/build/userland/ (bin/, sbin/, linuxrc), rebuilt only when
# the config, a source archive or this script changes.
#
# The source archives are downloaded once into tools/ and checked against the
# SHA-256 below. To build offline, put them there yourself. SOURCES_URL
# replaces both download sites with a mirror; the checksums still apply.
#
# Usage: ./build_userland.sh
#

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
TOP="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD="$SCRIPT_DIR/build"
OUT="$BUILD/userland"
HOST=riscv64-anyka-linux-musl
JOBS="$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)"

BUSYBOX=busybox-1.35.0
BUSYBOX_SHA256=faeeb244c35a348a334f4a59e44626ee870fb07b6884d68c10ae8bc19f83a694
BUSYBOX_URL=https://busybox.net/downloads
MTD=mtd-utils-2.3.1
MTD_SHA256=03d9dc58ad10ea3549d9528f6b17a44d8944e18e96c0f31474f9f977078b83dc
MTD_URL=https://infraroot.at/pub/mtd

for tool in "$HOST-gcc" "$HOST-strip"; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "Error: '$tool' not found in PATH. Install the toolchain (scripts/get_toolchain.sh) and add it to PATH." >&2
        exit 1
    fi
done
for tool in gcc make bzip2 tar sha256sum; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "Error: '$tool' not found in PATH. On Ubuntu: sudo apt install gcc make bzip2" >&2
        exit 1
    fi
done

# Never leave a partial or rejected download behind
trap 'rm -f "$TOP/tools/$BUSYBOX.tar.bz2.part" "$TOP/tools/$MTD.tar.bz2.part"' EXIT
trap 'exit 1' INT TERM HUP

verify() {
    echo "$2  $1" | sha256sum -c --status -
}

fetch() {   # fetch <archive> <sha256> <download site>
    f="$TOP/tools/$1"
    if [ -f "$f" ] && verify "$f" "$2"; then
        return 0
    fi
    url="${SOURCES_URL:-$3}/$1"
    echo "Downloading $url"
    if command -v curl >/dev/null 2>&1; then
        curl -fL -o "$f.part" "$url"
    elif command -v wget >/dev/null 2>&1; then
        wget -O "$f.part" "$url"
    else
        echo "Error: need curl or wget. On Ubuntu: sudo apt install wget" >&2
        exit 1
    fi
    if ! verify "$f.part" "$2"; then
        echo "Error: SHA-256 mismatch for $url (expected $2)" >&2
        exit 1
    fi
    mv "$f.part" "$f"
}

want="$(cat "$SCRIPT_DIR/busybox-1.35.0.config" "$SCRIPT_DIR/build_userland.sh" | sha256sum | cut -c1-64)"
if [ -f "$BUILD/userland.stamp" ] && [ "$(cat "$BUILD/userland.stamp")" = "$want" ] &&
   [ -x "$OUT/bin/busybox" ] && [ -x "$OUT/bin/mtd_debug" ]; then
    echo "Userland up to date: $OUT"
    exit 0
fi

fetch "$BUSYBOX.tar.bz2" "$BUSYBOX_SHA256" "$BUSYBOX_URL"
fetch "$MTD.tar.bz2" "$MTD_SHA256" "$MTD_URL"

rm -rf "$BUILD/src" "$OUT" "$BUILD/userland.stamp"
mkdir -p "$BUILD/src" "$OUT"

# BusyBox: the config sets the cross prefix, a static build and the applet links
tar -xjf "$TOP/tools/$BUSYBOX.tar.bz2" -C "$BUILD/src"
cp "$SCRIPT_DIR/busybox-1.35.0.config" "$BUILD/src/$BUSYBOX/.config"
yes "" | make -C "$BUILD/src/$BUSYBOX" oldconfig >/dev/null
make -C "$BUILD/src/$BUSYBOX" -j"$JOBS"
make -C "$BUILD/src/$BUSYBOX" CONFIG_PREFIX="$OUT" install >/dev/null

# mtd_debug only: every optional dependency of mtd-utils is switched off
tar -xjf "$TOP/tools/$MTD.tar.bz2" -C "$BUILD/src"
(
    cd "$BUILD/src/$MTD"
    ./configure --host="$HOST" --disable-tests --disable-unit-tests --without-jffs --without-ubifs \
        --without-xattr --without-lzo --without-zstd --without-crypto --without-selinux LDFLAGS=-static >/dev/null
    make mtd_debug
)
"$HOST-strip" -o "$OUT/bin/mtd_debug" "$BUILD/src/$MTD/mtd_debug"

echo "$want" > "$BUILD/userland.stamp"
echo "Userland built: $OUT"
