# DepthOS
[![Build Status](https://github.com/FedorLap2006/DepthOS/actions/workflows/kernel.yml/badge.svg)](https://github.com/FedorLap2006/DepthOS/actions/workflows/kernel.yml)
[![Join the chat at https://gitter.im/depthos-dev/community](https://badges.gitter.im/depthos-dev/community.svg)](https://gitter.im/depthos-dev/community?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

Stable, flexible and very simple in use Operating System, which doesn't restrict you

## Notable features
- ATA support (with DMA)
- SB16 support
- PC speaker support
- Ext2 (readonly)
- Virtual memory management
- Userland

## Ports
- GCC compiler
- Binutils suite
- Coreutils (only some)
- GNUGo
- libpng
- zlib
- bash

## Dependencies
DepthOS requires GNU Make, GCC and NASM to build. And QEMU to run.


### Kernel
To just build the kernel, run `make build`.

## Packages

To build [packages](https://github.com/FedorLap2006/DepthOS-packages), you have to install [xbstrap](https://github.com/managarm/xbstrap) first.

Afterwards you must create a build directory (`$PKGS_BUILDDIR`) and `cd` into it.
Then run `xbstrap init ../pkgs`.

Now you can run `xbstrap compile-tool <tool>` (e.g. gcc) or `xbstrap build <package>` (e.g. libpng) to build tools/packages.

After you have built the package/tool, you can install it by calling `xbstrap install`/`xbstrap install-tool`.
If it's a package, it will be installed into the `$PKGS_BUILDDIR/system-root`.
If it's a tool it's going to be installed into `$PKGS_BUILDDIR/tools/path/to/tool` (e.g. `$BUILDDIR/tools/cross-gcc/bin/i686-depthos-gcc`).

To install the package into the image, you'd have to set the `SYSROOT` variable to `$PKGS_BUILDDIR/system-root` when running `tools/sync.sh` script.

## Build
Kernel and userland requires a custom toolchain compiled for the platform.
To use it, set `CC` and `LD` environment variables when running `make`.

> [!NOTE]
> You can use `cross-gcc` and `cross-binutils` tools from previous section to build the kernel as well.
> Like this: `CC=$PKGS_BUILDDIR/tools/cross-gcc/bin/i686-depthos-gcc LD=$PKGS_BUILDDIR/tools/cross-binutils/bin/i686-depthos-ld make ...`

## Generating the image

DepthOS requires a hard drive image to run, it contains all the necessary data and programs.
To generate it, you can use `tools/sync.sh` script. It will copy everything from `disk-fs` folder (which is automatically created when you build applications).
If `SYSROOT` environment variable is set, it will also copy all files from there.

The resulting image is located in the `_disk_image.raw` file.
### ISO
Before running DepthOS, you will also need to build an ISO. You can do so by using `tools/iso.sh` script.

## Running
To run DepthOS you can use `tools/qemu.sh` script, it provides all necessary kernel parameters.
Although currently you will also need to pass `-audiodev pa,id=pa -device sb16,audiodev=pa` to it. This will be fixed soon.
