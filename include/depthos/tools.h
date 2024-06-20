#pragma once

#include <depthos/stdtypes.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(*(a)))
#define MIN(a, b) ((a) > (b) ? b : a)
#define MAX(a, b) ((a) > (b) ? a : b)
#define SAFE_FNPTR_CALL(fnptr, errv, ...) (fnptr ? fnptr(__VA_ARGS__) : errv)
#define CEIL_DIV(a, b) (((a) / (b)) + ((a%b) != 0))
#define optim_barrier() __asm__ volatile("" : : : "memory");

#define __noreturn __attribute__((noreturn))
#define __unused __attribute__((unused))
#define __pack __attribute__((packed))
#define __align(X) __attribute__((aligned(X)))
#define __section(X) __attribute((section(X)))
#define __always_inline __attribute__((always_inline))
#define __noinline __attribute__((noinline))
#define __cleanup(x) __attribute__((cleanup(x)))
#define __printf_fmt(S, A) __attribute__((format(__printf__, S, A)))
#ifdef __GCC__
#define __nooptim __attribute__((optimize("-O0")))
#else
#define __nooptim
#endif
#define offsetof(type, member) __builtin_offsetof(type, member)
#define typeof(v) __typeof__(v)
#define asmlinkage __attribute__((__regparm__(0)))

#define __pragma(x) _Pragma(#x)
#define __diag_push(compiler) __pragma(compiler diagnostic push)
#define __diag_pop(compiler) __pragma(compiler diagnostic pop)
#if defined(__GNUC__) && (__GNUC__ > 8) || (__GNUC__ == 8 && __GNUC_MINOR__ > 1)
#define __diag_ignore_GCC(option) __pragma(GCC diagnostic ignored option)
#else
#define __diag_ignore_GCC(option)
#endif
#define __diag_ignore_clang(option)

#define __diag_ignore(compiler, option, comment)                               \
  __diag_ignore_##compiler(option)

#define __diag_ignore_all(option, comment)                                     \
  __diag_ignore(GCC, option, comment);                                         \
  __diag_ignore(clang, option, comment)

#define ___stringify(x) #x
#define __stringify(x) ___stringify(x)

#define __MAP0(m, ...)
#define __MAP1(m, t, a) m(t, a)
#define __MAP2(m, t, a, ...) m(t, a), __MAP1(m, __VA_ARGS__)
#define __MAP3(m, t, a, ...) m(t, a), __MAP2(m, __VA_ARGS__)
#define __MAP4(m, t, a, ...) m(t, a), __MAP3(m, __VA_ARGS__)
#define __MAP5(m, t, a, ...) m(t, a), __MAP4(m, __VA_ARGS__)
#define __MAP6(m, t, a, ...) m(t, a), __MAP5(m, __VA_ARGS__)
#define __MAP(x, ...) __MAP##x(__VA_ARGS__)

#define safe_add(a, b, res) __builtin_add_overflow(a, b, res)
#define safe_sub(a, b, res) __builtin_sub_overflow(a, b, res)
