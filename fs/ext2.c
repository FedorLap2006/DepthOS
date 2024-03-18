#include <depthos/assert.h>
#include <depthos/errno.h>
#include <depthos/ext2.h>
#include <depthos/file.h>
#include <depthos/fs.h>
#include <depthos/heap.h>
#include <depthos/idt.h>
#include <depthos/kconfig.h>
#include <depthos/logging.h>
#include <depthos/math.h>
#include <depthos/string.h>
#include <depthos/vmm.h>

struct ext2_private {
  struct ext2_superblock *super;
  struct ext2_blk_group_descriptor *bgdt;
  struct ext2_inode *root;
  off_t bgdt_block;
  size_t num_block_groups;
  size_t block_size, fragment_size;
};
struct ext2_file_private {
  uint32_t inode;
  struct ext2_inode *cached;
};

#if CONFIG_EXT2_LOG_ENABLE == 1
#define ext2_log(...) klogf(__VA_ARGS__)
#else
// #define ext2_log(...) iowait(5000000)
#define ext2_log(...)
#endif

#define IMPL(fs) ((struct ext2_private *)fs->impl)

int ext2_read_block(struct filesystem *fs, void *buf, off_t block) {
  off_t sector = (block * IMPL(fs)->block_size) / fs->dev->block_size;
  uint32_t n_sectors = ceil_div(IMPL(fs)->block_size, fs->dev->block_size);
  // ext2_log("attempting to read %ld block (%ld, (%ld * %ld) / %ld)", block,
  //          sector, block, IMPL(fs)->block_size, fs->dev->block_size);
  int read_sectors = fs->dev->read(fs->dev, buf, n_sectors, &sector);
  // ext2_log("read %d sectors", read_sectors);
  // ext2_log("read (%d / %ld) blocks", read_sectors,
  //          ceil_div(IMPL(fs)->block_size, fs->dev->block_size));
  return read_sectors == n_sectors;
}
struct ext2_inode *ext2_get_inode(struct filesystem *fs, uint32_t inode) {
  inode--;
  uint32_t group_idx = inode / IMPL(fs)->super->inodes_per_blk_group;
  struct ext2_blk_group_descriptor group = IMPL(fs)->bgdt[group_idx];

  uint32_t index = inode % IMPL(fs)->super->inodes_per_blk_group;
  // uint32_t block = (index % IMPL(fs)->super->inode_size) /
  // IMPL(fs)->block_size;
  ext2_log("found %lu inode in %ld group [%ld]", inode + 1, group_idx, index);
  off_t block = group.inode_table_addr +
                (index * IMPL(fs)->super->inode_size) / IMPL(fs)->block_size;
  off_t offset = (index * IMPL(fs)->super->inode_size) % IMPL(fs)->block_size;
  ext2_log("block: %lx offset=%lx", block, offset);
  off_t lba = (block * IMPL(fs)->block_size + offset) / fs->dev->block_size;
  // off_t lba =
  //     (((group.inode_table_addr +
  //        (index * IMPL(fs)->super->inode_size) / IMPL(fs)->block_size)) *
  //          IMPL(fs)->block_size +
  //      (index * IMPL(fs)->super->inode_size) % IMPL(fs)->block_size) /
  //     fs->dev->block_size;

  size_t sectors = ceil_div(IMPL(fs)->super->inode_size, fs->dev->block_size);
  char *buf = kmalloc(sectors * fs->dev->block_size);
  ext2_log("allocating buf with size of %d: %p", sectors * fs->dev->block_size,
           buf);
  memset(buf, 0, sectors * fs->dev->block_size);
  struct ext2_inode *inode_data = kmalloc(sizeof(struct ext2_inode));
  if (!inode_data) {
    ext2_log("no memory");
    return NULL;
  }

  int read_sectors = fs->dev->read(fs->dev, buf, sectors, &lba);
  ext2_log("read %d sectors", read_sectors);
  memcpy(inode_data, buf, sizeof(struct ext2_inode));
  ext2_log("inode: {type=%hx sectors=%ld flags=%lx blocks[0]=%lx size=%lx}",
           inode_data->type_permissions, inode_data->num_disk_sectors,
           inode_data->flags, inode_data->dblocks[0], inode_data->size_lo);
  // ext2_log("lba: %lx", lba);
  return inode_data;
}
off_t ext2_inode_size(struct ext2_inode *inode) {
  // TODO: size_hi
  // klogf("%ld %ld", inode->size_lo, inode->size_hi_dacl);
  return inode->size_lo;
}

