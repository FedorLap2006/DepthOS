#include <depthos/kernel.h>
#include <depthos/tools.h>
#include <depthos/errno.h>
#include <depthos/heap.h>
#include <depthos/list.h>
#include <depthos/logging.h>
#include <depthos/math.h>
#include <depthos/paging.h>
#include <depthos/pmm.h>
#include <depthos/proc.h>
#include <depthos/stdtypes.h>
#include <depthos/string.h>
#include <depthos/syscall.h>
#include <depthos/vmm.h>

#if defined(CONFIG_VMM_LOG_ENABLE)
#define vmm_log(...) klogf(__VA_ARGS__)
#else
#define vmm_log(...)
#endif

#define MMAP_BASE 0x40000000
// #define MMAP_BASE 0x25000000
// #define MMAP_BASE 0x01200000

DECL_SYSCALL1(mmap, struct sc_mmap_params *, params) {
  uintptr_t addr = (uintptr_t)params->addr;

  if (!addr) {
    if (params->flags & MAP_FIXED) {
      vmm_log("could not map fixed region: addr is NULL");
      return INVALID_ADDR;
    }
    addr = MMAP_BASE;
    addr += current_task->mmap_bump_idx * PAGE_SIZE;
    klogf("mmap bump idx: 0x%lx (%ld, %ld)", current_task->mmap_bump_idx, current_task->process->pid, current_task->thid);
    current_task->mmap_bump_idx += PG_RND_UP(params->length) / PAGE_SIZE;
  }


  int ret = do_mmap(current_task->pgd, (void*)addr, params->length,
                       params->prot, params->flags, params->fd, params->offset);
  if (!ret) {
    vmm_log("could not map region: %d",
            errno);
    return -errno;
  }
  return (long)addr;
}

// DECL_SYSCALL2(munmap, uintptr_t, addr, size_t, len) {
// }

bool vma_overlap(struct vm_area *a, struct vm_area *b) {
  return a->addr > b->addr ? (b->addr + b->size) > a->addr
                           : (a->addr + a->size) > b->addr;
}

int do_mmap(pagedir_t pgd, void *addr, size_t length, int prot, int flags,
              int fd, off_t offset) {
  if (!length) {
    errno = EINVAL;
    return 0;
  }

  uintptr_t end;

  if (safe_add((uintptr_t)addr, length, &end)) {
    vmm_log("could not map: overflow");
    errno = EOVERFLOW;
    return 0;
  }

  klogf("mmap: range=(%p...%p) prot=%d flags=%d fd=%d off=%ld",
        addr, (void*)end, prot, flags, fd, offset);


  if (end > VIRT_BASE) {
    vmm_log("could not proceed: overlaps with kernel");
    errno = EEXIST;
    return 0;
  }

  if (flags & MAP_ANON != 0) {
    vmm_log("mapping empty region: %p..%p", addr, addr + PG_RND_UP(length));
  }

  struct vm_area *area = vma_create(current_task, (uintptr_t)addr, PG_RND_UP(length), flags);
  if (!area) {
    vmm_log("could not create mapped region: %d", errno);
    return 0;
  }

  if (((flags & MAP_ANON) == 0) && fd) {
    vmm_log("mapping %d to memory region", fd);
    int ret = vma_map_file(area, fd, offset);
    if (ret < 0) {
      vmm_log("bruh? %d", errno);
      // XXX: overlaps?
      vma_dispose(current_task, area);
      return 0;
    }
  }
  return 1;
}

struct vm_area *vma_create(struct task *task, uintptr_t addr, size_t size,
                           int flags) {
  if (addr % PAGE_SIZE) {
    errno = EINVAL;
    return NULL;
  }

  struct vm_area *vma = (struct vm_area *)kmalloc(sizeof(struct vm_area));
  // TODO: cow!
  vma->flags = flags;
  vma->addr = addr;
  vma->size = size;
  vma->n_pages = ceil_div(size, PAGE_SIZE);

  vma->parent = task;
  vma->pgd = task->pgd;
  vma->ops = NULL;
  vma->phys_addr = 0;
  vma->limit = 0;
  vma->file = NULL;
  vma->file_off = 0;

  vma->n_proc = 1;
  if (!task->vm_areas) {
    task->vm_areas = list_create();
    list_push(task->vm_areas, (list_value_t)vma);
    return vma;
  }

  vmm_log("walking through task areas");
  list_foreach(task->vm_areas, item) { // TODO: insert ordered value
    struct vm_area *area = list_item(item, struct vm_area *);
    if (vma_overlap(area, vma)) {
      // TODO: discard on MAP_FIXED
      errno = ENOMEM;
      kfree(vma, sizeof(struct vm_area));
      return NULL;
    }
    if (area->addr > vma->addr) {
      list_insert_value(task->vm_areas, item->prev, item->next,
                        (list_value_t)vma);

      return vma;
    }
  }
 
  list_push(task->vm_areas, (list_value_t)vma);

  return vma;
}

