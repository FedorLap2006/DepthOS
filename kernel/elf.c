#include <depthos/bitmap.h>
#include <depthos/elf.h>
#include <depthos/fs.h>
#include <depthos/heap.h>
#include <depthos/kernel.h>
#include <depthos/logging.h>
#include <depthos/paging.h>
#include <depthos/pmm.h>
#include <depthos/proc.h>
#include <depthos/string.h>

bool elf_probe(const char *path) {
  struct fs_node *file = vfs_open(path);
  if (!file) {
    // printk("UHHHHHH\n");
    return false;
  }
  vfs_seek(file, 0);
  char magic[4];
  int b = vfs_read(file, magic, 4);
  magic[b] = 0;
  // for (int i = 0; i < 4; i++)
  //   printk("elf: %x %c\n", (unsigned char)magic[i], (unsigned char)magic[i]);
  return magic[0] == 0x7f && strcmp(magic + 1, "ELF") == 0;
}

void elf_loadf(struct task *tsk, struct fs_node *file) {
  char magic[5];
  vfs_seek(file, 0);
  int b = vfs_read(file, magic, 4);
  magic[b] = 0;
  if (b != 4 || magic[0] != 0x7f || strcmp(magic + 1, "ELF") != 0) {
    klogf("file is not elf");
    return;
  }
  vfs_seek(file, 0);
  struct elf_header header;
  vfs_read(file, &header, sizeof(struct elf_header));

  char elf[4];
  memcpy(elf, header.ident.magic + 1, 3);
  elf[3] = 0;

  vfs_seek(file, header.pheader_offset);
  struct elf_program_header *segments =
      kmalloc(header.phentry_size * header.pheader_num);
  vfs_read(file, segments, header.phentry_size * header.pheader_num);
  klogf("%d", (uint32_t)&header.entry - (uint32_t)&header);
  klogf("%s: ident={elf=%s magic=%x x64=%d big=%d version=%d} type=%d "
        "machine=%d version=%d entry=0x%x "
        "pheader={0x%x,size=%d,count=%d} "
        "sheader={0x%x,size=%d,actual=%d,count=%d} "
        "stridx=%d",
        file->path, elf, header.ident.magic[0], header.ident.x64,
        header.ident.big_endian, header.ident.version, header.type,
        header.machine, header.version, header.entry, header.pheader_offset,
        header.phentry_size, header.pheader_num, header.sheader_offset,
        header.shentry_size, sizeof(struct elf_section_header),
        header.sheader_num, header.shstrndx);

#if 1
  vfs_seek(file, header.sheader_offset);
  struct elf_section_header *sections =
      kmalloc(header.shentry_size * header.sheader_num);
  vfs_read(file, sections, header.shentry_size * header.sheader_num);

  for (int i = 0; i < header.sheader_num; i++) {
    vfs_seek(file, sections[header.shstrndx].offset + sections[i].name);
    char name[50];
    vfs_read(file, name, 50);

    if (i == 1) {
      vfs_seek(file, sections[i].offset);
      unsigned char *buf = kmalloc(sections[i].size);
      memset(buf, 0, sections[i].size);
      vfs_read(file, buf, sections[i].size);
      for (int k = 0; k < sections[i].size; k++) {
        klogf("[%d]: %x", k, buf[k]);
      }
    }
    klogf("sections[%d]: name=%s type=%d flags=%x addr=0x%x offset=0x%x "
          "size=%d align=%d",
          i, name, sections[i].type, sections[i].flags, sections[i].addr,
          sections[i].offset, sections[i].size, sections[i].addralign);
  }
#endif

  for (int i = 0; i < header.pheader_num; i++) {
    klogf("segments[%d]: type=%d offset=0x%x addr=0x%x vaddr=0x%x filesz=%d "
          "memsz=%d "
          "flags=%x align=%d",
          i, segments[i].type, segments[i].offset, segments[i].paddr,
          segments[i].vaddr, segments[i].filesz, segments[i].memsz,
          segments[i].flags, segments[i].align);
    switch (segments[i].type) {
    case ELF_PHEADER_LOAD:
      klogf("segments[%d]: {vaddr=0x%x size=%d}", i, segments[i].vaddr,
            segments[i].memsz);
      if (segments[i].vaddr + segments[i].memsz >= VIRT_BASE)
        goto finish;

      // klogf("pgd = %d", sizeof(kernel_pgd));
      // tsk->pgd = kmalloc(sizeof(kernel_pgd));
      if (tsk->pgd) {
        activate_pgd(kernel_pgd);
        kfree(tsk->pgd, 4096);
      }
      tsk->pgd = create_pgd();
      // memset(tsk->pgd, 0, sizeof(kernel_pgd));
      // memcpy(tsk->pgd, kernel_pgd, sizeof(kernel_pgd));
      for (int j = 0; j < header.sheader_num; j++) {
        if (!sections[j].addr)
          continue;
        // tsk->pgd[pde_index(ROUND_DOWN(sections[j].addr, 0x1000))] =
        // 0;
        map_addr(tsk->pgd, sections[j].addr, 1, true);
      }

#if 0
      for (int j = 0; j < segments[i].memsz; j += 4096 * 1024) {
        memset(table, 0, 4096);
        klogf("pgd[%d]:", pde_index(addr));
        for (int k = 0; k < ROUND_UP(segments[i].memsz, 0x1000); k += 4096) {
          table[pte_index(addr + j + k)] = make_pte(pmm_alloc(1), 1, 1);
          klogf("[%d] (0x%x) at 0x%x: %x", pte_index(addr + j + k),
                addr + j + k, table[pte_index(addr + j + k)] & 0xFFFFF000,
                table[pte_index(addr + j + k)]);
        }
        tsk->pgd[pde_index(addr + j)] =
            make_pde(ADDR_TO_PHYS(table), 1, 1);
      }
#endif
      // klogf("directory:");
      // for (int j = 0; j < 1024; j++) {
      //   if (!tsk->pgd[j])
      //     continue;
      //   klogf("[0x%x]: %x:", j * 4096 * 1024, tsk->pgd[j]);
      //   for (int k = 0; k < 1024; k++) {
      //     pageinfo_t pg = parse_page(
      //         get_page(tsk->pgd, j * 4096 * 1024 + k * 4096));
      //     klogf("\t[0x%x]: 0x%x", j * 4096 * 1024 + k * 4096, pg.frame);
      //   }
      // }
#if 0
      klogf("table:");
      for (int j = 0; j < 1024; j++) {
        page_t *page = get_page(tsk->pgd, addr + j * 4096);
        if (!page) {
          klogf("0x%x is not mapped", addr + j * 4096);
          continue;
        }
        klogf("[0x%x]: %x", addr + j * 4096, *page);
      }
      klogf("kernel:");
      for (int j = 0; j < 1024; j++) {
        if (kernel_pgd[j])
          klogf("[0x%x]: %x", j * 4096 * 1024, kernel_pgd[j]);
      }
      // *(char *)addr = 0;

      pagetb_t stack_table = kmalloc(4096);
      tsk->pgd[pde_index(VIRT_BASE - 4096)] =
          make_pde(stack_table, 1, 1);
      stack_table[pte_index(VIRT_BASE - 4096)] = make_pte(pmm_alloc(1), 1, 1);
#endif

      map_addr(tsk->pgd, VIRT_BASE - 4096, 1, true);
      unsigned char *buf = kmalloc(segments[i].memsz);
      if (!buf)
        panicf("cannot allocate program buffer");

      // klogf("segment size: %d", segments[i].memsz);
      // for (uint32_t j = 0; j < segments[i].memsz; j++) {
      //   klogf("[%d]: %x", j, buf[j]);
      // }
      pagedir_t pgd = get_current_pgd();
      activate_pgd(tsk->pgd); // TODO: move to sched or syscall
      for (int j = 0; j < header.sheader_num; j++) {
        if (!sections[j].addr)
          continue;
        memset(buf, 0, sections[j].size);
        vfs_seek(file, sections[j].offset);
        vfs_read(file, buf, sections[j].size);
        memcpy(sections[j].addr, buf, sections[j].size);
      }
      activate_pgd(pgd);
      // klogf("reading directly from memory:");
      // for (uint32_t j = 0; j < segments[i].memsz; j++) {
      //   klogf("[%d] %x", j, *((unsigned char *)addr + j));
      // }
      // klogf("0x%x", header.entry);
      // klogf("0x%x", *(unsigned char *)header.entry);
      // idt_dump();

      tsk->binfo.entry = header.entry;
      goto finish;
    }
  }

finish:
  tsk->binfo.entry = header.entry;
  tsk->name = strdup(file->path);
  vfs_close(file);
  kfree(segments, header.phentry_size * header.pheader_num);

  // klogf("sections[%d]: type=%d flags=%x addr=0x%x offset=0x%x size=%d
  // align=%d",
  //       header.shstrndx, sections[header.shstrndx].type,
  //       sections[header.shstrndx].flags,
  //       sections[header.shstrndx].addr,
  //       sections[header.shstrndx].offset,
  //       sections[header.shstrndx].size,
  //       sections[header.shstrndx].addralign);
}

void elf_load(struct task *tsk, const char *path) {
  struct fs_node *file = vfs_open(path);
  elf_loadf(tsk, file);
}

void elf_exec(struct task *tsk) {
  idt_disable_hwinterrupts();
  activate_pgd(tsk->pgd);
  __asm__ volatile("mov $((4 * 8) | 3), %%ax;"
                   "mov %%ax, %%ds;"
                   "mov %%ax, %%es;"
                   "mov %%ax, %%fs;"
                   "mov %%ax, %%gs;" ::);
  __asm__ volatile("movl %%eax, %%esp;"
                   "xor %%ebp, %%ebp;"
                   "pushl $((4 * 8) | 3);"
                   "pushl %%eax;"
                   "pushl $0x202;"
                   "pushl $((3 * 8) | 3);"
                   "pushl %%ebx;"
                   "iret"
                   :
                   : "a"(VIRT_BASE), "b"(tsk->binfo.entry));
}
