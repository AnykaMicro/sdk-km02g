#!/bin/bash
# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2026 Guangzhou Anyka Microelectronics Co., Ltd.
#
# Automatic reviewer: runs every check in REVIEW.md that a script can decide
# and prints PASS, FAIL or SKIP per check ID. Exit status 1 if anything failed.
#
# Usage: ./scripts/review.sh source   checks on the checked-out files (CI runs this)
#        ./scripts/review.sh build    source checks, then the images in tools/ota/ (after make)
#

set -u

TOP="$(cd "$(dirname "$0")/.." && pwd)"
cd "$TOP"
MODE="${1:-}"
case "$MODE" in source|build) ;; *) echo "Usage: $0 source | build" >&2; exit 2 ;; esac

FAILED=0
pass() { echo "PASS  $1"; }
fail() { echo "FAIL  $1"; FAILED=1; }
skip() { echo "SKIP  $1"; }
check() {   # check <id: text> <command...>: PASS when the command succeeds
    local name=$1; shift
    if "$@"; then pass "$name"; else fail "$name"; fi
}
# Tracked text files, or every file when this is not a git checkout
files() {
    if git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
        git ls-files -z | xargs -0 grep -IlZ '' 2>/dev/null | tr '\0' '\n'
    else
        find . -path ./.git -prune -o -path ./rootfs/build -prune -o -type f -print | sed 's#^\./##' |
            while read -r f; do grep -Iq '' "$f" 2>/dev/null && echo "$f"; done
    fi
}
none() {    # none <ERE> [path filter ERE to skip]: no tracked text file matches
    local hits
    hits=$(files | grep -Ev "${2:-^$}" | tr '\n' '\0' | xargs -0 grep -nIE "$1" 2>/dev/null)
    [ -z "$hits" ] || { echo "$hits" | head -5 | sed 's/^/      /'; return 1; }
}

echo "== Source checks ($(git rev-parse --short HEAD 2>/dev/null || echo 'no git'))"

# R1 Secrets: no password hash, real Wi-Fi credential or private lab address in the tree
check "R1.1 no crypt password hash in tracked files" none '\$(1|5|6|y)\$[^:$ ]{2,}\$'
check "R1.2 Wi-Fi files hold placeholders only" none '^[[:space:]]*(psk|wpa_passphrase|password)="?[^"[:space:]Y]' '^(README|REVIEW)\.md$'
check "R1.3 no 172.16.0.0/12 lab address" none '\b172\.(1[6-9]|2[0-9]|3[01])\.[0-9]+\.[0-9]+\b' '^REVIEW\.md$|^scripts/review\.sh$'
check "R1.4 no hard-coded default AP passphrase" none '"12345678"' '^REVIEW\.md$|^scripts/review\.sh$'
check "R1.5 log calls never print a password" none '\b(log_[a-z_]+|printf|PRINT)[[:space:]]*\(.*(pass:|password[^_]|psk)[^)]*%s'

# R2 Clean-room USB webcam: nothing derived from the GPL uvc-gadget code
check "R2.1 no uvc.c, uvc.h or kernel list.h" bash -c '! git ls-files 2>/dev/null | grep -qE "(^|/)(uvc\.[ch]|list\.h)$"'
check "R2.2 no uvc-gadget identifiers" none 'uvc_events_process|uvc_video_qbuf|uvc_video_process|UVC_DEV\]|Ideas on board' '^REVIEW\.md$|^scripts/review\.sh$'

# R3 Names and links: one repository name, one toolchain checksum everywhere
check "R3.1 no reference to the old repository name" none 'Anyka-Micro/anykavision-ai-sdk|anykavision-ai-sdk/releases' '^REVIEW\.md$|^scripts/review\.sh$'
sha=$(sed -n 's/^SHA256="\(.*\)"/\1/p' scripts/get_toolchain.sh)
check "R3.2 README (EN and ZH) shows the toolchain SHA-256 of get_toolchain.sh" bash -c "[ \"\$(grep -c '$sha' README.md)\" = 2 ]"
check "R3.3 no other 64-hex toolchain checksum in README" bash -c "! grep -oE 'SHA-256 \`[0-9a-f]{64}\`' README.md | grep -v '$sha' | grep -q ."
check "R3.4 the download URL names Anyka-Micro/sdk-km02g" grep -q 'github.com/Anyka-Micro/sdk-km02g/releases/download/' scripts/get_toolchain.sh

# R4 Scope: only what this repository should ship
check "R4.1 no APK or phone-app binary" bash -c '! git ls-files 2>/dev/null | grep -qiE "\.(apk|aab|upd)$"'
check "R4.2 no V1.00 base ramdisk or bundled host tools" bash -c '! git ls-files 2>/dev/null | grep -qE "^rootfs/(ramfs\.img|lz4|mkfs\.jffs2)$|^tools/uboot_tools/"'
check "R4.3 no source archive or toolchain committed" bash -c '! git ls-files 2>/dev/null | grep -qE "^tools/.*\.(tar\.(gz|bz2|xz)|part)$"'

