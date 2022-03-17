#pragma once

#include <depthos/stdtypes.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*(a)))

#define __noreturn __attribute__((noreturn))
#define __unused __attribute__((unused))
#define __align(X) __attribute__((aligned(X)))
#define __pack __attribute__((packed))

#define ___stringify(x) #x
#define __stringify(x) ___stringify(x)
