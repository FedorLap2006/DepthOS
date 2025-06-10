#pragma once

#include <depthos/stdtypes.h>



#define WAIT_EXIT(ec) (((ec) & 0xff) << 8)
#define WAIT_SIG(sig) ((sig) & 0x7f)
