#include <depthos/bitmap.h>
#include <depthos/errno.h>
#include <depthos/heap.h>
#include <depthos/logging.h>
#include <depthos/string.h>

static inline size_t bitmap_elem_offset(size_t idx) {
  return idx % bitmap_elem_size();
}
static inline uint8_t bitmap_elem_mask(size_t idx) {
  return 1 << bitmap_elem_offset(idx);
}

void bitmap_init(bitmap_t *bm) { memset(bm->bits, 0, bitmap_byte_size(bm)); }

bitmap_t *bitmap_create(size_t c) {
  bitmap_t *bitmap = kmalloc(sizeof(bitmap_t));
  bitmap->bits = kmalloc(bitmap_elem_index(c - 1) + 1);
  bitmap->size = c;
  return bitmap;
}

static inline bool bitmap_in_bounds(bitmap_t *bm, size_t idx) {
  return idx < bm->size;
}

void bitmap_mark(bitmap_t *bm, size_t i) {
  if (!bitmap_in_bounds(bm, i))
    return;
  bm->bits[bitmap_elem_index(i)] |= bitmap_elem_mask(i);
}

void bitmap_reset(bitmap_t *bm, size_t i) {
  if (!bitmap_in_bounds(bm, i))
    return;
  bm->bits[bitmap_elem_index(i)] &= ~bitmap_elem_mask(i);
}
void bitmap_flip(bitmap_t *bm, size_t i) {
  if (!bitmap_in_bounds(bm, i))
    return;
  bm->bits[bitmap_elem_index(i)] ^= bitmap_elem_mask(i);
}

bool bitmap_test(const bitmap_t *bm, size_t i) {
  if (!bitmap_in_bounds(bm, i))
    return 0;
  return (bm->bits[bitmap_elem_index(i)] & bitmap_elem_mask(i)) != 0;
}

void bitmap_set(bitmap_t *bm, bool v, size_t idx) {
  v &= 0x1;
  if (v)
    bitmap_mark(bm, idx);
  else
    bitmap_reset(bm, idx);
}

void bitmap_setmulti(bitmap_t *bm, bool v, size_t idx, size_t c) {
  for (int i = 0; i < bm->size - idx && i < c; i++)
    bitmap_set(bm, v, idx + i);
}

ssize_t bitmap_scan(bitmap_t *bm, size_t s, size_t n, bool v) {
  size_t r, rc = 0;

  for (size_t i = 0; i < bm->size; i++) {
    if (bitmap_test(bm, s + i) != v) {
      rc = 0;
      continue;
    }
    if (!rc)
      r = i;
    if (++rc == n)
      return r;
  }
  return -EBITMAP;
}

ssize_t bitmap_scan_flip(bitmap_t *bm, size_t s, size_t n, bool v) {
  size_t idx = bitmap_scan(bm, s, n, v);
  if (idx < 0) {
    return idx;
  }

  for (int i = 0; i < n; i++)
    bitmap_flip(bm, idx + i);

  return idx;
}

void bitmap_dump(const bitmap_t *bm, size_t s, size_t e) {
  if (e < 0 || e > bm->size)
    e = bm->size;
  for (int i = s; i < e; i++) {
    printk("%d", bitmap_test(bm, i));
  }
  printk("\n");
}

void bitmap_dump_pretty(const bitmap_t *bm, size_t s, size_t e) {
  if (e < 0 || e > bm->size)
    e = bm->size;
  for (int i = s; i < e; i++) {
    printk("%d", bitmap_test(bm, i));
    if ((i + 1) % 8 == 0 && i < bm->size - 1)
      printk(" ");
  }
  printk("\n");
}

void bitmap_dump_compact(const bitmap_t *bm, size_t s, size_t e, size_t width) {
  if (e < 0)
    e = bm->size - e;
  if (s >= bm->size)
    return;
  if (e >= bm->size)
    e = bm->size - 1;
  width /= 2;
  size_t tmp = e;
  e /= bitmap_elem_size();
  printk("%-5lu ", 0 * bitmap_elem_size());
  for (size_t i = s / bitmap_elem_size(); i <= e; i++) {
    printk("%x", bm->bits[i] & 0xf);
    printk("%x", bm->bits[i] >> 4);
    if (i == e) {
      break;
    }
    if (i && (i + 1) % width == 0) {
      printk("\n");
      printk("%-5lu ", (i + 1) * bitmap_elem_size());
    } else if (i < e)
      printk(" ");
  }
  printk("\n\n");
}
