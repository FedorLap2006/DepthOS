#include <depthos/math.h>

int ceil_div(int a, int b) {
  int res = a / b;
  if (a % b != 0)
    res++;
  return res;
}
