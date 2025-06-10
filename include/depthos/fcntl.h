#pragma once

#define O_ACCMODE 0x0007
#define O_EXEC 1
#define O_RDONLY 2
#define O_RDWR 3
#define O_SEARCH 4
#define O_WRONLY 5

#define O_APPEND    0x000008
#define O_CREAT     0x000010
#define O_DIRECTORY 0x000020
#define O_EXCL      0x000040
#define O_NOCTTY    0x000080
#define O_NOFOLLOW  0x000100
#define O_TRUNC     0x000200
#define O_NONBLOCK  0x000400
#define O_DSYNC     0x000800
#define O_RSYNC     0x001000
#define O_SYNC      0x002000
#define O_CLOEXEC   0x004000
#define O_PATH      0x008000
#define O_LARGEFILE 0x010000
#define O_NOATIME   0x020000
#define O_ASYNC     0x040000
#define O_TMPFILE   0x080000
#define O_DIRECT    0x100000
