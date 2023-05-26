#include <depthos/dev.h>
#include <depthos/heap.h>
#include <depthos/mbr.h>

struct mbr *mbr_parse(struct device *dev) {
  struct mbr *res = (struct mbr *)kmalloc(sizeof(struct mbr));
  off_t offset = 0;
  if (!dev->read(dev, res, 1, &offset)) {
    kfree(res, sizeof(struct mbr));
    return NULL;
  }

  return res;
}
