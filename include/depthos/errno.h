#pragma once

#include <depthos/stdtypes.h>


#define EINVAL 1
#define EOVERFLOW 2
#define E2BIG 3
#define ENIMPL 4
#define ENOSYS ENIMPL
#define EFAULT 5
#define EACCES 6
#define EPERM 7
#define ENOENT 8
#define EBADF 9
#define EAGAIN 10
#define ENOTDIR 11
#define ENOIOCTL 12
#define ENOTTY ENOIOCTL
#define EMFILE 13
#define ENOEXEC 14
#define EEXIST 15
#define ENOMEM 16
#define ERANGE 17
#define EADDRINUSE 18
#define ECONNREFUSED 19
#define ENOTSOCK 20

extern int errno;