struct ext2_sync_params {
  void *buf;
  size_t nbytes;
  off_t offset;
  bool write;
};

size_t ext2_sync_direct_blocks(struct filesystem *fs, uint32_t *blocks,
                               int nblocks, off_t offset, void *buf,
                               size_t nbytes, bool write) {
  size_t r = 0;
  char *blockbuf = kmalloc(IMPL(fs)->block_size);
  soff_t block_offset = offset / IMPL(fs)->block_size;
  off_t byte_offset = offset % IMPL(fs)->block_size;
  ext2_log("syncing blocks (table=%p offset=%ld nbytes: %ld nblocks: %d)",
           blocks, offset, nbytes, nblocks);
  for (off_t i = block_offset; i < nblocks && r <= nbytes; i++) {
    if (!blocks[i]) {
      ext2_log("no blocks to read, %ld", i);
      break;
      // goto ret;
    }

    if (write) {
    } else {
      ext2_read_block(fs, blockbuf, blocks[i]);
      size_t bytes_to_read =
          MIN(nbytes - r, IMPL(fs)->block_size - byte_offset);
      memcpy(buf + r, blockbuf + byte_offset,
             bytes_to_read); // TODO: - bytes_offset?, we DO go out of bounds,
                             // on block buf
      r += bytes_to_read;
    }
    byte_offset = 0;
  }
  // ret:;
  kfree(blockbuf, IMPL(fs)->block_size);
  ext2_log("synced %ld (%d blocks) / %ld (%d blocks) bytes with %p", r,
           ceil_div(r, IMPL(fs)->block_size), nbytes,
           ceil_div(nbytes, IMPL(fs)->block_size), buf);
  return r;
}

size_t ext2_sync_indirect_blocks(struct filesystem *fs, uint32_t pointer_block,
                                 off_t offset, void *buf, size_t nbytes,
                                 bool write) {
  uint32_t *idbuf = (uint32_t *)kmalloc(IMPL(fs)->block_size);
  ext2_read_block(fs, idbuf, pointer_block);
  ext2_log("syncing indirect blocks (pointer=%ld offset=%ld nbytes=%ld)",
           pointer_block, offset, nbytes);
  return ext2_sync_direct_blocks(
      fs, idbuf, EXT2_INODE_IDBLOCKS_COUNT(IMPL(fs)->block_size), offset, buf,
      nbytes, write);
}

size_t ext2_sync_doubly_indirect_blocks(struct filesystem *fs,
                                        uint32_t pointer_block, off_t offset,
                                        void *buf, size_t nbytes, bool write) {
  uint32_t *idbuf = (uint32_t *)kmalloc(IMPL(fs)->block_size);
  ext2_read_block(fs, idbuf, pointer_block);
  size_t block_offset = offset / IMPL(fs)->block_size;
  size_t idx = block_offset / EXT2_INODE_IDBLOCKS_COUNT(IMPL(fs)->block_size);
  // size_t nblocks = MIN((size_t)ceil_div(nbytes, IMPL(fs)->block_size),
  //                      EXT2_INODE_IDBLOCKS_COUNT(IMPL(fs)->block_size)); //
  //                      THIS IS NOT TO SCALE
  // size_t nblocks =
  //     MIN((size_t)ceil_div(nbytes,
  //                          IMPL(fs)->block_size *
  //                              EXT2_INODE_IDBLOCKS_COUNT(IMPL(fs)->block_size)),
  //         EXT2_INODE_IDBLOCKS_COUNT(IMPL(fs)->block_size) -
  //             idx); // TIS WRONG, JUST PUT i = block_offset; i < ...
  ext2_log("syncing doubly indirect blocks (pointer=%ld offest=%ld nbytes=%ld)",
           pointer_block, offset, nbytes);
  ext2_log(
      "nbytes=%ld block_size=%ld nblocks=%d ndid_blocks=%d did_block_size=%ld "
      "max=%ld "
      "(max-offset)=%ld ",
      nbytes, IMPL(fs)->block_size, ceil_div(nbytes, IMPL(fs)->block_size),
      ceil_div(nbytes, IMPL(fs)->block_size *
                           EXT2_INODE_IDBLOCKS_COUNT(IMPL(fs)->block_size)),
      EXT2_INODE_IDBLOCKS_COUNT(IMPL(fs)->block_size),
      EXT2_INODE_DIDBLOCKS_COUNT(IMPL(fs)->block_size),
      EXT2_INODE_IDBLOCKS_COUNT(IMPL(fs)->block_size) - idx);
  ext2_log("idx: %ld offset: %ld total size: %ld actual limit: %ld block: %ld",
           idx, block_offset,
           EXT2_INODE_DIDBLOCKS_COUNT(IMPL(fs)->block_size) *
               EXT2_INODE_IDBLOCKS_COUNT(IMPL(fs)->block_size),
           IMPL(fs)->block_size / 4, IMPL(fs)->block_size);

  offset -= idx * EXT2_INODE_IDBLOCKS_COUNT(IMPL(fs)->block_size) *
            IMPL(fs)->block_size;
  size_t r = 0;

  // XXX: implementation is recent, so might be prune to some cases I haven't
  // thought of, needs testing.

  for (uint32_t i = idx;
       i < EXT2_INODE_DIDBLOCKS_COUNT(IMPL(fs)->block_size) && nbytes > 0;
       i++) {
    if (!idbuf[idx]) {
      ext2_log("no pointers to read, %ld", i);
      break;
      // goto ret;
    }
    size_t dr =
        ext2_sync_indirect_blocks(fs, idbuf[i], offset, buf, nbytes, write);
    r += dr;
    nbytes -= dr;
    buf += dr;
    offset = 0;
  }
  ext2_log("doubly indirect sync finished: done=%ld remaining=%ld total=%ld", r,
           nbytes, r + nbytes);
  return r;
  // XXX: Do we just sync one...?

  // return ext2_sync_indirect_blocks(
  //     fs, idbuf[idx],
  //     offset - idx * EXT2_INODE_IDBLOCKS_COUNT(IMPL(fs)->block_size) *
  //                  IMPL(fs)->block_size,
  //     buf, nbytes, write);
}

