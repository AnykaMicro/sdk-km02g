# SPDX-License-Identifier: Apache-2.0
# Copyright (C) 2026 Guangzhou Anyka Microelectronics Co., Ltd.
#
# Repack Anyka's V1.00 Buildroot toolchain for the Release: root-owned, sorted,
# one mtime, and no build-machine path left in any file, link or instruction.
# The tar stream is the same on every run; the gzip bytes also depend on zlib.
#
# Usage: python3 -I scripts/repack_toolchain.py <V1.00 riscv64-anyka-linux-musl.tar.gz> <out.tar.gz>
# It works in memory: nothing from the archive is extracted to disk or run.

import gzip, hashlib, io, posixpath, re, sys, tarfile

IN_SHA256 = "9feed8ce46a44e9a892b50399fed49c6d17e221ebbddfb0f61fefadab21c3fb4"
MTIME = 1776223290  # the v1.00 Release asset's timestamp, 2026-04-15
TOP = "riscv64-anyka-linux-musl"
LOCFILE = TOP + "/share/buildroot/sdk-location"
MOVABS = re.compile(rb"(?=[\x48\x49][\xb8-\xbf](.{8}))", re.S)  # REX.W B8+r imm64


def clean(data, rules, imm):
    binary = b"\0" in data or data.startswith((b"\x7fELF", b"!<arch>"))
    out = data
    for old, new in rules:
        out = out.replace(old, new.ljust(len(old), b"/") if binary else new)
    if binary:
        patched = bytearray(out)
        for m in MOVABS.finditer(out):
            if m.group(1) in imm:
                patched[m.start(1):m.end(1)] = imm[m.group(1)]
        out = bytes(patched)
        assert len(out) == len(data), "binary changed size"
    return out


def repack(src, dst):
    raw = open(src, "rb").read()
    if hashlib.sha256(raw).hexdigest() != IN_SHA256:
        sys.exit(f"{src}: not the V1.00 toolchain (expected SHA-256 {IN_SHA256})")
    tin = tarfile.open(fileobj=io.BytesIO(gzip.decompress(raw)))
    # Where Buildroot installed the SDK: <build dir>/riscv64-anyka-linux-musl. Read
    # from the archive so the build engineer's home path is not written here.
    sdk = tin.extractfile(LOCFILE).read().strip()
    old = sdk[:-len(TOP) - 1]
    # Text files get the new prefix as is (as relocate-sdk.sh does); binaries get it
    # padded with '/' to the old length, so no offset moves.
    rules = [(old + b"/output/build", b"/build"), (old, b"/opt")]
    # The gcc drivers also build sdk on the stack with movabs, 8 bytes at a time.
    # Patch the chunks that hold the build dir; the rest is the target triple.
    pad = b"/opt".ljust(len(old), b"/") + sdk[len(old):]
    imm = {sdk[i:i + 8]: pad[i:i + 8] for i in range(0, len(old), 8)}
    members = {m.name: m for m in tin}
    assert len(members) == len(tin.getmembers()), "duplicate member names"
    names = sorted(members)
    # A hardlink group's data goes with whichever of its names sorts first, so
    # every link points backwards and tar extracts the archive in one pass.
    lead = {}
    for n in names:
        m = members[n]
        lead.setdefault(m.linkname if m.islnk() else n, n)
    with open(dst, "wb") as f, gzip.GzipFile(filename="", mode="wb", fileobj=f, mtime=0) as gz, \
            tarfile.open(fileobj=gz, mode="w", format=tarfile.GNU_FORMAT) as tout:
        for n in names:
            m = members[n]
            key = m.linkname if m.islnk() else n
            if m.islnk():
                m = members[key]
                assert m.isreg(), f"{n}: hardlink to {key}"
            t = tarfile.TarInfo(n)  # uid/gid 0, empty uname/gname
            t.mode, t.mtime = m.mode, MTIME
            if m.isdir():
                t.type = tarfile.DIRTYPE
            elif m.issym():
                t.type, t.linkname = tarfile.SYMTYPE, m.linkname
                if m.linkname.startswith(sdk.decode() + "/"):
                    t.linkname = posixpath.relpath(TOP + m.linkname[len(sdk):], posixpath.dirname(n))
            elif not m.isreg():
                sys.exit(f"{n}: unexpected member type {m.type!r}")
            elif lead[key] != n:
                t.type, t.linkname = tarfile.LNKTYPE, lead[key]
            else:
                data = clean(tin.extractfile(m).read(), rules, imm)
                t.size = len(data)
                tout.addfile(t, io.BytesIO(data))
                continue
            tout.addfile(t)
    parts = old.split(b"/")  # "", "home", user, then the build-dir layout
    check(dst, len(names), sum(m.islnk() for m in members.values()),
          [old, parts[2], b"/".join(parts[3:5])], imm)


def check(path, entries, hardlinks, leaks, imm):
    ms = []
    with tarfile.open(path) as t:
        for m in t:
            ms.append(m)
            assert not any(x.decode() in m.name + m.linkname for x in leaks), m.name
            if m.isreg():
                d = t.extractfile(m).read()
                assert not any(x in d for x in leaks), m.name
                assert not any(x.group(1) in imm for x in MOVABS.finditer(d)), m.name
                if m.name == LOCFILE:
                    assert d == b"/opt/" + TOP.encode() + b"\n", d
    assert len(ms) == entries, len(ms)
    assert sum(m.islnk() for m in ms) == hardlinks, "hardlinks lost"
    assert {(m.uid, m.gid, m.uname, m.gname) for m in ms} == {(0, 0, "", "")}
    assert {m.mtime for m in ms} == {MTIME}
    assert not [m.name for m in ms if m.issym() and m.linkname.startswith("/")]
    assert not [m.name for m in ms if not m.issym() and m.mode & 0o6002]
    print(hashlib.sha256(open(path, "rb").read()).hexdigest(), path)


if __name__ == "__main__":
    if len(sys.argv) != 3:
        sys.exit("usage: python3 -I scripts/repack_toolchain.py <in.tar.gz> <out.tar.gz>")
    repack(*sys.argv[1:])
