#pragma once

#include <depthos/stddef.h>
#include <depthos/list.h>

#define AF_UNIX 0x3
#define AF_LOCAL AF_UNIX


typedef int sa_family_t;

struct sockaddr_unix {
  sa_family_t sun_family; // AF_UNIX
  char sun_filepath[108]; // Null-terminated
};

struct sockaddr {
  sa_family_t sa_family;
  char sa_data[14];
};

struct socket;

struct socket_operations {
  int (*connect)(struct socket* socket, const struct sockaddr *addr, size_t addrlen);
  int (*bind)(struct socket* socket, const struct sockaddr *addr, size_t addrlen);
  int (*send)(struct socket *socket, const void* buf, size_t size, int flags);
  int (*recv)(struct socket *socket, const void* buf, size_t size, int flags);
  short (*poll)(struct socket* socket); 
};

struct net_message {
  const void *data;
  size_t size;
};

struct socket {
  int family;
#define SOCK_STREAM 0x4
#define SOCK_SEQPACKET 0x3
  int type;
#define SOCK_STATE_DISCONNECTED 0
#define SOCK_STATE_LISTENING 1
#define SOCK_STATE_CONNECTED 2
  int state;
  struct list *conn_queue;
  int backlog;
  struct list *messages;
  struct socket *peer;

  struct socket_operations *ops;
};

struct socket *create_local_socket(int type);

static inline struct socket *create_socket(int family, int type) {
  switch(family) {
  case AF_LOCAL:
    return create_local_socket(type);
  }
  return NULL;
}


#define net_socket_send(socket, buf, size, flags) SAFE_FNPTR_CALL(socket->ops->send, -ENOSYS, socket, buf, size, flags)
#define net_socket_recv(socket, buf, size, flags) SAFE_FNPTR_CALL(socket->ops->recv, -ENOSYS, socket, buf, size, flags)