size_t ext2_sync_inode_data(struct filesystem *fs, struct ext2_inode *inode,
                            off_t offset, void *buf, size_t nbytes,
                            bool write) {
  size_t r = 0, dr = 0;
  size_t block_offset = offset / IMPL(fs)->block_size;
  ext2_log("syncing inode data");
  ext2_log("offset=%ld nbytes=%ld buffer=%p", offset, nbytes, buf);
  ext2_log("inode: size=%ld indirect=%d double_indirect=%d triple_indirect=%d",
           ext2_inode_size(inode), inode->single_idblock_ptr != 0,
           inode->double_idblock_ptr != 0, inode->triple_idblock_ptr != 0);
  ext2_log("offset: %ld/%ld (direct=%d indirect=%ld double_indirect=%ld "
           "triple_indirect=%d)",
           offset, block_offset, EXT2_INODE_DBLOCKS_COUNT,
           EXT2_INODE_DBLOCKS_COUNT +
               EXT2_INODE_IDBLOCKS_COUNT(IMPL(fs)->block_size),
           EXT2_INODE_DBLOCKS_COUNT +
               EXT2_INODE_IDBLOCKS_COUNT(IMPL(fs)->block_size) +
               EXT2_INODE_DIDBLOCKS_COUNT(IMPL(fs)->block_size) *
                   EXT2_INODE_IDBLOCKS_COUNT(IMPL(fs)->block_size),
           0);

  ext2_log("size: lo=%ld (%d blocks) hi_dacl=%ld (%d blocks)", inode->size_lo,
           ceil_div(inode->size_lo, IMPL(fs)->block_size), inode->size_hi_dacl,
           ceil_div(inode->size_hi_dacl, IMPL(fs)->block_size));

  if (!write &&
      offset >= ext2_inode_size(
                    inode)) { // XXX: how does write work on out-of-bounds?
    return 0;
  }
  if (offset + nbytes > ext2_inode_size(inode)) {
    ext2_log(
        "offset (%ld) + nbytes (%ld) are greater file size (%ld), reducing "
        "to %ld",
        offset, nbytes, ext2_inode_size(inode), inode->size_lo - offset);
    nbytes = ext2_inode_size(inode) - offset;
  }

  if (block_offset < EXT2_INODE_DBLOCKS_COUNT) {
    r = ext2_sync_direct_blocks(fs, inode->dblocks, EXT2_INODE_DBLOCKS_COUNT,
                                offset, buf, nbytes, write);
    ext2_log("read %ld bytes", r);
    if (r == nbytes) {
      return r;
    }
    offset = 0;
    buf += r;
    nbytes -= r;
  } else {
    offset -= EXT2_INODE_DBLOCKS_COUNT * IMPL(fs)->block_size;
  }

  if (block_offset < EXT2_INODE_DBLOCKS_COUNT +
                         EXT2_INODE_IDBLOCKS_COUNT(IMPL(fs)->block_size) &&
      inode->single_idblock_ptr) {
    ext2_log("indirect blocks: offset=%ld nbytes=%ld bytes_read=%ld", offset,
             nbytes, r);
    // ext2_log("indirect blocks: buffer offset: %ld", r);

    dr = ext2_sync_indirect_blocks(fs, inode->single_idblock_ptr, offset, buf,
                                   nbytes, write);
    ext2_log("indirect blocks: read %ld bytes", dr);
    r += dr;

    if (r == nbytes) {
      return r;
    }
    offset = 0;
    buf += dr;
    nbytes -= dr;
  } else {
    offset -=
        EXT2_INODE_IDBLOCKS_COUNT(IMPL(fs)->block_size) * IMPL(fs)->block_size;
  }

  if (block_offset < EXT2_INODE_DBLOCKS_COUNT +
                         EXT2_INODE_IDBLOCKS_COUNT(IMPL(fs)->block_size) +
                         EXT2_INODE_DIDBLOCKS_COUNT(IMPL(fs)->block_size) *
                             EXT2_INODE_IDBLOCKS_COUNT(IMPL(fs)->block_size) &&
      inode->double_idblock_ptr) {
    ext2_log("doubly indirect blocks: offset=%ld nbytes=%ld bytes_read=%ld",
             offset, nbytes, r);

    dr = ext2_sync_doubly_indirect_blocks(fs, inode->double_idblock_ptr, offset,
                                          buf, nbytes, write);
    ext2_log("indirect blocks: read %ld bytes", dr);
    r += dr;
    if (r == nbytes) {
      return r;
    }
    offset = 0;
    buf += dr;
    nbytes -= dr;
  }

  if (nbytes)
    klogf("read finished: done=%ld remaining=%ld", r, nbytes);
  return r;
}

