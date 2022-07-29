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

bool elf_probef(struct fs_node *file) {
  vfs_seek(file, 0);
  char magic[4];
  int b = vfs_read(file, magic, 4);
  magic[b] = 0;
  // for (int i = 0; i < 4; i++)
  //   printk("elf: %x %c\n", (unsigned char)magic[i], (unsigned char)magic[i]);
  return magic[0] == 0x7f && strcmp(magic + 1, "ELF") == 0;
}

bool elf_probe(const char *path) {
  struct fs_node *file = vfs_open(path);
  if (!file) {
    return false;
  }
  return elf_probef(file);
}

void elf_load_segment(struct task *task, struct fs_node *file,
                      struct elf_program_header *segment) {
  if (segment->type != ELF_PHEADER_LOAD)
    return;
  klogf("loadable segment: vaddr=0x%x size=%d", segment->vaddr, segment->memsz);
  if (segment->vaddr + segment->memsz >= VIRT_BASE) {
    klogf("segment is overlapping with kernel memory space, "
          "skipping");
    return;
  }
  vfs_seek(file, segment->offset);
  for (int i = 0; i < PG_RND_UP(segment->memsz) / PAGE_SIZE; i++)
    map_addr(task->pgd, segment->vaddr + i * PAGE_SIZE, 1, true, false);
  void *addr = (void *)segment->vaddr;
  int b = vfs_read(file, addr, segment->filesz);
  if (b != segment->filesz) {
    printk("elf: failed to read segment\n");
    return;
  }
  if (segment->memsz > segment->filesz) {
    memset(addr + segment->filesz, 0, segment->memsz - segment->filesz);
  }
}

void elf_loadf(struct task *tsk, struct fs_node *file) {
  if (!elf_probef(file)) {
    klogf("cannot load elf, invalid magic") return;
  }
  vfs_seek(file, 0);
  struct elf_header header;
  vfs_read(file, &header, sizeof(struct elf_header));

  char elf[4];
  memcpy(elf, header.ident.magic + 1, 3);
  elf[3] = 0;

  vfs_seek(file, header.pheader_offset);
  struct elf_program_header *segments =
      kmalloc(header.phentry_size *
              header.pheader_num); // should this address be modified?
                                   // 0xc040a500 and where
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

  vfs_seek(file, header.sheader_offset);
  struct elf_section_header *sections =
      kmalloc(header.shentry_size * header.sheader_num);
  vfs_read(file, sections, header.shentry_size * header.sheader_num);

  for (int i = 0; i < header.sheader_num; i++) {
    vfs_seek(file, sections[header.shstrndx].offset + sections[i].name);
    char name[50];
    vfs_read(file, name, 50);
    klogf("sections[%d]: name=%s type=%d flags=%x addr=0x%x offset=0x%x "
          "size=%d align=%d",
          i, name, sections[i].type, sections[i].flags, sections[i].addr,
          sections[i].offset, sections[i].size, sections[i].addralign);
  }
  if (tsk->pgd) {
    activate_pgd(kernel_pgd);
    kfree(tsk->pgd, 4096);
  }
  tsk->pgd = create_pgd();
  pagedir_t pgd = get_current_pgd();
  activate_pgd(tsk->pgd);

  struct elf_program_header *pheader_load = NULL;
  for (int i = 0; i < header.pheader_num; i++) {
    klogf("segments[%d]: type=%d offset=0x%x addr=0x%x vaddr=0x%x filesz=%d "
          "memsz=%d "
          "flags=%x align=%d",
          i, segments[i].type, segments[i].offset, segments[i].paddr,
          segments[i].vaddr, segments[i].filesz, segments[i].memsz,
          segments[i].flags, segments[i].align);
    elf_load_segment(tsk, file, &segments[i]);
  }

  activate_pgd(pgd);
  tsk->binfo.entry = header.entry;
  tsk->name = strdup(file->path);

cleanup:
  kfree(segments, header.phentry_size * header.pheader_num);
  kfree(sections, header.shentry_size * header.sheader_num);

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
  vfs_close(file);
}

void elf_exec(struct task *tsk) {
  // idt_disable_hwinterrupts();
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
