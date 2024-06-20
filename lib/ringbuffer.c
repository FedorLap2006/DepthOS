#include <depthos/heap.h>
#include <depthos/ringbuffer.h>
#include <depthos/string.h>

struct ringbuffer *ringbuffer_create(size_t max_size, size_t elem_size) {
  struct ringbuffer *rb = (struct ringbuffer *)kmalloc(sizeof(struct ringbuffer));
  if (!rb) return NULL;
  // printk("hello");
  rb->data = kmalloc(elem_size * max_size);
  rb->max_size = max_size;
  rb->elem_size = elem_size;
  ringbuffer_init(rb, true);
  return rb;
}

void ringbuffer_init(struct ringbuffer *rb, bool zero) {
  rb->read_idx = rb->write_idx = 0;
  rb->size = 0;
  if (rb->elem_size == 0)
    rb->elem_size = 1;
  if (zero) memset(rb->data, 0, rb->elem_size * rb->max_size);
}

void *ringbuffer_push(struct ringbuffer *rb, void *v) {
  void *e = ringbuffer_at(rb, rb->write_idx++);
  memcpy(e, v, rb->elem_size);
  rb->write_idx %= rb->max_size;
  rb->size++;
  rb->size %= rb->max_size + 1;
  return e;
}

void *ringbuffer_pop(struct ringbuffer *rb) {
  if (ringbuffer_empty(rb)) return NULL;
  void *e = ringbuffer_at(rb, rb->read_idx++);
  rb->read_idx %= rb->max_size;
  rb->size--;
  return e;
}