#define FILE_IMPL(file) ((struct ext2_file_private *)file->impl)
int ext2_read(struct fs_node *file, char *buf, size_t n, off_t *offset) {
  // ext2_log("file size: %d", FILE_IMPL(file)->cached->size_lo);
  // ext2_log("offset: %ld bytes: %ld", *offset, n);
  // ext2_log("size: lo=%ld hi_dacl=%ld", FILE_IMPL(file)->cached->size_lo,
  //          FILE_IMPL(file)->cached->size_hi_dacl);
  // if (*offset + n > FILE_IMPL(file)->cached->size_lo) { // TODO: size_hi
  //   ext2_log(
  //       "offset (%ld) + n (%ld) are greater file size (%ld), reducing n to
  //       %ld", *offset, n, FILE_IMPL(file)->cached->size_lo,
  //       FILE_IMPL(file)->cached->size_lo - *offset);
  //   n = FILE_IMPL(file)->cached->size_lo - *offset;
  // }

  size_t r = ext2_sync_inode_data(file->fs, FILE_IMPL(file)->cached, *offset,
                                  buf, n, false);
  if (*offset + n == ext2_inode_size(FILE_IMPL(file)->cached)) {
    file->eof = true;
  }

  *offset += r;
  return r;
}

int ext2_write(struct fs_node *file, char *buf, size_t n, off_t *offset) {
  return 0;
}

int ext2_iter(struct fs_node *file, struct dentry *dst, size_t max_name_len,
              off_t *offset) {
  // TODO: refactor
  struct ext2_dentry *raw_dentry =
      (struct ext2_dentry *)kmalloc(sizeof(struct ext2_dentry));
  dst->offset = *offset;
  off_t tmpoff = *offset;
  ext2_log("reading into %p", raw_dentry);
  unsigned int r =
      ext2_read(file, (char *)raw_dentry, sizeof(*raw_dentry), &tmpoff);
  if (r < sizeof(*raw_dentry)) {
    ext2_log("WARNING: ext2 dentry read from disk is smaller than miminal size "
             "(%d < %d)",
             r, sizeof(*raw_dentry));
    return 0;
  }

  size_t len =
      MIN(max_name_len - 1, raw_dentry->name_length_lo); // TODO: high length

  ext2_log("reading into %p", dst->name);
  unsigned int nr =
      ext2_read(file, dst->name, len,
                &tmpoff); // TODO: filler func to not over-allocate
  ext2_log("read %d", nr);
  dst->name[nr] = 0;
  dst->inode = raw_dentry->inode;
  dst->type = raw_dentry->type_name_length_hi;
  ext2_log("entry name is %s", dst->name);
  // kfree(raw_dentry, sizeof(struct ext2_dentry));

  ext2_log("offset: %d %d", dst->offset, *offset);
  *offset += raw_dentry->size;
  return r + nr; // TODO: do we actually want to expose the actual
                 // size of the dentry to userland?
}

