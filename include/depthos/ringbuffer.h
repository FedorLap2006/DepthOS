#pragma once

#include <depthos/stdtypes.h>

typedef uint64_t ringbuffer_elem_t;
typedef struct ringbuffer {
	ringbuffer_elem_t *data;
	size_t max_size;
	size_t read_idx;
	size_t write_idx;
}ringbuffer_t;

struct ringbuffer *ringbuffer_create(size_t max_size);
void ringbuffer_init(struct ringbuffer *rb, bool zero);
size_t ringbuffer_size(struct ringbuffer *rb);
bool ringbuffer_empty(struct ringbuffer *rb);

ringbuffer_elem_t *ringbuffer_push(struct ringbuffer *rb, ringbuffer_elem_t v);
ringbuffer_elem_t *ringbuffer_pop(struct ringbuffer *rb);