# R5 Hygiene: scripts parse, whitespace is clean, Anyka sources carry SPDX
r5_shell() {
    local f ok=0
    for f in $(files | grep -E '\.sh$'); do
        bash -n "$f" || ok=1
        if head -n1 "$f" | grep -q '^#!/bin/sh' && command -v dash >/dev/null 2>&1; then dash -n "$f" || ok=1; fi
    done
    return $ok
}
check "R5.1 every script passes bash -n (and dash -n for /bin/sh)" r5_shell
if git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    check "R5.2 no whitespace errors (git diff --check)" git diff --check 4b825dc642cb6eb9a060e54bf8d69288fbee4904 --
else
    skip "R5.2 no whitespace errors (not a git checkout)"
fi
r5_spdx() {
    local missing
    missing=$(files | grep -E '\.(c|h|sh|mk)$|(^|/)Makefile$' | grep -v '^rootfs/skeleton/' |
        while read -r f; do grep -q 'SPDX-License-Identifier' "$f" || echo "$f"; done)
    [ -z "$missing" ] || { echo "$missing" | head -5 | sed 's/^/      /'; return 1; }
}
check "R5.3 every C, header, script and Makefile has an SPDX line" r5_spdx

[ "$MODE" = build ] || { [ $FAILED = 0 ] && echo "All source checks passed." || echo "Source checks FAILED."; exit $FAILED; }

echo "== Build checks (tools/ota/)"
IMG=tools/ota/ramfs.img
if [ ! -f "$IMG" ]; then
    fail "B0 tools/ota/ramfs.img exists (run make first)"
    exit 1
fi
for tool in lz4 cpio dd; do command -v $tool >/dev/null 2>&1 || { fail "B0 host tool $tool"; exit 1; }; done
W="$(mktemp -d)"; trap 'rm -rf "$W"' EXIT
dd if="$IMG" bs=64 skip=1 of="$W/r.lz4" status=none && lz4 -dqf "$W/r.lz4" "$W/r.cpio" && mkdir "$W/x" &&
    (cd "$W/x" && cpio -idm --quiet < ../r.cpio 2>/dev/null)
LIST=$(cpio -itv < "$W/r.cpio" 2>/dev/null)

# R6 Image contents
check "R6.1 every ramdisk entry is owned by root" bash -c '[ -z "$(echo "$1" | awk "\$3!=\"root\" || \$4!=\"root\"")" ]' _ "$LIST"
check "R6.2 nothing group- or world-writable (except sticky dirs)" bash -c '[ -z "$(echo "$1" | awk "substr(\$1,1,1)!=\"l\" && substr(\$1,10,1)!=\"t\" && (substr(\$1,6,1)==\"w\" || substr(\$1,9,1)==\"w\")")" ]' _ "$LIST"
check "R6.3 BusyBox, mtd_debug, the app and the init/OTA scripts are present" bash -c 'cd "$1" && [ -x bin/busybox ] && [ -x bin/mtd_debug ] && [ -x etc/ai_glass_demo ] && [ -x etc/init.d/rcS ] && [ -x sbin/update.sh ] && [ "$(readlink linuxrc)" = bin/busybox ]' _ "$W/x"
check "R6.4 the image holds exactly the files of rootfs/skeleton plus the userland and app" bash -c '
    a=$(cd "$1/x" && find . ! -type d | sort | grep -vE "^\./(bin|sbin)/|^\./linuxrc$|^\./etc/ai_glass_demo$")
    b=$(cd rootfs/skeleton && find . ! -type d | sort | grep -v "^\./sbin/")
    [ "$a" = "$b" ]' _ "$W"
check "R6.5 the header name is set by RAMFS_NAME in create_ramfs.sh" bash -c '
    n=$(dd if="$1" bs=1 skip=32 count=32 status=none | tr -d "\0"); grep -q "RAMFS_NAME:-$n}" rootfs/create_ramfs.sh' _ "$IMG"

# R7 Shipped binaries: the same secret and clean-room checks, on the built executables
r7() {   # r7 <ERE>: no ELF in the image has a matching string
    local f
    for f in $(find "$W/x" -type f); do
        head -c4 "$f" | grep -q ELF || continue
        if tr -c '[:print:]' '\n' < "$f" | grep -qE "$1"; then echo "      ${f#$W/x/}"; return 1; fi
    done
}
check "R7.1 no uvc-gadget strings in any executable" r7 'uvc_video_qbuf|UVC_DEV\]|Ideas on board|uvc_events_process'
check "R7.2 no 172.16.0.0/12 lab address in any executable" r7 '\b172\.(1[6-9]|2[0-9]|3[01])\.[0-9]+\.[0-9]+\b'
check "R7.3 no shell-built OTA command in the app" r7 'ota_flasher +-f %s'

# R8 Config partition
J=tools/ota/usr.jffs2
if [ ! -f "$J" ]; then
    fail "R8 tools/ota/usr.jffs2 exists"
elif ! command -v jffs2reader >/dev/null 2>&1; then
    skip "R8 config image checks (jffs2reader from mtd-utils not installed)"
else
    # jffs2reader does not list the first directory entry, so check what it does list
    J_LIST=$(jffs2reader "$J" -d / 2>/dev/null)
    check "R8.1 every config file is owned by root" bash -c '[ -z "$(echo "$1" | awk "\$3!=0 || \$4!=0")" ]' _ "$J_LIST"
    check "R8.2 /etc/shadow is readable by root only" bash -c 'echo "$1" | grep -qE "^-rw------- .* /shadow *$"' _ "$J_LIST"
fi

[ $FAILED = 0 ] && echo "All checks passed." || echo "Checks FAILED."
exit $FAILED
