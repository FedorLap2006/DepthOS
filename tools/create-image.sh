#!/bin/sh

FILE="${2:-_disk_image.raw}"
qemu-img create $FILE $1
mke2fs -b 2048 -I 512 -t ext2 $FILE
