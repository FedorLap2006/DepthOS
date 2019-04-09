#pragma once

#include <types.h>

#include "vfs.h"

#define MAX_DEVS

typedef enum __device_kind {
	DEVICE_UNKNOWN,
	DEVICE_CHAR,
	DEVICE_BLOCK,
}dev_kind;

typedef struct __device_t {
	char *name;
	uint32_t devID;
	dev_kind kind;
	
	struct __fs_t *fs;

	uint8_t (*read)(uint8_t *buf,uint32_t offset,uint32_t len, void* dev);
	uint8_t (*write)(uint8_t *buf,uint32_t offset,uint32_t len, void* dev);

	void *priv;

}dev_t;

void devs_init();
uint32_t dev_add(dev_t *dev);
dev_t* dev_get(uint32_t id);
int dev_getnum();

