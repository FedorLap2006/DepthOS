#include "dev.h"
#include <heap.h>
#include <memory.h>
#include <task.h>

dev_t *devs = 0;
uint32_t last_dev_id = 0;


void devs_init() {
	devs = (dev_t*)malloc(MAX_DEVS*sizeof(dev_t));
	memset(devs,0,MAX_DEVS*sizeof(dev_t));
	last_dev_id = 0;
	__prockill();
}

uint32_t dev_add(dev_t *dev) {
	devs[last_dev_id] = dev;
	last_dev_id++;
	return last_dev_id-1;
}

int dev_getnum() { return last_dev_id; }
dev_t* dev_get(uint32_t id) {
	return &devs[id];
}
