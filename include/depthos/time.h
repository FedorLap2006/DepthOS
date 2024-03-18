#pragma once

#include <depthos/stdtypes.h>


typedef int32_t time_t;

struct timespec {
  time_t sec;
  long nano;
};

