#pragma once

#include <depthos/paging.h>
#include <depthos/stdtypes.h>

#define MAP_PRIVATE 0x01
#define MAP_FIXED 0x04
#define MAP_ANON 0x08

struct sc_mmap_params {
  void *addr;
  size_t length;
  int prot;
  int flags;
  int fd;
  off_t offset;
};

int do_mmap(pagedir_t pgd, void *addr, size_t length, int prot, int flags,
              int fd, off_t offset);

struct vm_area_ops;

typedef struct vm_area {
  /**
   * Amount of processes with access to the memory area.
   * Decreases each time vma_dispose is called.
   * Once the last process disposes the area, its space is marked free to use by
   * other processes.
   */
  int n_proc;
  /**
   * Address of the area.
   */
  uintptr_t addr;
  /**
   * Total size of the area in bytes.
   */
  size_t size;
  /**
   * Can be set by files or devices to prevent accessing out-of-bounds addresses
   * on unallocated pages.
   */
  size_t limit;
  /**
   * Amount of physical pages area occupies.
   */
  size_t n_pages;
  /**
   * Physical address is located at.
   */
  uintptr_t phys_addr; // TODO: page array

  /**
   * Parent pgd the area is attached to.
   */
  pagedir_t pgd;
  /**
   * Access protection flags.
   */
  int prot; // TODO: pgdprot_t type
#define VMA_FLAGS_COW 0x1
#define VMA_FLAGS_ANON MAP_FIXED
#define VMA_FLAGS_FIXED MAP_ANON
  /**
   * Area flags.
   */
  int flags;
  /**
   * File area is mapped to.
   */
  struct fs_node *file;
  /**
   * Must be divisible by PAGE_SIZE
   */
  off_t file_off;

  /**
   * Collection of functions to interact with the area.
   */
  struct vm_area_ops *ops;

  /**
   * Task that owns the the area.
   */
  struct task *parent;

} vm_area_t;


enum vm_fault_kind {
  VM_FAULT_IGNORE = 0x0,
  VM_FAULT_CRASH = 0x1,
  VM_FAULT_BUSERROR = 0x2
};

struct vm_area_ops {
  /**
   * Load a page from a file into a virtual memory area.
   *
   * @param vaddr Memory address to load page into
   * @param offset Offset in a file to load the page from, must be a multiple of
   * PAGE_SIZE
   */
  enum vm_fault_kind (*load_page)(struct vm_area *area, uintptr_t vaddr, off_t offset);
};

struct vm_area *vma_create(struct task *task, uintptr_t addr, size_t size,
                           int flags);
int vma_map_file(struct vm_area *area, int fd, off_t fd_off);
void vma_protect(struct vm_area *area, int prot);
void vma_dispose(struct task *task, struct vm_area *area);
void vma_map(struct vm_area *area, int fd, off_t fd_off);
void vma_unmap(struct vm_area *area);
