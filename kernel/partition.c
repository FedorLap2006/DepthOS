#include <depthos/errno.h>
#include <depthos/logging.h>
#include <depthos/partition.h>

#define IMPL(dev) ((struct generic_partition *)dev->impl)
#define SDEV(pdev) IMPL(pdev)->dev

int partdev_read(struct device *pdev, void *buf, size_t count, off_t *offset) {
  // assert(IMPL(pdev)->sector_count);
  if (!IMPL(pdev)->sector_count)
    return 0;

  if (*offset + count >= IMPL(pdev)->sector_count) {
    count = IMPL(pdev)->sector_count - *offset;
  } else if (!SDEV(pdev)->read) {
    return ENIMPL;
  }

  off_t tmpoff = *offset + IMPL(pdev)->lba;
  // klogf("tmpoff: %d (offset=%d lba=%d)", tmpoff, *offset, IMPL(pdev)->lba);
  int ret = SDEV(pdev)->read(SDEV(pdev), buf, count, &tmpoff);
  *offset = tmpoff - IMPL(pdev)->lba;
  return ret;
}

int partdev_write(struct device *pdev, void *buf, size_t count, off_t *offset) {
  // assert(IMPL(pdev)->sector_count);
  if (!IMPL(pdev)->sector_count)
    return 0;

  if (*offset + count >= IMPL(pdev)->sector_count) {
    count = IMPL(pdev)->sector_count - *offset;
  } else if (!SDEV(pdev)->write) {
    return ENIMPL;
  }

  off_t tmpoff = *offset + IMPL(pdev)->lba;
  // klogf("tmpoff: %d (offset=%d lba=%d)", tmpoff, *offset, IMPL(pdev)->lba);
  int ret = SDEV(pdev)->write(SDEV(pdev), buf, count, &tmpoff);
  *offset = tmpoff - IMPL(pdev)->lba;
  return ret;
}

int partdev_seek(struct device *pdev, off_t offset, int whence, off_t *pos) {
  // TODO: implement
  return ENIMPL;
}

struct device *create_partition_device(char *name,
                                       struct generic_partition *partition) {
  struct device *pdev = kmalloc(sizeof(struct device));
  pdev->block_size = partition->dev->block_size;
  pdev->type = partition->dev->type;
  pdev->name = name;
  pdev->impl = partition;
  pdev->pos = 0;
  pdev->read = partdev_read;
  pdev->write = partdev_write;
  pdev->seek = partdev_seek;
  return pdev;
}

struct generic_partition *create_mbr_partition(struct device *dev, int idx,
                                               struct mbr_partition p) {
  struct generic_partition *part = kmalloc(sizeof(struct generic_partition));
  part->lba = p.lba;
  part->sector_count = p.sectors;
  part->index = idx;
  part->dev = dev;
  part->attr = p.attributes & MBR_PART_ATTR_ACTIVE_BOOTABLE != 0
                   ? PARTITION_ATTR_BOOTABLE
                   : 0;
  return part;
}
