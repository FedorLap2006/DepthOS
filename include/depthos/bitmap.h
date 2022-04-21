#pragma once

#include <depthos/stdtypes.h>

#define EBITMAP 1

typedef uint8_t bitmap_elem_t;

typedef struct bitmap {
  bitmap_elem_t *bits;
  size_t size;
} bitmap_t;

/**
 * @brief Reset given bitmap
 *
 * @param bm Bitmap to initialise
 */
void bitmap_init(bitmap_t *bm);
/**
 * @brief Allocate new bitmap
 *
 * @param n Bitmap size (in bits)
 * @return Created bitmap
 */
bitmap_t *bitmap_create(size_t n);
/**
 * @brief Mark (set to 1) bit at the specified index
 *
 * @param bm Bitmap to mark bit in.
 * @param idx Index where bit is located
 */
void bitmap_mark(bitmap_t *bm, size_t idx);
/**
 * @brief Reset (set to 0) bit at the specified index
 *
 * @param bm Bitmap to reset bit in
 * @param idx Index where bit is located
 */
void bitmap_reset(bitmap_t *bm, size_t idx);
/**
 * @brief Set bit at the specified index to a given value
 *
 * @param bm Bitmap to set bit in
 * @param v Value to set bit to
 * @param idx Index where bit is located
 */
void bitmap_set(bitmap_t *bm, bool v, size_t idx);
/**
 * @brief Flip (0 => 1, 1 => 0) bit at the specified index
 *
 * @param bm Bitmap to flip bit in
 * @param idx Index where bit is located
 */
void bitmap_flip(bitmap_t *bm, size_t idx);
/**
 * @brief Get value of bit located at the specified index
 *
 * @param bm Bitmap to get bit from
 * @param i Index where bit is located
 */
bool bitmap_test(const bitmap_t *bm, size_t i);
/**
 * @brief Set multiple bits to a given value
 *
 * @param bm Bitmap to set bits in
 * @param v Value to set bits to
 * @param idx Starting index
 * @param n Amount of bits to set
 */
void bitmap_setmulti(bitmap_t *bm, bool v, size_t idx, size_t n);
/**
 * @brief Scan bitmap for a given value
 *
 * @param bm Bitmap to scan bits from
 * @param s Starting index
 * @param n Amount of bits to scan
 * @param v Value to scan bits for
 * @return Index of the first bit, otherwise -EBITMAP
 */
size_t bitmap_scan(bitmap_t *bm, size_t s, size_t n, bool v);
/**
 * @brief Scan the bitmap for a given value and flip bits afterwards
 *
 * @param bm Bitmap to scan and flip bits in
 * @param s Starting index
 * @param n Amount of bits to scan and flip
 * @param v Value to scan bits for
 * @return Index of the first bit, otherwise -EBITMAP
 */
size_t bitmap_scan_flip(bitmap_t *bm, size_t s, size_t n, bool v);
/**
 * @brief Print bitmap to the console
 *
 * @param bm Bitmap to print
 * @param s Starting index
 * @param e Ending index (inclusive)
 */
void bitmap_dump(const bitmap_t *bm, size_t s, size_t e);
/**
 * @brief Pretty-print bitmap to the console
 *
 * @param bm Bitmap to print
 * @param s Starting index
 * @param e Ending index (inclusive)
 */
void bitmap_dump_pretty(const bitmap_t *bm, size_t s, size_t e);
/**
 * @brief Print bitmap in compact (byte) format
 *
 * @param bm Bitmap to print
 * @param s Starting index. Must be a multiple of bitmap_elem_size
 * @param e Ending index (inclusive). Must be a multiple of bitmap_elem size
 * @param width Amount of bytes in a single line
 */
void bitmap_dump_compact(const bitmap_t *bm, size_t s, size_t e, size_t width);

static inline size_t bitmap_elem_size() { return sizeof(bitmap_elem_t) * 8; }
static inline size_t bitmap_size(const bitmap_t *bm) { return bm->size; }
static inline size_t bitmap_elem_index(size_t idx) {
  return idx / bitmap_elem_size();
}
static inline size_t bitmap_byte_size(const bitmap_t *bm) {
  return bitmap_elem_index(bitmap_size(bm) - 1) + 1;
}
