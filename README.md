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

### Kernel
To just build the kernel, run `make build`.

### Applications
All applications can be build using `make apps`.

However, each application has it's build system, and might require additional configuration. See application's README for more details

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
