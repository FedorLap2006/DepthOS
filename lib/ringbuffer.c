#include <depthos/ringbuffer.h>
#include <depthos/string.h>

struct ringbuffer *ringbuffer_create(size_t max_size) {
	struct ringbuffer *rb = kmalloc(sizeof(struct ringbuffer));
	if (!rb) return NULL;
	printk("hello");
	rb->data = kmalloc(sizeof(ringbuffer_elem_t) * max_size); // TODO: use void*
	rb->max_size = max_size;
	ringbuffer_init(rb, true);
	return rb;
}
void ringbuffer_init(struct ringbuffer *rb, bool zero) {
	rb->read_idx = rb->write_idx = 0;
	if (zero) memset(rb->data, 0, sizeof(ringbuffer_elem_t)*rb->max_size);
}

size_t ringbuffer_size(struct ringbuffer *rb) {
	return rb->write_idx - rb->read_idx;
}
bool ringbuffer_empty(struct ringbuffer *rb) {
	return ringbuffer_size(rb) == 0;
}

ringbuffer_elem_t *ringbuffer_push(struct ringbuffer *rb, ringbuffer_elem_t v) {
	ringbuffer_elem_t *e = &rb->data[rb->write_idx++];
	*e = v;
	rb->write_idx %= rb->max_size;
	return e;
}

ringbuffer_elem_t *ringbuffer_pop(struct ringbuffer *rb) {
	if (ringbuffer_empty(rb)) return NULL;
	ringbuffer_elem_t *e = &rb->data[rb->read_idx++];
	rb->read_idx %= rb->max_size;
	return e;
}
