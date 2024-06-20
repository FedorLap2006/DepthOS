#pragma once

#include <depthos/mutex.h>
#include <depthos/stdtypes.h>

#define ISA_DMA_MAX_ADDR (16 * 1024 * 1024)
#define ISA_DMA_MAX_LEN (64 * 1024) /* 64KB or 65536 bytes */
#define ISA_DMA_NUM_CHANNELS 8
#define ISA_DMA_NUM_CONTROLLER_CHANNELS 4

bool isa_dma_set_state(int ch, bool enabled);

#define ISA_DMA_MODE_AUTOINIT_SHIFT 4

#define ISA_DMA_MODE_DIR_SHIFT 2
#define ISA_DMA_MODE_DIR_MASK 0x3
#define ISA_DMA_MODE_DIR_SELFTEST 0x00
#define ISA_DMA_MODE_DIR_PERIPHERAL_WRITING 0x01
#define ISA_DMA_MODE_DIR_PERIPHERAL_READING 0x2

#define ISA_DMA_MODE_DOWN_SHIFT 5

#define ISA_DMA_MODE_TYPE_SHIFT 6
#define ISA_DMA_MODE_TYPE_MASK 0x3
#define ISA_DMA_MODE_TYPE_ON_DEMAND 0x00
#define ISA_DMA_MODE_TYPE_SINGLE 0x01
#define ISA_DMA_MODE_TYPE_BLOCK 0x2
#define ISA_DMA_MODE_TYPE_CASCADE 0x3

bool isa_dma_set_mode(int ch, int dir, bool autoinit, bool down, int type);
bool isa_dma_reset_flipflop(int ch);

/**
 * @brief Set transfer address for a channel
 *
 * @param ch Channel to set address for
 * @param addr Address to set. If the channel is 16 bit (5, 6, 7) - needs to be
 * aligned by 2.
 */
bool isa_dma_set_address(int ch, uint32_t addr);
// Must be subtracted by 1.
bool isa_dma_set_length(int ch, size_t length);

extern mutex_t isa_dma_mutex;
extern mutex_t isa_dma_channel_mutexes[ISA_DMA_NUM_CHANNELS];
