#!/bin/bash

QEMU=qemu-system-i386
BOOT_ISO=depthos.iso
DISK_IMAGE=_disk_image.raw
# QEMU_ARGS="-M pc-i440fx-2.8 -m 4G -monitor tcp:127.0.0.1:55555 -serial stdio"
# COMMON_ARGS=( "-M pc-i440fx-2.8 -m 1G -serial stdio" ) # -d trace:ide_data_*
COMMON_ARGS=( "-m 1G -serial stdio" ) # -d trace:ide_data_*
DRIVE_BOOT="-drive id=boot,file=$BOOT_ISO,format=raw,if=none,unit=0"
DRIVE_STOR="-drive id=disk1,file=$DISK_IMAGE,format=raw,if=none,unit=1"
DEV_ARGS=( "-device ide-hd,drive=boot,bus=ide.0 -device ide-hd,drive=disk1,bus=ide.0" )

# while getopts "dl" opt; do
#   case "$opt" in
#     d)
#       QEMU_ARGS="${QEMU_ARGS} -s -S"
#       ;;
#     l)
#       QEMU_ARGS="${QEMU_ARGS} -d int,pcall,cpu,fpu -D qemu.log"
#       ;;
#   esac
# done

# qemu-system-i386 -M pc-i440fx-2.8 -drive id=boot,file=depthos.iso,format=raw,if=none -device ahci,id=ahbus -device ide-cd,drive=boot -m 4G -monitor none -serial stdio

echo $QEMU ${COMMON_ARGS[@]} ${DRIVE_BOOT} ${DRIVE_STOR} ${DEV_ARGS[@]} $@  
$QEMU ${COMMON_ARGS[@]} ${DRIVE_BOOT} ${DRIVE_STOR} ${DEV_ARGS[@]} $@  
  # -device ide-hd,drive=boot,bus=ide.0 -device ide-cd,drive=disk1,bus=ide.0
