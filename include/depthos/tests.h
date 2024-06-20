#pragma once
#include <depthos/stdio.h>

/**
 * @brief Initialise all kernel tests
 */
extern void tests_init(void);
/**
 * @brief Run the tests
 */
extern void tests_run(void);

#define TEST(name)                                                             \
  printk("\n\x1B[96;1m%s: Executing %s\x1B[0m\n", __func__, name);

#undef assert
#define assert(expr)                                                           \
  if (!(expr))                                                                 \
    panicf("%s: Assertion failed (%s)", __func__, #expr);                      \
  else                                                                         \
    printk("\x1B[32;1m%s: Assertion passed (%s)\x1B[0m\n", __func__, #expr);

