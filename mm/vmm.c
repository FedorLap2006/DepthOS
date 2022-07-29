#include <depthos/logging.h>
#include <depthos/paging.h>
#include <depthos/proc.h>
#include <depthos/syscall.h>
#include <depthos/vmm.h>

#define MMAP_BASE 0x40000000
int mmap_addr_counter = 0;

DECL_SYSCALL1(vm_map, struct sc_mmap_params *, params) {
  void *addr = do_mmap(current_task->pgd, params->addr, params->length,
                       params->prot, params->flags, params->fd, params->offset);
  klogf("addr=%p length=%x prot=%x flags=%x fd=%d offset=%d", addr,
        params->length, params->prot, params->flags, params->fd,
        params->offset);
  return addr;
}

void *do_mmap(pagedir_t pgd, void *addr, size_t length, int prot, int flags,
              int fd, off_t offset) {
  if (!addr) {
    if (flags & MAP_FIXED) {
      klogf("attempting a fixed mmap on 0x0");
      return -1;
    }
    addr = MMAP_BASE + mmap_addr_counter * PAGE_SIZE;
    mmap_addr_counter += PG_RND_UP(length) / PAGE_SIZE;
  }

  if (flags & MAP_ANON) {
    klogf("mapping zero'ed %p..%p", addr, addr + PG_RND_UP(length));
    map_addr(current_task->pgd, addr, PG_RND_UP(length) / PAGE_SIZE, true,
             false);
    memset(addr, 0, length);
  }

  return addr;
}
