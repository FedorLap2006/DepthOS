#pragma once

#include <depthos/dev.h>
#include <depthos/heap.h>
#include <depthos/mbr.h>
#include <depthos/stdtypes.h>

struct generic_partition {
#define PARTITION_ATTR_BOOTABLE 0x1
#define PARTITION_ATTR_GPT 0x2
  uint8_t attr;
  struct device *dev;
  size_t lba;
  int index;
  size_t sector_count;
};

struct device *create_partition_device(char *name,
                                       struct generic_partition *partition);

struct generic_partition *create_mbr_partition(struct device *dev, int idx,
                                               struct mbr_partition p);

// struct generic_partition create_gpt_partition(struct device *dev, int idx,
//                                               struct gpt_partition *p);
