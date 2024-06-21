#!/bin/sh

MOUNTDIR=_mnt

if [ "$1" = -u ]; then
  umount $MOUNTDIR
else
  mount -t ext2 -o loop,rw _disk_image.raw $MOUNTDIR
fi
