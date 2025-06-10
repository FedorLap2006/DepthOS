#include <depthos/poll.h>
#include <depthos/dev.h>
#include <depthos/list.h>
#include <depthos/proc.h>
#include <depthos/ringbuffer.h>
#include <depthos/file.h>
#include <depthos/heap.h>
#include <depthos/pipe.h>
#include <depthos/fs.h>
#include <depthos/string.h>


#define PIPE_SIZE 4096

struct open_pipe {
  devid_t device;
  inode_t inode;
  struct pipe pipe;
};

struct list open_pipes = {.first = NULL, .last = NULL, .length = 0};

struct pipe* pipe_open(devid_t device, inode_t inode) {
  list_foreach(&open_pipes, item) {
    struct open_pipe *op = list_item(item, struct open_pipe*);
    if (inode == op->inode)
      return &op->pipe;
  }
  klogf("pipe open new %ld, %ld", device, inode);
  struct open_pipe* op = (struct open_pipe*)kmalloc(sizeof(*op));
  op->inode = inode;
  op->device = device;
  op->pipe.buf = ringbuffer_create(PIPE_SIZE, 1);
  op->pipe.n_readers = 0;
  op->pipe.n_writers = 0;
  list_push(&open_pipes, (list_value_t)op);
  return &op->pipe;
}

int pipe_file_read(struct fs_node *file, char *buffer, size_t count, off_t *offset) {
  struct pipe* pipe = file->pipe;


  klogf("reading %d", pipe->n_writers);

  if (file->eof)
    return 0;

  while(pipe->buf->size == 0) // TODO: O_NONBLOCK 
      sched_yield();
    

  size_t i;

  for (i = 0; i < count; i++) {
    klogf("pipe size: %lu", pipe->buf->size);
    klogf("blocking");
    if (pipe->buf->size == 0)
      break;
   
    klogf("unblocked");
    
    char *e = (char*)ringbuffer_pop(pipe->buf);
    memcpy(buffer + i, e, pipe->buf->elem_size);
  }

  *offset += i;
  return i;
}

int pipe_file_write(struct fs_node *file, char *buffer, size_t count, off_t *offset) {
  struct pipe* pipe = file->pipe;
  task_dump_filetable(current_task->filetable);
  klogf("%s %s", file->name, file->path);
  klogf("writing %d", pipe->n_writers);
  klogf("blocking");
  while (pipe->buf->size + count > pipe->buf->max_size) // TODO: O_NONBLOCK
    sched_yield();
  klogf("unblocked");

  ringbuffer_pushn(pipe->buf, buffer, count);
  *offset += count;

  return count;
}

struct file_operations pipe_read_ops = {
  .read = pipe_file_read,
};

struct file_operations pipe_write_ops = {
  .write = pipe_file_read,
};

void pipe_file_close(struct fs_node* file) {
  klogf("file->pipe = %p",file->pipe);
  struct pipe* pipe = file->pipe;

  // TODO: properly use flags
  if (file->ops->read == pipe_file_read) pipe->n_readers--; 
  else if(file->ops->write == pipe_file_read) pipe->n_writers--;

  if (pipe->n_writers == 0) {
    file->eof = true;
  }
}

short pipe_file_poll(struct fs_node* file) {
  struct pipe* pipe = file->pipe;
  short revents = 0;
 
  if (pipe->buf->size > 0)
    revents |= POLLIN;

  if (pipe->buf->size < pipe->buf->max_size)
    revents |= POLLOUT;


  return revents;
}

struct fs_node* create_pipe_file(struct pipe *p, bool write) {
  struct fs_node* file = (struct fs_node*)kmalloc(sizeof(struct fs_node));
  file->fs = NULL;
  file->ops = write ? &pipe_write_ops : &pipe_read_ops;
  file->pipe = p;
  file->size = p->buf->size;

  if (write) p->n_writers++;
  else p->n_readers++;

  return file;
}


