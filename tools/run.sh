#!/bin/sh


ROOTDIR=`git rev-parse --show-toplevel`

$ROOTDIR/tools/iso.sh
$ROOTDIR/tools/qemu.sh $@
