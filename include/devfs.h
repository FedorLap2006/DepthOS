#pragma once

#include <dev.h>
#include <heap.h>

enum {
	FS_EXT2,
	FS_NTFS,
	FS_SDOS
};

void devfs_init(int fs_type) {
	fs_t *fs = (fs_t*)malloc(sizeof(fs_t));
	switch(fs_type) {
	case FS_SDOS:
		fs->name = "sdos";
		fs->probe = sdos_probe;
		fs->read = sdos_read;
		fs->write = sdos_write;
		fs->read_dir = sdos_readdir;
		fs->touch_file = sdos_createfile;
		fs->touch_dir = sdos_createdir;
		fs->exist = sdos_exist;
		fs->mount = sdos_mount;
	case FS_NTFS:
	case EXT2:
	default:
		break;
	dev_t *dev = (dev_t*)malloc(sizeof(dev_t));
	dev->read = fs->read;
	dev->write = fs->write;
	dev->fs = fs;
	dev->kind = DEVICE_CHAR;
	dev->devID = 132;
	dev_add(dev);
}
