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


## Build
Kernel and userland requires a custom toolchain compiled for the platform.
To use it, set `CC` and `LD` environment variables when running `make`.

You can use one produced by "Working with ports" section (in particular, by `cross-toolchain` package).
Like this: `CC=$PORTS_BUILDDIR/tools/cross-gcc/bin/i686-depthos-gcc LD=$PORTS_BUILDDIR/tools/cross-binutils/bin/i686-depthos-ld`

### Kernel
To just build the kernel, run `make build`.

### Applications
### Meson applications
Meson based application will require you to use meson cross-file's functionality.
A typical meson cross-file for DepthOS will look like this:
```
[binaries]
c = '/home/lapfed/code/DepthOS/ports-build/tools/cross-gcc/bin/i686-depthos-gcc'
cpp = '/home/lapfed/code/DepthOS/ports-build/tools/cross-gcc/bin/i686-depthos-g++'
# exe_wrapper = 'qemu-i386'

[host_machine]
system = 'depthos'
cpu_family = 'x86'
cpu = 'i386'
endian = 'little'

```
When configuring a build folder, use `--prefix` option, and set it to path of `disk-fs` directory.

### Makefile applications
These will require `CC`, `LD` and `DESTDIR` options. `DESTDIR`, like in meson, should point to `disk-fs` directory.

## Working with ports

To build ports, you have to install [xbstrap](https://github.com/managarm/xbstrap) first.
Afterwards you must create a build directory (`$PORTS_BUILDDIR`) and `cd` into it.
Then run `xbstrap init ../ports`.

Now you can run `xbstrap compile-tool <tool>` (e.g. gcc) or `xbstrap build <package>` (e.g. libpng) to build tools/packages.

After you have built the package/tool, you can install it by calling `xbstrap install`/`xbstrap install-tool`.
If it's a package, it will be installed into the `$PORTS_BUILDDIR/system-root`.
If it's a tool it's gonna be installed into `$PORTS_BUILDDIR/tools/path/to/tool` (e.g. `$BUILDDIR/tools/cross-gcc/bin/i686-depthos-gcc`).

To install the package into the image, you'd have to set the `SYSROOT` variable to `$PORTS_BUILDDIR/system-root` when running `tools/sync.sh` script.

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
