#pragma once
#include "../../stdtypes.h"

ubyte *GetAddr( ubyte* addr, uint64 offset);
ubyte *Alloc(uint64 size);
void Free(ubyte* ptr);
