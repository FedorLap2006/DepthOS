#!/bin/bash


ROOTDIR=`git rev-parse --show-toplevel`
# ${TOOLS_BASEDIR:-.}/tools/make.sh iso
$ROOTDIR/tools/make.sh iso
grub-mkrescue $ROOTDIR/iso/ -o $ROOTDIR/depthos.iso
