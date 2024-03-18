#pragma once

#include <depthos/partition.h>
#include <depthos/stdtypes.h>
#include <depthos/tools.h>

#define EXT2_SIGNATURE 0xef53

#define EXT2_ROOT_INODE 2
#define EXT2_MAX_FILENAME_LEN 255

enum ext2_fs_state {
  EXT2_FS_STATE_CLEAN = 1,
  EXT2_FS_STATE_ERR = 2,
};

enum ext2_error_handling {
  EXT2_ERROR_IGNORE = 1,
  EXT2_ERROR_REMOUNT = 2,
  EXT2_ERROR_PANIC = 3,
};

struct ext2_superblock {
  uint32_t total_inodes;
  uint32_t total_blocks;
  uint32_t su_block_count;
  uint32_t total_unallocated_blocks;
  uint32_t total_unallocated_inodes;
  uint32_t superblock_blk_num;
  uint32_t log_block_size;    // 10
  uint32_t log_fragment_size; // 10
  uint32_t blocks_per_blk_group;
  uint32_t fragments_per_blk_group;
  uint32_t inodes_per_blk_group;
  uint32_t last_mount_time;
  uint32_t last_written_time;
  uint16_t mounts_since_consistency;
  uint16_t mounts_allowed_before_consistency;
  uint16_t signature; // 0xef53
  uint16_t fs_state;
  uint16_t error_handling;
  uint16_t version_minor;
  uint32_t last_consistency_check_time;
  uint32_t forced_consistency_check_interval;
  uint32_t operating_system_id;
  uint32_t version_major;
  uint16_t reserved_user_id;
  uint16_t reserved_group_id;
  uint32_t first_inode;
  uint32_t inode_size;
  unsigned char unused[932];
} __pack;

struct ext2_blk_group_descriptor {
  uint32_t usage_block_addr;
  uint32_t usage_inode_addr;
  uint32_t inode_table_addr;
  uint16_t unallocated_blocks;
  uint16_t unallocated_inodes;
  uint16_t num_directories;
  char unused[14];
} __pack;

struct ext2_inode {
#define EXT2_INODE_T_FIFO 0x1000
#define EXT2_INODE_T_CHARDEV 0x2000
#define EXT2_INODE_T_DIR 0x4000
#define EXT2_INODE_T_BLOCKDEV 0x6000
#define EXT2_INODE_T_FILE 0x8000
#define EXT2_INODE_T_LINK 0xa000
#define EXT2_INODE_T_SOCKET 0xc000

#define EXT2_INODE_P_O_EXEC 0x001
#define EXT2_INODE_P_O_WRITE 0x002
#define EXT2_INODE_P_O_READ 0x004
#define EXT2_INODE_P_G_EXEC 0x008
#define EXT2_INODE_P_G_WRITE 0x010
#define EXT2_INODE_P_G_READ 0x020
#define EXT2_INODE_P_U_EXEC 0x040
#define EXT2_INODE_P_U_WRITE 0x080
#define EXT2_INODE_P_U_READ 0x100
#define EXT2_INODE_P_STICKY 0x200
#define EXT2_INODE_P_SET_GROUP_ID 0x400
#define EXT2_INODE_P_SET_USER_ID 0x800
  uint16_t type_permissions;
  uint16_t user_id;
  uint32_t size_lo;
  uint32_t last_access_time;
  uint32_t creation_time;
  uint32_t last_modify_time;
  uint32_t deletion_time;
  uint16_t group_id;
  uint16_t num_hard_links;
  uint32_t num_disk_sectors;
#define EXT2_INODE_F_SECURE_DELETION 0x1
#define EXT2_INODE_F_KEEP_COPY 0x2
#define EXT2_INODE_F_COMPRESSION 0x4
#define EXT2_INODE_F_SYNC_UPDATES 0x8
#define EXT2_INODE_F_IMMUTABLE 0x10
#define EXT2_INODE_F_APPEND_ONLY 0x20
#define EXT2_INODE_F_DUMP_EXCLUDE 0x40
#define EXT2_INODE_F_CONSTANT_LAST_ACCESSED 0x80
#define EXT2_INODE_F_HASH_INDEXED_DIR 0x10000
#define EXT2_INODE_F_AFS_DIR 0x20000
#define EXT2_INODE_F_JOURNAL_FILE_DATA 0x40000
  uint32_t flags;
  uint32_t os_specific;

#define EXT2_INODE_DBLOCKS_COUNT 12
  uint32_t dblocks[EXT2_INODE_DBLOCKS_COUNT];
#define EXT2_INODE_IDBLOCKS_COUNT(bsize) (bsize / 4)
  uint32_t single_idblock_ptr;
#define EXT2_INODE_DIDBLOCKS_COUNT(bsize) (bsize / 4)
  uint32_t double_idblock_ptr;
  uint32_t triple_idblock_ptr;
  uint32_t generation;
  uint32_t acl;
  uint32_t size_hi_dacl;
  uint32_t fragment_block_addr;
  unsigned char os_specific2[12];
} __pack;

struct ext2_dentry {
  uint32_t inode;
  uint16_t size;
  uint8_t name_length_lo;
#define EXT2_DENTRY_UNKNOWN 0
#define EXT2_DENTRY_FILE 1
#define EXT2_DENTRY_DIR 2
#define EXT2_DENTRY_CHARDEV 3
#define EXT2_DENTRY_BLOCKDEV 4
#define EXT2_DENTRY_FIFO 5
#define EXT2_DENTRY_SOCKET 6
#define EXT2_DENTRY_LINK_SOFT 7
  uint8_t type_name_length_hi;
  char name[];
} __pack;

void ext2_init();