struct vm_area_ops ext2_mmap_ops = {
    .load_page = generic_file_load_mapped_page,
};

int ext2_mmap(struct fs_node *file, struct vm_area *area) {
  area->ops = &ext2_mmap_ops;
  return 0;
}

soff_t ext2_seek(struct fs_node *file, soff_t pos, int whence) {
  return generic_file_seek_size(
      file, pos, ext2_inode_size(FILE_IMPL(file)->cached) - 1, whence);
}

struct file_operations ext2_file_ops = (struct file_operations){
    .read = ext2_read,
    .write = ext2_write,
    .seek = ext2_seek,
    .iter = ext2_iter,
    .mmap = ext2_mmap,
};

// TODO: file descriptor
// int ext2_readdir(struct file *file, struct dentry *dbuf) {}

// int ext2_write(struct file *file, )
// int ext2_read_dentries(struct filesystem *fs, struct ext2_inode *inode,
//                        struct dentry *dentries, size_t n) {

int ext2_read_dentries(struct filesystem *fs, struct ext2_inode *inode,
                       struct dentry *dentries, size_t n) {
  char *blkbuf = kmalloc(IMPL(fs)->block_size);
  assert(blkbuf != NULL);
  memset(blkbuf, 0, IMPL(fs)->block_size);
  off_t offset = 0;
  int dir_bytes_read =
      ext2_sync_inode_data(fs, inode, 0, blkbuf, IMPL(fs)->block_size, false);
  struct ext2_dentry *raw_dentries = (struct ext2_dentry *)blkbuf;

  size_t j = 0;
  ext2_log("block size=%d dentry_size=%d name_length=%d j=%d n=%d",
           IMPL(fs)->block_size, raw_dentries->size,
           raw_dentries->name_length_lo, j, n);
  for (size_t i = 0; i < IMPL(fs)->block_size && j < n &&
                     (raw_dentries->size || raw_dentries->name_length_lo);
       i += raw_dentries->size, j++) {
    raw_dentries = (struct ext2_dentry *)(blkbuf + i);
    if (!raw_dentries->inode) {
      ext2_log("inode is zero");
      continue;
    }
    if (!raw_dentries->size) {
      ext2_log("uh, size is zero");
      break;
    }
    ext2_log("debug 1, length=%d", raw_dentries->name_length_lo);
    char *name =
        kmalloc(raw_dentries->name_length_lo + 1); // TODO: name length high
    memcpy(name, raw_dentries->name, raw_dentries->name_length_lo);
    name[raw_dentries->name_length_lo] = 0;
    ext2_log("name: %p %s", name, name);
    dentries[j] = (struct dentry){
        .inode = raw_dentries->inode,
        .type = raw_dentries->type_name_length_hi, // TODO: lookup length
                                                   // feature in superblock
        .name = name,
        .offset = i,

    };
    ext2_log("raw dentry[%d]: %s (%d) total=%d inode=%x", j, dentries[j].name,
             raw_dentries->name_length_lo, raw_dentries->size,
             raw_dentries->inode);
  }

  return j;
}

struct fs_node *ext2_open(struct filesystem *fs, const char *path) {
  char pfrag[100][256];
  int f = 0;
  int n_pfrag = strsplt(path, &pfrag[0][0], 256, 100, '/');
  struct ext2_inode *curr = IMPL(fs)->root;
  uint32_t icurr = 2;
  struct dentry *dentry_buffer = kmalloc(sizeof(struct dentry) * 1024);
  ext2_log("opening %s", path);
  for (int i = path[0] == '/' ? 1 : 0; i < n_pfrag; i++) {
    if (curr->type_permissions & EXT2_INODE_T_DIR == 0) {
      ext2_log("ignoring non-dir");
      continue;
    }
    if (strlen(pfrag[i]) == 0)
      continue;
    int n = ext2_read_dentries(fs, curr, dentry_buffer, 1024);
    ext2_log("%d entries", n);
    for (int j = 0; j < n; j++) {
      ext2_log("fragment='%s' dentry='%s' inode=%ld cmp=%d", pfrag[i],
               dentry_buffer[j].name, dentry_buffer[j].inode,
               strcmp(pfrag[i], dentry_buffer[j].name));
      if (strcmp(pfrag[i], dentry_buffer[j].name) == 0) {
        curr = ext2_get_inode(fs, dentry_buffer[j].inode);
        icurr = dentry_buffer[j].inode;
        goto outer;
      }
    }
    kfree(dentry_buffer, sizeof(struct dentry) * 1024);
    return NULL;

  outer:;
  }

