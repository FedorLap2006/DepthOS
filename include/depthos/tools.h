#pragma once

#include <depthos/stdtypes.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*(a)))
#define MIN(a, b) ((a) > (b) ? b : a)
#define MAX(a, b) ((a) > (b) ? a : b)

#define __noreturn __attribute__((noreturn))
#define __unused __attribute__((unused))
#define __pack __attribute__((packed))
#define __align(X) __attribute__((aligned(X)))
#define __always_inline __attribute__((always_inline))
#define __noinline __attribute__((noinline))
#define offsetof(type, member) __builtin_offsetof(type, member)
#define asmlinkage __attribute__((__regparm__(0)))

#define __pragma(x) _Pragma(#x)
#define __diag_push(compiler) __pragma(compiler diagnostic push)
#define __diag_pop(compiler) __pragma(compiler diagnostic pop)
#define __diag_ignore(compiler, option, comment)                               \
  __pragma(compiler diagnostic ignored option)

#define ___stringify(x) #x
#define __stringify(x) ___stringify(x)

#define __MAP0(m, ...)
#define __MAP1(m, t, a) m(t, a)
#define __MAP2(m, t, a, ...) m(t, a), __MAP1(m, __VA_ARGS__)
#define __MAP3(m, t, a, ...) m(t, a), __MAP2(m, __VA_ARGS__)
#define __MAP(x, ...) __MAP##x(__VA_ARGS__)
