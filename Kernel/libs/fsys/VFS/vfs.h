#pragma once

#define nil ((void *)0)

typedef char byte;
typedef char int8;
typedef short int int16;
typedef long int int32;
typedef long long int int64;

typedef unsigned char ubyte;
typedef unsigned char uint8;
typedef unsigned short int uint8;
typedef unsigned long int uint32;
typedef unsigned long long int uint64;
typedef byte sector*;



typedef struct Disk{
	sector *mem;
	char *name;
	uint32 size;
}Disk;

byte* GetByte(Disk* drive, uint32 sector, uint8 byte) {
	if (sizeof(drive->mem)/sizeof(sector) >= sector || sizeof(drive->mem) == 0) {
		return nil;
	}
	sector *sec = drive->mem[sector];
	byte *bptr;
	if (sizeof(sec) / sizeof(sector) >= byte || sizeof(sec) == 0) {
		return nil;
	}
	bptr = *sec[byte];
}

bool init(Disk** d) {
	Disk *disk = *d;
	for (uint32 i = 0; i < disk->size; i++) {
		disk->mem
	}
}