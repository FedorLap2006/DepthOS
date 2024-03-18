#include <depthos/fs.h>
#include <depthos/file.h>
#include <depthos/logging.h>
#include <depthos/paging.h>
#include <depthos/string.h>
#include <depthos/vmm.h>

enum vm_fault_kind generic_file_load_mapped_page(struct vm_area *area,
                                                 uintptr_t vaddr,
                                                 off_t offset) {
  char buf[PAGE_SIZE];

  off_t aligned_offset = PG_RND_DOWN(offset);

  if (!area->file->ops || !area->file->ops->read) {
    return VM_FAULT_CRASH; // XXX: panic? that shouldn't happen
  }

  int n = area->file->ops->read(area->file, buf, PAGE_SIZE, &aligned_offset);

  // Since offset points to next readable location, if these two match, we're at
  // the end of the file. If aligned offset is less than requested offset, we're
  // out of bounds.
  if (aligned_offset <= offset) {
    return VM_FAULT_BUSERROR;
  }

  bool is_kernel_task =
      area->pgd == kernel_pgd; // TODO: refactor into a function
  map_addr(area->pgd, vaddr, 1, !is_kernel_task, true); // TODO: overlaps
  memcpy((void *)vaddr, buf, n);
  return 0;
}

soff_t generic_file_seek_size(struct fs_node *file, soff_t pos, off_t size,
                              int whence) {
  switch (whence) {
  case SEEK_END:
    pos += size;
    break;
  case SEEK_CUR:
    if (pos == 0)
      return file->pos;
    // TODO: locks
    pos += file->pos;
    break;
  }

  return vfs_setpos(file, pos, size);
}