  kfree(dentry_buffer,
        sizeof(struct dentry) *
            1024); // FIXME: we remove it, and kmalloc crashes.

  ext2_log("type: %x size: %x", curr->type_permissions, curr->size_lo);

  struct fs_node *node = kmalloc(sizeof(struct fs_node));
  memset(node, 0, sizeof(struct fs_node));
  node->type = curr->type_permissions & EXT2_INODE_T_DIR
                   ? FS_DIR
                   : FS_FILE; // TODO: other fs types
  node->ops = &ext2_file_ops;
  node->impl = kmalloc(sizeof(struct ext2_file_private));
  ext2_log("icurr=%d", icurr);
  struct ext2_inode *inode = ext2_get_inode(fs, icurr);
  FILE_IMPL(node)->cached = inode;
  FILE_IMPL(node)->inode = icurr;
  node->size = ext2_inode_size(inode);

  return node;
}

struct filesystem *ext2_mount(struct device *dev);

struct fs_operations ext2_fs = (struct fs_operations){
    .name = "ext2",
    .mount = ext2_mount,
    .open = ext2_open,
};

struct filesystem *ext2_mount(struct device *dev) {
  ext2_log("reading superblock...");
  struct ext2_superblock *super = kmalloc(sizeof(struct ext2_superblock));
  off_t off = 2;
  dev->read(dev, super, 2, &off);

  if (super->signature != EXT2_SIGNATURE) {
    panicf("invalid ext2 signature: %x", super->signature);
    return NULL;
  }
  ext2_log(
      "super block: {magic=%x v=%d.%d blks=%d blklog=%d super=%d inode_sz=%d}",
      super->signature, super->version_major, super->version_minor,
      super->total_blocks, super->log_block_size, super->superblock_blk_num,
      super->inode_size);

  struct ext2_private *priv = kmalloc(sizeof(struct ext2_private));
  priv->super = super;
  priv->block_size = 1024 << super->log_block_size;
  ext2_log("block size: %d", priv->block_size);
  priv->bgdt_block = priv->block_size == 1024 ? 2 : 1;
  off = (priv->bgdt_block * priv->block_size) / dev->block_size;

  priv->num_block_groups =
      ceil_div(super->total_blocks, super->blocks_per_blk_group);
  ext2_log("block groups: %d", priv->num_block_groups);

  size_t bgdt_size =
      sizeof(struct ext2_blk_group_descriptor) * priv->num_block_groups;
  size_t bgdt_sectors = ceil_div(bgdt_size, dev->block_size);
  char *bgdt_buf = kmalloc(bgdt_sectors * dev->block_size);

  struct ext2_blk_group_descriptor *bgdt = kmalloc(bgdt_size);

  dev->read(dev, bgdt_buf, bgdt_sectors, &off);

  memcpy(bgdt, bgdt_buf, bgdt_size);
  kfree(bgdt_buf, bgdt_sectors * dev->block_size);

  for (size_t i = 0; i < priv->num_block_groups; i++) {
    ext2_log("bgdt[%d]: usage_block=%d usage_inode=%d inode=%d", i,
             bgdt[i].usage_block_addr, bgdt[i].usage_inode_addr,
             bgdt[i].inode_table_addr);
  }
  priv->bgdt = bgdt;

  struct filesystem *fs = kmalloc(sizeof(struct filesystem));
  fs->impl = priv;
  fs->ops = &ext2_fs;
  fs->dev = dev;
  ext2_get_inode(fs, EXT2_ROOT_INODE);
  struct ext2_inode *root_inode = ext2_get_inode(fs, EXT2_ROOT_INODE);
  priv->root = root_inode;
  return fs;
}

void ext2_init() { vfs_register(&ext2_fs); }
