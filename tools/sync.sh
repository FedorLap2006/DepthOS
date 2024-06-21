#!/bin/bash

FSDIR=disk-fs
ROOTDIR=`git rev-parse --show-toplevel`
MOUNTDIR=_mnt
IMG=_disk_image.raw
# RSYNC_FLAGS="-vacI --update"
# RSYNC_FLAGS="-vacu"
RSYNC_FLAGS="-vacu"
echo -n "Mounting... " && mount -t ext2 -o loop,rw ${ROOTDIR}/${IMG} ${ROOTDIR}/${MOUNTDIR} && echo "done"
# cp -r "${ROOTDIR}/${FSDIR}"/* _mnt
# echo "SYSROOT: ${SYSROOT}"
[ -n "${SYSROOT}" ] && echo "rsync ${RSYNC_FLAGS} ${SYSROOT}/ ${ROOTDIR}/${MOUNTDIR}" && echo -n "Syncing sysroot... " && rsync ${RSYNC_FLAGS} ${SYSROOT}/ ${ROOTDIR}/${MOUNTDIR} && echo "done"

echo "rsync ${RSYNC_FLAGS} ${ROOTDIR}/${FSDIR}/ ${ROOTDIR}/${MOUNTDIR}"
echo -n "Syncing template directory... " && rsync ${RSYNC_FLAGS} ${ROOTDIR}/${FSDIR}/ ${ROOTDIR}/${MOUNTDIR} && echo "done"
sleep 1
echo -n "Unmounting... " && umount ${ROOTDIR}/${MOUNTDIR} && echo "done"

