#include "depthos/poll.h"
#include "depthos/tools.h"
#include <depthos/assert.h>
#include <depthos/errno.h>
#include <depthos/file.h>
#include <depthos/fs.h>
#include <depthos/heap.h>
#include <depthos/list.h>
#include <depthos/proc.h>
#include <depthos/socket.h>
#include <depthos/string.h>

struct bound_socket {
  struct socket *sock;
  const char *path;
};

static struct list bound_sockets = {};

int local_socket_bind(struct socket *sock, const struct sockaddr *addr,
                      size_t len) {
  struct sockaddr_unix *un = (struct sockaddr_unix *)addr;
  const char *path = un->sun_filepath;
  struct fs_node *fn = vfs_open(path);
  if (fn)
    return -EADDRINUSE;

  fn = vfs_create(path, 0777, FS_SOCKET, 0);
  struct bound_socket *bs = kmalloc_t(struct bound_socket);
  bs->sock = sock;
  bs->path = strdup(path);
  list_push(&bound_sockets, to_list_item(bs));
  return 0;
}

int local_socket_connect(struct socket *sock, const struct sockaddr *addr,
                         size_t len) {
  struct sockaddr_unix *un = (struct sockaddr_unix *)addr;
  const char *path = un->sun_filepath;
  list_foreach(&bound_sockets, item) {
    struct bound_socket *bs = list_item(item, struct bound_socket *);
    if (strcmp(un->sun_filepath, bs->path) == 0) {
      if ((int)bs->sock->conn_queue->length >= bs->sock->backlog) {
        return -ECONNREFUSED;
      }
      list_push(bs->sock->conn_queue, to_list_item(sock));
      return 0;
    }
  }
  return -ECONNREFUSED;
}


int local_socket_recv(struct socket *sock, const void *buf, size_t size,
                      int flags) {
  struct list *queue = sock->messages;
  while (!queue->length) {
    sched_yield();
  }

  struct net_message *msg =
      list_item(list_pop_front(queue), struct net_message *);

  size_t bytes_to_transfer = MIN(size, msg->size);

  memcpy((char *)buf, msg->data, bytes_to_transfer);
  if (size < msg->size) {
    msg->data += size;
    msg->size -= size;
    list_push_front(queue, to_list_item(msg));
  }


  return bytes_to_transfer;
}

int local_socket_send(struct socket *sock, const void *buf, size_t size,
                      int flags) {
  struct list *queue = sock->peer->messages;
  struct net_message *msg = kmalloc_t(struct net_message);
  msg->size = size;
  msg->data = buf;
  list_push(queue, to_list_item(msg));
}

short local_socket_poll(struct socket *sock) {
  short revents = POLLOUT;
  if (sock->messages->length > 0) revents |= POLLIN;
  return revents;
}

struct socket_operations local_socket_ops = {
    .bind = local_socket_bind,
    .connect = local_socket_connect,
    .send = local_socket_send,
    .recv = local_socket_recv,
};

struct socket *create_local_socket(int type) {
  struct socket *sock = kmalloc_t(struct socket);
  assert(type == SOCK_STREAM);
  sock->family = AF_LOCAL;
  sock->type = type;
  sock->ops = &local_socket_ops;
  sock->conn_queue = list_create();
  sock->messages = list_create();

  sock->state = SOCK_STATE_DISCONNECTED;
}
