#pragma once

// #include "stdtypes.h"
// #include "stdafx.h"

#include "../stdtypes.h"
#include "../stdafx.h"

#define LBA_FS_OK 0
#define LBA_FS_FAIL 1


#define LBA_FS_WRITE 0x03
#define LBA_FS_READ 0x02

typedef struct DAP{
//    ubyte size;
    uint16 sector_count;
    uint8 *buf;
    uint64 sector;
}DAP;

// not used comments ( examples )

// uint16 func;
// uint32 hdisk;
//
// uint32 sector_count;
// ubyte *buf;
// byte func; // read / write / etc.
//    uint dap_addr; // ds:si -- __LINE__

typedef struct LBA{
    uint32 hdrive;
    DAP dap; // dap info
}LBA;

int sys_writeLBA(LBA info);
int sys_readLBA(LBA info);
