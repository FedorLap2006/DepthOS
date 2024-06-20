#include <depthos/logging.h>
#include <depthos/dma-isa.h>
#include <depthos/mutex.h>
#include <depthos/ports.h>
#include <depthos/stdtypes.h>

static int isa_dma_addr_ports[ISA_DMA_NUM_CHANNELS] = {0x0,  0x2,  0x4,  0x6,
                                               0xC0, 0xC4, 0xC8, 0xCC};
static int isa_dma_addrx_ports[ISA_DMA_NUM_CHANNELS] = {0x87, 0x83, 0x81, 0x82,
                                                0x8F, 0x8B, 0x89, 0x8A};
static int isa_dma_offset_ports[ISA_DMA_NUM_CHANNELS] = {0x1,  0x3,  0x5,  0x7,
                                                 0xC2, 0xC6, 0xCA, 0xCE};

#define DEFINE_PORT(N, PV, SV) static int isa_dma_##N##_port[2] = {(PV), (SV)};

DEFINE_PORT(status, 0x08, 0xD0);
DEFINE_PORT(command, 0x08, 0xD0);
DEFINE_PORT(request, 0x09, 0xD2);
DEFINE_PORT(single_mask, 0x0A, 0xD4);
DEFINE_PORT(mode, 0x0B, 0xD6);
DEFINE_PORT(flipflop_reset, 0x0C, 0xD8);
DEFINE_PORT(intermediate, 0x0D, 0xDA);
DEFINE_PORT(primary_reset, 0x0D, 0xDA);
DEFINE_PORT(mask_reset, 0x0E, 0xDC);
DEFINE_PORT(multi_mask, 0x0F, 0xDE);

#define CHECK_PORT(I, R)                                                       \
  if (I >= ISA_DMA_NUM_CHANNELS)                                                   \
  return (R)
#define PORTV(N, I) (isa_dma_##N##_port[(I) >= ISA_DMA_NUM_CONTROLLER_CHANNELS])

bool isa_dma_set_mode(int ch, int dir, bool autoinit, bool down, int type) {
  CHECK_PORT(ch, false);
  if (dir > ISA_DMA_MODE_TYPE_MASK)
    return false;
  if (type > (ISA_DMA_MODE_TYPE_MASK - 1))
    return false;

  uint8_t v = ch & 0x3;
  v |= (dir & ISA_DMA_MODE_DIR_MASK) << ISA_DMA_MODE_DIR_SHIFT;
  v |= (autoinit & 0x1) << ISA_DMA_MODE_AUTOINIT_SHIFT;
  v |= (down & 0x1) << ISA_DMA_MODE_DOWN_SHIFT;
  v |= (type & ISA_DMA_MODE_TYPE_MASK) << ISA_DMA_MODE_TYPE_SHIFT;
  // klogf("dma mode set: %d (dir=%d autoinit=%d down=%d type=%d)", v, dir, autoinit, down, type);
  outb(PORTV(mode, ch), v);

  return true;
}

bool isa_dma_set_state(int ch, bool enabled) {
  CHECK_PORT(ch, false);
  // klogf("outb(%d, %d)", PORTV(single_mask, ch), ((enabled & 0x1) << 2) | (ch & 0x3));
  outb(PORTV(single_mask, ch), ((!enabled & 0x1) << 2) | (ch & 0x3));

  return true;
}

bool isa_dma_reset_flipflop(int ch) {
  CHECK_PORT(ch, false);
  outb(PORTV(flipflop_reset, ch), 0x0);

  return true;
}

bool isa_dma_set_address(int ch, uint32_t addr) {
  CHECK_PORT(ch, false);
  if (addr > ISA_DMA_MAX_ADDR)
    return false;

  isa_dma_reset_flipflop(ch);

  // klogf("addr: %p", (void*)addr);
  uint32_t start = addr >> 1;

  // klogf("writing addr to %x: 0..7=%lx", isa_dma_addr_ports[ch], start & 0xFF);
  outb(isa_dma_addr_ports[ch], start & 0xFF);
  // klogf("writing addr to %x: 8..15=%lx", isa_dma_addr_ports[ch], (start >> 8) & 0xFF);
  outb(isa_dma_addr_ports[ch], (start >> 8) & 0xFF);
  // klogf("writing addr to %x: 16..23=%lx", isa_dma_addrx_ports[ch], (addr >> 16) & 0xFF);
  outb(isa_dma_addrx_ports[ch], (addr >> 16) & 0xFF);

  return true;
}

bool isa_dma_set_length(int ch, size_t length) {
  CHECK_PORT(ch, false);
  if (length > ISA_DMA_MAX_LEN)
    return false;

  isa_dma_reset_flipflop(ch);
  outb(isa_dma_offset_ports[ch], length & 0xFF);
  outb(isa_dma_offset_ports[ch], (length >> 8) & 0xFF);

  return true;
}

MUTEX_DECLARE(isa_dma_mutex);
mutex_t isa_dma_channel_mutexes[ISA_DMA_NUM_CHANNELS] = {
    MUTEX_UNLOCKED, MUTEX_UNLOCKED, MUTEX_UNLOCKED, MUTEX_UNLOCKED,
    MUTEX_UNLOCKED, MUTEX_UNLOCKED, MUTEX_UNLOCKED, MUTEX_UNLOCKED
};