int vma_map_file(struct vm_area *area, int fd, off_t offset) {
  if (fd > TASK_FILETABLE_MAX) {
    errno = EBADF;
    return -1;
  }

  struct fs_node *file = area->parent->filetable[fd];
  if (!file) {
    errno = EBADF;
    return -1;
  }

  if (!file->ops || !file->ops->mmap) {
    errno = ENIMPL;
    return -1;
  }

  area->file = file;
  if (offset % PAGE_SIZE) {
    errno = EINVAL;
    return -1;
  }
  area->file_off = offset;

  int ret = file->ops->mmap(file, area);
  if (ret) {
    errno = ret;
    return -1;
  }

  if (area->n_pages >
      PG_RND_UP(area->size) / PAGE_SIZE) { // mmap might change the parameters
    area->n_pages = PG_RND_UP(area->size) / PAGE_SIZE;
  }

  return 0;
}

void vma_deallocate(struct vm_area *area) {
  kfree(area, sizeof(struct vm_area));
}

void vma_dispose(struct task *task, struct vm_area *area) {
  struct list_entry *item = NULL;
  list_foreachv(task->vm_areas, item) {
    if (list_item(item, struct vm_area *) == area) {
      break;
    }
  }
  if (!item)
    return;
  list_remove(task->vm_areas, item);
  area->n_proc--;
  if (!area->n_proc)
    vma_deallocate(area);

  unmap_addr(task->pgd, area->addr, area->size / PAGE_SIZE, area->n_proc == 0);
}

enum vm_fault_kind vma_pagefault_handler(regs_t *r, uintptr_t cr2) {
  vmm_log("handling vma pagefault");
  struct pagefault_state state = PAGEFAULT_STATE_STRUCT(r);
  bool is_kernel_task = current_task->pgd == kernel_pgd;
  uintptr_t aligned_cr2 = PG_RND_DOWN(cr2);

  vmm_log("vm_areas: %p (first=%p size=%ld)", current_task->vm_areas,
          current_task->vm_areas->first, current_task->vm_areas->length);
  list_foreach(current_task->vm_areas, item) {
    struct vm_area *area = list_item(item, struct vm_area *);
    if (cr2 >= area->addr && cr2 < area->addr + area->size) {
      vmm_log("found!");
      if (area->file) {
        // TODO: throw bus error and stuff when out of bounds
      }
      vmm_log("area {at=%p size=%ld npages=%ld file=%p}", (void *)area->addr,
              area->size, area->n_pages, area->file);
      if (!state.present) {
        if (area->limit && cr2 >= (area->addr + area->limit)) {
          return VM_FAULT_BUSERROR;
        }
        if (area->phys_addr) {
          vmm_log("physical addr");
          map_addr_fixed(current_task->pgd, area->addr, area->phys_addr,
                        area->n_pages, !is_kernel_task, true);

          // TODO: MAP_ANON?

        } else if (area->ops && area->ops->load_page) {
          vmm_log("loading a page");
          int st = area->ops->load_page(
              area, aligned_cr2, aligned_cr2 - area->addr + area->file_off);
          // vmm_log("status: %d", st);
          if (st != VM_FAULT_IGNORE) {
            return st;
          }
        } else {
          vmm_log("mapping anon page: at=%p kernel=%d rw=%d", (void *)aligned_cr2,
                is_kernel_task, true);
#ifdef CONFIG_VMA_MAP_WHOLE
          map_addr(current_task->pgd, area->addr, area->n_pages,
                   !is_kernel_task, true);
#else
          map_addr(current_task->pgd, aligned_cr2, 1, !is_kernel_task, true);
#endif
        }

        if (area->flags & MAP_ANON) {
#ifdef CONFIG_VMA_MAP_WHOLE
          vmm_log("zeroing accessed area");
          memset((void *)area->addr, 0, PAGE_SIZE * area->n_pages);
#else
          vmm_log("zeroing accessed page");
          memset((void *)aligned_cr2, 0, PAGE_SIZE);
#endif
        }
        return VM_FAULT_IGNORE;
      } else if (state.write) {
        vmm_log("access violation: write");
        page_t *pg = get_page(current_task->pgd, cr2);
        // vmm_log("read/write: pg=%p frame=%p", pg, parse_page(pg).frame);
        if ((*pg & (1 << PTE_C_COW_SHIFT)) != 0) {
          // TODO: clone
          return 0;
        }

        return VM_FAULT_CRASH;
      }
    }
  }
  return VM_FAULT_CRASH;
}
