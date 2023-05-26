#pragma once

#include <depthos/stdtypes.h>
#include <depthos/tools.h>

struct device;

struct mbr_partition {
#define MBR_PART_ATTR_ACTIVE_BOOTABLE (1 << 7)
  uint8_t attributes;
  uint8_t chs[3];
#define MBR_PART_TYPE_GPT 0xEE
  uint8_t type;
  uint8_t chs_last[3];
  size_t lba;
  size_t sectors;
} __pack;

struct mbr {
  unsigned char bootstrap[440];
  uint32_t signature;
  uint16_t reserved;
  struct mbr_partition partitions[4];
  uint16_t valid_bootsector_signature;
} __pack;

struct mbr *mbr_parse(struct device *dev);
