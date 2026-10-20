# Third-party notices

Unless a file says otherwise, everything in this repository, including the prebuilt libraries in `platform/lib`, is Copyright (C) Guangzhou Anyka Microelectronics Co., Ltd. and licensed under the [Apache License 2.0](LICENSE).

This file lists the third-party software that the SDK contains or distributes, and the licence obligations that come with it.

## 1. Inside the prebuilt libraries (`platform/lib`)

| Component | Found in | Licence |
|---|---|---|
| Speex / SpeexDSP (codec, echo canceller) | `libakaudiofilter`, `libakmedia`, `libmpi_aenc`, `libmpi_adec` | BSD-3-Clause |
| Kiss FFT (as bundled with SpeexDSP) | `libakaudiofilter` | BSD-3-Clause |
| cJSON (symbols renamed `ak_cJSON*`) | `libplat_dbg` | MIT |

Both licences require their copyright notice and licence text to accompany binary redistributions.

### Speex / SpeexDSP

The text below is the `COPYING` file of [xiph/speexdsp](https://github.com/xiph/speexdsp) (identical in xiph/speex), taken from the upstream repository on 2026-10-17; the latest release is SpeexDSP 1.2.1.

> **TODO (Anyka):** confirm which Speex/SpeexDSP release the libraries were built from, and replace this text if that release's `COPYING` differs.

```text
Copyright 2002-2008 	Xiph.org Foundation
Copyright 2002-2008 	Jean-Marc Valin
Copyright 2005-2007	Analog Devices Inc.
Copyright 2005-2008	Commonwealth Scientific and Industrial Research
                        Organisation (CSIRO)
Copyright 1993, 2002, 2006 David Rowe
Copyright 2003 		EpicGames
Copyright 1992-1994	Jutta Degener, Carsten Bormann

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

- Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

- Neither the name of the Xiph.org Foundation nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```

### Kiss FFT (as bundled in SpeexDSP, `libspeexdsp/kiss_fft.c`)

```text
Copyright (c) 2003-2004, Mark Borgerding
Copyright (c) 2005-2007, Jean-Marc Valin

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the author nor the names of any contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
```

### cJSON

```text
Copyright (c) 2009-2017 Dave Gamble and cJSON contributors

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
```

## 2. GitHub Release assets (not in git)

### Cross toolchain: `riscv64-anyka-linux-musl.tar.gz`

A Buildroot-generated SDK for Linux x86-64 hosts. Main components:

| Component | Licence |
|---|---|
| GCC 12.2.0 | GPL-3.0-or-later; runtime libraries under the GCC Runtime Library Exception |
| GNU Binutils 2.39 | GPL-3.0-or-later |
| musl libc 1.2.3 | MIT |
| Linux 5.10 UAPI headers | GPL-2.0 WITH Linux-syscall-note |
| GMP, MPFR | LGPL-3.0-or-later |
| Target libraries in the sysroot, e.g. ncurses, zlib, LZO, libnl, libevent, MIT Kerberos, libtirpc, expat, acl, attr | various (GPL-2.0-or-later, LGPL, BSD, MIT and others) |

The complete source code and licence texts for this archive must be published next to it in the same Release, generated with Buildroot's `make legal-info`. Do not publish the archive without them.

> **TODO (Anyka):** publish the `legal-info` output (sources, licence texts and manifest) as a Release asset before, or together with, the toolchain.

### Base ramdisk: `ramfs.img`

The ramdisk that `rootfs/create_ramfs.sh` unpacks and repacks with your build of `ai_glass_demo`. The Release asset must itself be built from this repository (`make`, then `tools/ota/ramfs.img`). Never publish the `rootfs/ramfs.img` of the V1.00 bundle: its prebuilt `ai_glass_demo` predates the cleaned sources and the rewritten USB webcam module.

Every executable in the image is statically linked:

| Component | Licence |
|---|---|
| BusyBox 1.35.0 (`bin/busybox` and its links) | GPL-2.0-only |
| `bin/mtd_debug` from mtd-utils | GPL-2.0-or-later |
| `etc/format_tool`: SD/NAND formatter, an executable that runs an embedded shell script | Anyka. **TODO (Anyka):** confirm its origin |
| Init and helper scripts: `etc/init.d/rcS`, `etc/init.d/rc.local`, `etc/profile`, `sbin/*.sh` | Apache-2.0 (Anyka) |
| `etc/ai_glass_demo`, built from this repository | Apache-2.0 (Anyka) |
| musl libc 1.2.3, linked into `ai_glass_demo`, `format_tool` and `mtd_debug` | MIT (notice below) |
| GCC runtime (libgcc, libstdc++), linked into `ai_glass_demo` | GPL-3.0-or-later WITH GCC-exception-3.1 |

musl libc (from its `COPYRIGHT` file, release 1.2.3; the full file, which also lists contributors and files under other permissive licences, belongs in the source bundle):

```text
Copyright © 2005-2020 Rich Felker, et al.

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
```

The BusyBox 1.35.0 source, the exact BusyBox `.config` and the mtd-utils source must be published with the `ramfs.img` Release asset. Do not publish the asset without them.

> **TODO (Anyka):** add those sources and the `.config` to the Release.

## 3. Host tools you install yourself

The build calls these from your `PATH`; the SDK does not ship them: `lz4` (BSD-2-Clause library, GPL-2.0 command-line tool), `mkfs.jffs2` (mtd-utils, GPL-2.0-or-later), `mkimage` (u-boot-tools, GPL-2.0-or-later), `cpio` and `xxd`.

## 4. Removed from the V1.00 bundle

- Prebuilt host binaries `rootfs/lz4`, `rootfs/mkfs.jffs2` and `tools/uboot_tools/mkimage` (GPL). Install the distribution packages instead (see README).
- `platform/include_3rd/iconv.h` (GNU libiconv, LGPL-2.0-or-later). Nothing used it.
- `list.h` (both copies) and the USB webcam code `libTakePhotoCode/uvc.c` and `uvc.h`. They are replaced by `ak_list.h` and `libTakePhotoCode/usb_webcam.c`/`.h`, written from scratch for this repository.
- Build outputs (`tools/ota/update.bin`, `ramfs.img`, `usr.jffs2`) and the 116 MB toolchain archive, which are now built locally or published as Release assets.
