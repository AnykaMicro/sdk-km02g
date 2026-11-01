# Release review (grill checklist)

Every change set and every release must survive these questions. Review a **commit**, not a working copy, and write down its hash.

`scripts/review.sh` answers every question marked **auto**:

```bash
./scripts/review.sh source    # on the checked-out commit; CI runs this on every push
make && ./scripts/review.sh build    # also inspects tools/ota/ after a build
```

It prints `PASS`, `FAIL` or `SKIP` per ID and exits 1 if anything failed. Questions marked **manual** need a person; **board** needs a KM02G board. A question is only settled when its evidence (a check output, a link or a test log) is recorded with the commit it applies to.

## Security

| ID | Question | How |
|---|---|---|
| R1.1 | Is any password hash committed? Real ones belong in the git-ignored `rootfs/config.local/` | auto |
| R1.2 | Do the Wi-Fi files hold only placeholders? | auto |
| R1.3 | Is there an internal lab address (172.16.0.0/12) anywhere? | auto |
| R1.4 | Is a default AP passphrase hard-coded? | auto |
| R1.5 | Can any log line print a password? | auto |
| M1 | Can anything from outside (IPI/BLE command, phone value, SD or FTP file name, config file) reach a shell, a config file or a fixed-size buffer unchecked? Read every `system()`, `popen()`, `fprintf` into a config file and string copy that the change touches | manual |
| M2 | Does the build refuse an image whose root has no password, unless `ALLOW_LOCKED_ROOT=1` is given? | manual (`rootfs/create_jffs2fs.sh`) |

## Clean-room USB webcam

| ID | Question | How |
|---|---|---|
| R2.1 | Are the GPL `uvc.c`/`uvc.h` or the kernel `list.h` back? | auto |
| R2.2 | Do uvc-gadget identifiers appear in any source? | auto |
| R7.1 | Do uvc-gadget strings appear in any executable we ship? | auto (build) |
| M3 | Was the change to `usb_webcam.c` written without reading the GPL originals? | manual |

## Names, links and checksums

| ID | Question | How |
|---|---|---|
| R3.1 | Does anything still use the old repository name? | auto |
| R3.2 | Do the English and Chinese READMEs show the toolchain SHA-256 that `scripts/get_toolchain.sh` enforces? | auto |
| R3.3 | Is any other, stale toolchain checksum left in the README? | auto |
| R3.4 | Does the download URL point at `Anyka-Micro/sdk-km02g`? | auto |
| M4 | Do the docs (anykavision-ai-docs) describe the same build steps, file names and versions as this README? | manual |

## Scope

| ID | Question | How |
|---|---|---|
| R4.1 | Is an APK, AAB or `.upd` firmware file committed? The phone app is not part of this release | auto |
| R4.2 | Is the V1.00 base ramdisk or a bundled GPL host tool back? | auto |
| R4.3 | Is a downloaded source archive or the toolchain committed? | auto |

## Hygiene

| ID | Question | How |
|---|---|---|
| R5.1 | Does every script parse (`bash -n`, and `dash -n` for `#!/bin/sh`)? | auto |
| R5.2 | Are there whitespace errors? | auto |
| R5.3 | Does every C file, header, script and Makefile carry an SPDX line? | auto |
| M5 | Do the printf-family formats match their arguments? The build's `-fno-builtin` hides this from gcc | CI `format` job |
| M6 | Do `make test` and a full `make` pass in a clean Ubuntu container with the published toolchain? | manual |

## Images (after `make`)

| ID | Question | How |
|---|---|---|
| R6.1 | Is every ramdisk entry owned by root? | auto (build) |
| R6.2 | Is anything group- or world-writable? | auto (build) |
| R6.3 | Are BusyBox, `mtd_debug`, the app and the init and OTA scripts in the image? | auto (build) |
| R6.4 | Does the image hold exactly `rootfs/skeleton/` plus the built userland and app, and nothing else? | auto (build) |
| R6.5 | Is the header name the one `RAMFS_NAME` sets? | auto (build) |
| R7.2 | Is a lab address in a shipped executable? | auto (build) |
| R7.3 | Does the app still build an OTA command line for a shell? | auto (build) |
| R8.1 | Is every config-partition file owned by root? | auto (build) |
| R8.2 | Is `/etc/shadow` readable by root only? | auto (build) |

## Licensing and Release files

| ID | Question | How |
|---|---|---|
| M7 | Does `THIRD_PARTY_NOTICES.md` list every component of every shipped binary and Release file, with the licence text where the licence requires it? | manual |
| M8 | Is every GPL/LGPL binary on the Release accompanied by its source? (toolchain: Buildroot `legal-info`; ramdisk: BusyBox and mtd-utils archives plus the BusyBox config) | manual |
| M9 | Do the Release files match the checksums in this repository? | manual |
| M10 | Was the Release toolchain made from the V1.00 archive by `scripts/repack_toolchain.py`, and does a second run give the same file? Its self-check fails on any build-machine path left | manual |

## Needs a board

These stay open until someone runs them on a KM02G board and records the result. Building and unit tests do not prove them.

| ID | Question |
|---|---|
| H1 | Does the flashed image boot to a console login with the root password from `config.local`? |
| H2 | Does an OTA update (`update.bin`) apply, and does the on-device updater accept the ramdisk header name? |
| H3 | Does the config partition (8 KiB JFFS2 erase blocks) mount without errors? |
| H4 | Does the USB webcam stream to Linux, Windows and macOS hosts? |
| H5 | Do Wi-Fi STA, AP and P2P modes, and RTSP live view, work? |
