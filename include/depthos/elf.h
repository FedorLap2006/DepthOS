#pragma once

#include <depthos/stddef.h>
#include <depthos/stdtypes.h>

struct elf_header_ident {
  unsigned char magic[4];
  bool x64;
  bool big_endian;
  uint8_t version;
  uint8_t abi;
  char zero[8];
} __pack;

struct elf_header {
  struct elf_header_ident ident;
  uint16_t type;
  uint16_t machine;
  uint32_t version;
  uint32_t entry;
  uint32_t pheader_offset;
  uint32_t sheader_offset;
  uint32_t flags;
  uint16_t header_size;
  uint16_t phentry_size;
  uint16_t pheader_num;
  uint16_t shentry_size;
  uint16_t sheader_num;
  uint16_t shstrndx;
} __pack;

struct elf_section_header {
  uint32_t name;
#define ELF_SHEADER_NULL 0
#define ELF_SHEADER_PROGBITS 1
#define ELF_SHEADER_SYMTAB 2
#define ELF_SHEADER_STRTAB 3
#define ELF_SHEADER_RELA 4
#define ELF_SHEADER_HASH 5
#define ELF_SHEADER_DYNAMIC 6
#define ELF_SHEADER_NOTE 7
#define ELF_SHEADER_NOBITS 8
#define ELF_SHEADER_REL 9
#define ELF_SHEADER_SHLIB 10
#define ELF_SHEADER_DYNSYM 11
#define ELF_SHEADER_LOPROC 0x70000000
#define ELF_SHEADER_HIPROC 0x7fffffff
#define ELF_SHEADER_LOUSER 0x80000000
#define ELF_SHEADER_HIUSER 0xffffffff
  uint32_t type;
#define ELF_SHEADER_FLAG_WRITE 0x1
#define ELF_SHEADER_FLAG_ALLOC 0x2
#define ELF_SHEADER_FLAG_EXECINSTR 0x4
#define ELF_SHEADER_FLAG_MASKPROC 0xf0000000
  uint32_t flags;
  uint32_t addr;
  uint32_t offset;
  uint32_t size;
  uint32_t link;
  uint32_t info;
  uint32_t addralign;
  uint32_t entsize;
};

struct elf_program_header {
#define ELF_PHEADER_NULL 0
#define ELF_PHEADER_LOAD 1
#define ELF_PHEADER_DYNAMIC 2
#define ELF_PHEADER_INTERP 3
#define ELF_PHEADER_NOTE 4
#define ELF_PHEADER_SHILB 5
#define ELF_PHEADER_PHDR 6
#define ELF_PHEADER_LOPROC 0x70000000
#define ELF_PHEADER_HIPROC 0x7fffffff
  uint32_t type;
  uint32_t offset;
  uint32_t vaddr;
  uint32_t paddr;
  uint32_t filesz;
  uint32_t memsz;
#define ELF_PF_X 1
#define ELF_PF_W 2
#define ELF_PF_R 4
  uint32_t flags;
  uint32_t align;
};

/**
 * @brief Check if file at the given path is an ELF executable.
 *
 * @param path Path of the file to check
 * @return true File at the given path is an ELF executable.
 * @return false File at the given path is not an ELF executable.
 */
bool elf_probe(const char *path);

/**
 * @brief Load an ELF application
 *
 * @param path Path to the application
 */
void elf_load(const char *path);