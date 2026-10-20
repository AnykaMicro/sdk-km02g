#!/bin/sh
# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2026 Guangzhou Anyka Microelectronics Co., Ltd.
#
# Download the RISC-V cross toolchain (GCC 12.2 / musl, Linux x86-64 host),
# verify its SHA-256 and unpack it.
#
# Usage: ./scripts/get_toolchain.sh [install-dir]     (default: /opt)
# Run it as your normal user; it asks for sudo only if install-dir isn't writable.
# Set TOOLCHAIN_URL to download from a mirror; the checksum is still enforced.
#

set -e

# TODO(release): placeholder until the GitHub Release exists.
URL="${TOOLCHAIN_URL:-https://github.com/Anyka-Micro/anykavision-ai-sdk/releases/download/v1.00/riscv64-anyka-linux-musl.tar.gz}"
SHA256="9feed8ce46a44e9a892b50399fed49c6d17e221ebbddfb0f61fefadab21c3fb4"
DEST="${1:-/opt}"
FILE="$(cd "$(dirname "$0")/.." && pwd)/tools/riscv64-anyka-linux-musl.tar.gz"

verify() {
    echo "$SHA256  $1" | sha256sum -c --status -
}

if ! command -v sha256sum >/dev/null 2>&1; then
    echo "Error: need sha256sum. On Ubuntu: sudo apt install coreutils" >&2
    exit 1
fi

# Download as the user; use sudo only to unpack into a directory the user can't write
SUDO=
if ! { mkdir -p "$DEST" 2>/dev/null && [ -w "$DEST" ]; }; then
    if ! command -v sudo >/dev/null 2>&1; then
        echo "Error: $DEST is not writable and sudo is not available. Pass a writable install-dir." >&2
        exit 1
    fi
    echo "$DEST is not writable; using sudo to unpack"
    SUDO=sudo
fi

# Never leave a partial or rejected download behind, even when interrupted
trap 'rm -f "$FILE.part"' EXIT
trap 'exit 1' INT TERM HUP

if [ -f "$FILE" ] && verify "$FILE"; then
    echo "Using existing $FILE"
else
    echo "Downloading $URL"
    if command -v curl >/dev/null 2>&1; then
        curl -fL -o "$FILE.part" "$URL"
    elif command -v wget >/dev/null 2>&1; then
        wget -O "$FILE.part" "$URL"
    else
        echo "Error: need curl or wget. On Ubuntu: sudo apt install wget" >&2
        exit 1
    fi
    if ! verify "$FILE.part"; then
        echo "Error: SHA-256 mismatch for $URL (expected $SHA256)" >&2
        exit 1
    fi
    mv "$FILE.part" "$FILE"
fi

$SUDO mkdir -p "$DEST"
# The archive's owner is a build-host account; never recreate it
$SUDO tar --no-same-owner -xzf "$FILE" -C "$DEST"
echo "Toolchain installed. Add it to PATH:"
echo "  export PATH=\$PATH:$DEST/riscv64-anyka-linux-musl/bin"
