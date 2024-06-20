#pragma once

#include "depthos/file.h"
#include "depthos/ringbuffer.h"
#include <depthos/stdtypes.h>

struct pipe {
  struct ringbuffer* buf;
  int n_writers;
  int n_readers;
};


struct fs_node* create_pipe_file(struct pipe *p, bool write);
struct pipe* pipe_open(devid_t device, inode_t inode);


int pipe_file_read(struct fs_node *file, char *buffer, size_t count, off_t *offset);
int pipe_file_write(struct fs_node *file, char *buffer, size_t count, off_t *offset);
void pipe_file_close(struct fs_node *file);
