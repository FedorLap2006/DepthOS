# DepthOS
[![Build Status](https://github.com/FedorLap2006/DepthOS/actions/workflows/kernel.yml/badge.svg)](https://github.com/FedorLap2006/DepthOS/actions/workflows/kernel.yml)
[![Join the chat at https://gitter.im/depthos-dev/community](https://badges.gitter.im/depthos-dev/community.svg)](https://gitter.im/depthos-dev/community?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)



Stable, flexible and very simple in use Operating System, which doesn't restrict you

## Dependencies
DepthOS requires GNU Make, GCC and NASM to build. And QEMU to run.

## Build
Custom toolchain can be configured through environment variables:
- `BINCPATH` - Root directory of GCC and GNU binutils (defaults to `/bin`)
- `CC` - Custom C compiler (defaults to `$BINCPATH/gcc`)
- `LD` - Custom linker (defaults to `$BINCPATH/ld`)

### Kernel
To just build the kernel, run `make build`. If you also want to test it, run just `make` instead.

### Applications
All applications can be build using `make apps`.

However, each application has it's build system, and might require additional configuration. See application's README for more details

If you weren't using `make apps`, you would need to manually install applications into `initrd/` folder (currently DepthOS supports only the initrd filesystem) and then rebuild the image file with `make initrd`.

To install the applications manually, you can go into `apps/` directory and execute `make install` there. If you only want to install a particular application, see the it's README for details.
## Running

To run DepthOS you can use QEMU's `kernel` and `initrd` flags:
```
qemu-system-i686 -kernel DepthOS-1.0 -initrd initrd.img
```
### ISO
To build an ISO you would first need to install all the binaries into `iso/` folder, that can be done by executing `make iso`.

After that you can use `grub-mkrescue` to build the ISO itself:
```
grub-mkrescue -o DepthOS.iso iso/
```
And run it with QEMU or any other emulator:
```
qemu-system-i686 DepthOS.iso
```
