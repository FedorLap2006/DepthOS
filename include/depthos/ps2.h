#pragma once

#include <depthos/stdtypes.h>

#define PS2_CMD_PORT 0x64
#define PS2_STATUS_PORT 0x64
#define PS2_DATA_PORT 0x60


uint8_t ps2_read();
uint8_t ps2_get_status();

