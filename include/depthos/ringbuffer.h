#pragma once

#include <depthos/stdtypes.h>
#include <depthos/logging.h>

typedef struct ringbuffer {
	void* data;
  size_t elem_size;
	size_t max_size;
	size_t read_idx;
	size_t write_idx;
  size_t size;
}ringbuffer_t;

#define ringbuffer_at(rb, idx) ((rb)->data + (idx) * rb->elem_size)
#define ringbuffer_size(rb) ((rb)->size)
#define ringbuffer_empty(rb) (ringbuffer_size(rb) == 0)
#define ringbuffer_full(rb) (ringbuffer_size(rb) == (rb)->max_size)

struct ringbuffer *ringbuffer_create(size_t max_size, size_t elem_size);
void ringbuffer_init(struct ringbuffer *rb, bool zero);

void *ringbuffer_push(struct ringbuffer *rb, void *v);
static inline void ringbuffer_pushn(struct ringbuffer *rb, void *data, size_t n) {
  void *end = data + n * rb->elem_size;
  while (data < end) {
    ringbuffer_push(rb, data);
    data += rb->elem_size;
  }
}

void *ringbuffer_pop(struct ringbuffer *rb);
