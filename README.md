# AnykaVision AI SDK

English | [中文](#中文)

Software development kit for the **AnykaVision AI** camera-glasses reference design:

- **KM02G**: 64-bit dual-core RISC-V image SoC (ISP, NPU, video encoder, stabilisation). It runs Linux and the demo application in this repository.
- **AK1090**: Bluetooth audio SoC that links the glasses to the phone app. Its firmware is not in this repository.

| Path | Contents |
|---|---|
| `apps/ai_glass_demo/` | Demo application: commands from the AK1090, photo, video recording, RTSP live view, Wi-Fi, USB webcam (UVC), stabilisation (EIS) |
| `platform/include/`, `platform/lib/` | Middleware API headers and prebuilt RISC-V libraries (`.a` and `.so`), listed [below](#prebuilt-libraries) |
| `rootfs/` | The ramdisk's own files (`skeleton/`) and BusyBox config, the config partition (`config/`), and the scripts that build both images |
| `scripts/` | Build scripts and the toolchain download |
| `tools/ota/` | Builds the `update.bin` OTA package |
| `tests/` | Host-side unit tests (`make test`) |

Documentation is in the separate [anykavision-ai-docs](https://github.com/Anyka-Micro/anykavision-ai-docs) repository: in Chinese, with the English translation in progress.

**Status:** the build and the host-side tests pass. Nothing has been verified on a board yet: flashing, OTA updates, the USB webcam and Wi-Fi are built but not hardware-tested.

## Host requirements

An x86-64 Linux host, Ubuntu 18.04 or later. The toolchain runs only there; on macOS or Windows use a Linux VM or container. Clone into a path without spaces: the Makefiles cannot handle them.

```bash
sudo apt install make gcc bzip2 file lz4 mtd-utils u-boot-tools cpio xxd wget
```

`gcc` and `bzip2` build BusyBox and mtd-utils from source, and `file` lets the toolchain point its paths at its install directory. On Ubuntu 18.04 and 20.04, install `liblz4-tool` instead of `lz4` and `vim-common` instead of `xxd`. The build needs a `mkimage` that supports `-A riscv`, so very old u-boot-tools releases may not work. `make test` also needs `linux-libc-dev`.

## Build

1. **Install the toolchain.** This downloads it from the Release, checks its SHA-256 and unpacks it to `/opt`. Run it as your normal user; it uses `sudo` only for the unpack if `/opt` is not writable:

   ```bash
   ./scripts/get_toolchain.sh
   export PATH=$PATH:/opt/riscv64-anyka-linux-musl/bin
   riscv64-anyka-linux-musl-gcc --version
   ```

   Pass another directory as the first argument to install elsewhere; the script relocates the toolchain there. Set `TOOLCHAIN_URL` to use a mirror (the checksum is still enforced). If you already have the archive, for example from the draft Release while the repository is private (`gh release download v1.00 --repo Anyka-Micro/sdk-km02g`), put it in `tools/` first: the script checks its SHA-256 and skips the download.

2. **Nothing to download for the ramdisk.** `make` builds it from this repository: the files in `rootfs/skeleton/`, BusyBox 1.35.0 and `mtd_debug` (mtd-utils 2.3.1) built from source, and your `ai_glass_demo`. The first build downloads the two source archives into `tools/` and checks their SHA-256. To build offline, put them there yourself, or set `SOURCES_URL` to a mirror.

3. **Set a root password.** The shipped `rootfs/config/shadow` locks root, so the build stops until you set a password or accept a locked root. Keep your password in `rootfs/config.local/`: its files replace those of `rootfs/config/` in the config image, and git ignores the folder, so the hash is never committed.

   ```bash
   mkdir -p rootfs/config.local
   cp rootfs/config/shadow rootfs/config.local/
   openssl passwd -6
   ```

   Put the hash that `openssl` prints in the second field of the `root` line of `rootfs/config.local/shadow`. To build an image with no console login instead, run `ALLOW_LOCKED_ROOT=1 make`. Be careful: `update.bin` also carries the config partition, so an OTA update built that way locks root on devices already in use. On the device, `/etc/shadow` is on the writable config partition, so `passwd` changes it permanently.

4. **Optional: set the debug Wi-Fi password.** The image-debug and USB photo modes start a Wi-Fi access point only if it has a WPA2 passphrase (8 to 63 characters). Copy `apps/ai_glass_demo/common/config/app_main_config.ini` to `rootfs/config.local/` and set `password=` (and optionally `ssid=`) under `[DebugAp]`. Don't put a comment after the value: everything after `=` is part of it.

   The other Wi-Fi files in `rootfs/config/` hold placeholders only. The app writes `hostapd.conf` and `wpa_supplicant.conf` at run time. The device's P2P scripts read `wpa_p2p_supplicant.conf`, whose network block is disabled; put your own copy in `rootfs/config.local/` if you need one.

5. **Build.**

   ```bash
   make
   ```

   `make` cleans, builds `ai_glass_demo`, builds the ramdisk and the config image. The results are in `tools/ota/`: `update.bin` (OTA package), `ramfs.img` and `usr.jffs2`. `make app` builds only the application; `make install` packs the images and needs a built application. `make test` runs the host-side tests. BusyBox and `mtd_debug` are built once and kept in `rootfs/build/`; `make distclean` removes them.

   - Only the default feature switches in `apps/ai_glass_demo/common.mk` compile; turning EIS, RTSP or recording off does not build yet.
   - mtd-utils 2.x builds `usr.jffs2` with 8 KiB erase blocks (it refuses the 4 KiB that the old bundled tool used). After flashing, check on your board that the config partition mounts without JFFS2 errors.

## Release assets

Large or GPL-licensed binaries are published on the GitHub Releases page, not in git:

| Asset | Contents |
|---|---|
| `riscv64-anyka-linux-musl.tar.gz` | GCC 12.2 / musl cross toolchain for x86-64 Linux hosts, fetched by `scripts/get_toolchain.sh`. Repacked for publishing: root-owned, with no build-machine paths. SHA-256 `631f7c929fef2455eb01f1333b5fee83cfaff162c93c44a5a31c604e1997f65d` |
| `ramfs.img` | The ramdisk that `make` builds from this repository at the release commit: BusyBox 1.35.0, `mtd_debug`, the init and OTA scripts in `rootfs/skeleton/`, `etc/format_tool` (an Anyka tool kept unchanged from V1.00) and `ai_glass_demo`. You don't need it to build; it is there for flashing and comparison. Never the image of the original V1.00 bundle |
| Source archives | `busybox-1.35.0.tar.bz2` and `mtd-utils-2.3.1.tar.bz2` with their upstream signatures, and the BusyBox config used. The toolchain's own sources are still to be added |

## USB webcam mode

`libTakePhotoCode/usb_webcam.c` streams the photo JPEGs as MJPEG through the Linux UVC gadget function. It has not been tested on hardware yet.

The gadget is set up by `/usr/sbin/usb_udc_g.sh` and `/usr/sbin/uvc.sh`. They come from the device's APP partition (the squashfs mounted at `/usr`) in the factory firmware: neither this repository nor `update.bin`, which carries only the RAMFS and CONFIG partitions, provides them. Their descriptors must match what the code expects:

- exactly one MJPEG format with one frame size, equal to `[Photo] main_res_width` × `main_res_height` in `camera.ini` (4032 × 3024 by default);
- a frame interval for `[Photo] main_chn_fps` (10 fps = 1000000);
- isochronous streaming at high speed with a 512-byte `wMaxPacketSize` (`UVC_STREAMING_MAXPKT` in `video.c`). At that rate a 4032 × 3024 JPEG takes about 1.5 s;
- `bmControls` 0 for the camera terminal and the processing unit, whose controls are not implemented.

## Prebuilt libraries

`platform/lib` holds each library as a static `.a` and a shared `.so`. Their APIs are in `platform/include`.

| Libraries | Purpose |
|---|---|
| `libplat_common`, `libplat_osal`, `libplat_mem`, `libplat_thread`, `libplat_timer`, `libplat_log`, `libplat_dbg`, `libplat_uuid` | Platform base: OS abstraction, memory, threads, timers, logging, debug, device ID |
| `libplat_vi`, `libplat_vpss`, `libplat_isp_sdk` | Video input, video processing and ISP |
| `libmpi_venc`, `libmpi_osd`, `libmpi_ipu` | Video and JPEG encoding, on-screen display, image processing unit (scaling, DMA) |
| `libmpi_mux`, `libmpi_demux`, `libakmedia` | Recording containers and media files |
| `libplat_ai`, `libplat_ao`, `libplat_vqe` | Audio input and output, voice-quality enhancement |
| `libmpi_aenc`, `libmpi_adec`, `libakaudiocodec`, `libakaac`, `libakamr`, `libakmp3`, `libakg722`, `libakresample` | Audio encoding, decoding and resampling |
| `libakaudiofilter`, `libakcrydetect`, `libaksoundleveldetect`, `libaksoundsourcelocalization`, `libaklinearrayssl` | Audio effects and analysis: filters, cry detection, sound level, sound-source localisation |
| `libplat_npu`, `libak_nna`, `libak_nne`, `libmpi_svp`, `libak_object_filter` | Neural-network accelerator and engine, smart vision (object classification), detection-box filtering |
| `libak_eis`, `libak_mrd`, `libak_vibe` | Electronic image stabilisation, motion-region detection, background modelling |
| `librtsp_server`, `libapp_video` | RTSP server, video helpers |
| `libapp_its`, `libapp_ats` | ISP and audio tuning-tool servers |

## Licence

Apache License 2.0, see [LICENSE](LICENSE) and [NOTICE](NOTICE). This covers the source code, scripts, configuration and the prebuilt libraries in `platform/lib`. Third-party components keep their own licences; see [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md). To report a security problem, see [SECURITY.md](SECURITY.md).

---

## 中文

**AnykaVision AI** AI 拍摄眼镜参考设计的软件开发包（SDK）：

- **KM02G**：64 位双核 RISC-V 图像 SoC（ISP、NPU、视频编码、防抖），运行 Linux 和本仓库的示例应用。
- **AK1090**：蓝牙音频 SoC，负责眼镜与手机 App 的连接。其固件不在本仓库中。

| 路径 | 内容 |
|---|---|
| `apps/ai_glass_demo/` | 示例应用：处理来自 AK1090 的命令、拍照、录像、RTSP 实时预览、Wi-Fi、USB 摄像头（UVC）、防抖（EIS） |
| `platform/include/`、`platform/lib/` | 中间件 API 头文件和预编译的 RISC-V 库（`.a` 和 `.so`），列表见[下文](#预编译库) |
| `rootfs/` | ramdisk 自身的文件（`skeleton/`）和 BusyBox 配置、配置分区（`config/`），以及生成这两个镜像的脚本 |
| `scripts/` | 编译脚本和工具链下载脚本 |
| `tools/ota/` | 生成 `update.bin` OTA 升级包 |
| `tests/` | 主机端单元测试（`make test`） |

文档位于独立的 [anykavision-ai-docs](https://github.com/Anyka-Micro/anykavision-ai-docs) 仓库：中文版，英文翻译正在进行中。

**状态：** 编译和主机端测试均已通过。尚未在开发板上验证任何功能：烧写、OTA 升级、USB 摄像头和 Wi-Fi 都已编译，但未经硬件测试。

## 编译主机要求

x86-64 Linux 主机，Ubuntu 18.04 或更新版本。工具链只能在这类主机上运行；macOS 或 Windows 用户请使用 Linux 虚拟机或容器。请克隆到不含空格的路径：Makefile 无法处理路径中的空格。

```bash
sudo apt install make gcc bzip2 file lz4 mtd-utils u-boot-tools cpio xxd wget
```

`gcc` 和 `bzip2` 用于从源码编译 BusyBox 和 mtd-utils，`file` 用于让工具链把路径指向其安装目录。在 Ubuntu 18.04 和 20.04 上，请用 `liblz4-tool` 代替 `lz4`，用 `vim-common` 代替 `xxd`。编译需要支持 `-A riscv` 的 `mkimage`，过旧的 u-boot-tools 版本可能无法使用。`make test` 还需要 `linux-libc-dev`。

## 编译步骤

1. **安装工具链。** 以下脚本会从 Release 下载工具链，校验 SHA-256，并解压到 `/opt`。请以普通用户运行；仅当 `/opt` 不可写时，脚本才会用 `sudo` 解压：

   ```bash
   ./scripts/get_toolchain.sh
   export PATH=$PATH:/opt/riscv64-anyka-linux-musl/bin
   riscv64-anyka-linux-musl-gcc --version
   ```

   如需安装到其他目录，可将目录作为第一个参数传入，脚本会把工具链重定位到该目录。设置 `TOOLCHAIN_URL` 可使用镜像地址（仍会强制校验 SHA-256）。如果你已有该压缩包（例如在仓库仍为私有时从草稿 Release 下载：`gh release download v1.00 --repo Anyka-Micro/sdk-km02g`），请先把它放到 `tools/`：脚本会校验其 SHA-256 并跳过下载。

2. **ramdisk 无需下载。** `make` 会用本仓库的内容生成它：`rootfs/skeleton/` 中的文件、从源码编译的 BusyBox 1.35.0 和 `mtd_debug`（mtd-utils 2.3.1），以及你编译的 `ai_glass_demo`。首次编译时会把这两个源码包下载到 `tools/` 并校验 SHA-256。如需离线编译，请自行把它们放到该目录，或设置 `SOURCES_URL` 指向镜像地址。

3. **设置 root 密码。** 发布的 `rootfs/config/shadow` 锁定了 root，因此在设置密码或明确接受锁定之前，编译会停止。请把密码放在 `rootfs/config.local/` 中：该目录中的文件会在配置镜像里替换 `rootfs/config/` 中的同名文件，而且 git 会忽略该目录，所以密码哈希不会被提交。

   ```bash
   mkdir -p rootfs/config.local
   cp rootfs/config/shadow rootfs/config.local/
   openssl passwd -6
   ```

   把 `openssl` 输出的哈希填入 `rootfs/config.local/shadow` 中 `root` 行的第二个字段。如果要生成不能从控制台登录的镜像，请运行 `ALLOW_LOCKED_ROOT=1 make`。注意：`update.bin` 也包含配置分区，用这种方式生成的 OTA 升级包会把已在使用的设备的 root 也锁定。设备上的 `/etc/shadow` 位于可写的配置分区，因此用 `passwd` 修改后会永久生效。

4. **可选：设置调试 Wi-Fi 密码。** 只有设置了 WPA2 密码（8 到 63 个字符），图像调试模式和 USB 拍照模式才会开启 Wi-Fi 热点。请把 `apps/ai_glass_demo/common/config/app_main_config.ini` 复制到 `rootfs/config.local/`，并在 `[DebugAp]` 中设置 `password=`（也可设置 `ssid=`）。不要在值后面加注释：`=` 之后的全部内容都属于值。

   `rootfs/config/` 中的其他 Wi-Fi 文件只包含占位值。应用在运行时写入 `hostapd.conf` 和 `wpa_supplicant.conf`。`wpa_p2p_supplicant.conf` 由设备的 P2P 脚本读取，其中的网络配置块已禁用；如有需要，请在 `rootfs/config.local/` 中放入你自己的版本。

5. **编译。**

   ```bash
   make
   ```

   `make` 会先清理，然后编译 `ai_glass_demo`，生成 ramdisk 和配置镜像。结果位于 `tools/ota/`：`update.bin`（OTA 升级包）、`ramfs.img` 和 `usr.jffs2`。`make app` 只编译应用；`make install` 打包镜像，需要先编译好应用。`make test` 运行主机端测试。BusyBox 和 `mtd_debug` 只编译一次并保存在 `rootfs/build/`；`make distclean` 会删除它们。

   - 只有 `apps/ai_glass_demo/common.mk` 中默认的功能开关组合可以编译；关闭 EIS、RTSP 或录像功能目前无法编译。
   - mtd-utils 2.x 生成的 `usr.jffs2` 使用 8 KiB 擦除块（它不接受旧版内置工具使用的 4 KiB）。烧写后，请在开发板上确认配置分区挂载时没有 JFFS2 错误。

## Release 附件

体积较大或采用 GPL 许可的二进制文件发布在 GitHub Releases 页面，不放入 git：

| 附件 | 内容 |
|---|---|
| `riscv64-anyka-linux-musl.tar.gz` | 适用于 x86-64 Linux 主机的 GCC 12.2 / musl 交叉工具链，由 `scripts/get_toolchain.sh` 下载。已为发布重新打包：文件归 root 所有，不含编译机器的路径。SHA-256 `631f7c929fef2455eb01f1333b5fee83cfaff162c93c44a5a31c604e1997f65d` |
| `ramfs.img` | `make` 在发布对应的 commit 上用本仓库生成的 ramdisk：BusyBox 1.35.0、`mtd_debug`、`rootfs/skeleton/` 中的启动和 OTA 脚本、`etc/format_tool`（保持 V1.00 原样的安凯工具）以及 `ai_glass_demo`。编译时不需要它，它用于烧写和对照。绝不能使用原 V1.00 包中的镜像 |
| 源码压缩包 | `busybox-1.35.0.tar.bz2` 和 `mtd-utils-2.3.1.tar.bz2`（附上游签名），以及所用的 BusyBox 配置。工具链本身的源码尚待补充 |

## USB 摄像头模式

`libTakePhotoCode/usb_webcam.c` 通过 Linux UVC gadget 功能，将拍照得到的 JPEG 以 MJPEG 视频流输出。该功能尚未在硬件上测试。

gadget 由 `/usr/sbin/usb_udc_g.sh` 和 `/usr/sbin/uvc.sh` 配置。这两个脚本来自出厂固件中设备的 APP 分区（挂载在 `/usr` 的 squashfs）：本仓库和 `update.bin`（只包含 RAMFS 和 CONFIG 分区）都不提供它们。其描述符必须与代码的要求一致：

- 必须且只能声明一种 MJPEG 格式和一种帧尺寸，尺寸等于 `camera.ini` 中的 `[Photo] main_res_width` × `main_res_height`（默认 4032 × 3024）；
- 帧间隔对应 `[Photo] main_chn_fps`（10 fps = 1000000）；
- 高速（high speed）等时传输，`wMaxPacketSize` 为 512 字节（`video.c` 中的 `UVC_STREAMING_MAXPKT`）。按此速率，一张 4032 × 3024 的 JPEG 约需 1.5 秒；
- 摄像头终端（camera terminal）和处理单元（processing unit）的 `bmControls` 设为 0，因为这些控制尚未实现。

## 预编译库

`platform/lib` 中每个库都有静态库 `.a` 和动态库 `.so` 两种形式，其 API 位于 `platform/include`。

| 库 | 用途 |
|---|---|
| `libplat_common`、`libplat_osal`、`libplat_mem`、`libplat_thread`、`libplat_timer`、`libplat_log`、`libplat_dbg`、`libplat_uuid` | 平台基础：操作系统抽象、内存、线程、定时器、日志、调试、设备 ID |
| `libplat_vi`、`libplat_vpss`、`libplat_isp_sdk` | 视频输入、视频处理和 ISP |
| `libmpi_venc`、`libmpi_osd`、`libmpi_ipu` | 视频和 JPEG 编码、屏幕叠加（OSD）、图像处理单元（缩放、DMA） |
| `libmpi_mux`、`libmpi_demux`、`libakmedia` | 录像封装格式和媒体文件 |
| `libplat_ai`、`libplat_ao`、`libplat_vqe` | 音频输入和输出、语音质量增强 |
| `libmpi_aenc`、`libmpi_adec`、`libakaudiocodec`、`libakaac`、`libakamr`、`libakmp3`、`libakg722`、`libakresample` | 音频编码、解码和重采样 |
| `libakaudiofilter`、`libakcrydetect`、`libaksoundleveldetect`、`libaksoundsourcelocalization`、`libaklinearrayssl` | 音频效果和分析：滤波、哭声检测、声音强度、声源定位 |
| `libplat_npu`、`libak_nna`、`libak_nne`、`libmpi_svp`、`libak_object_filter` | 神经网络加速器和引擎、智能视觉（目标分类）、检测框过滤 |
| `libak_eis`、`libak_mrd`、`libak_vibe` | 电子防抖、运动区域检测、背景建模 |
| `librtsp_server`、`libapp_video` | RTSP 服务器、视频辅助功能 |
| `libapp_its`、`libapp_ats` | ISP 和音频调试工具服务 |

## 许可证

Apache License 2.0，见 [LICENSE](LICENSE) 和 [NOTICE](NOTICE)。该许可证适用于源代码、脚本、配置文件以及 `platform/lib` 中的预编译库。第三方组件保留各自的许可证，见 [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md)。报告安全问题请见 [SECURITY.md](SECURITY.md)。
