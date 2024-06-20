#include <depthos/math.h>
#include <depthos/proc.h>
#include <depthos/ringbuffer.h>
#include <depthos/tools.h>
#include <depthos/assert.h>
#include <depthos/dev.h>
#include <depthos/dma-isa.h>
#include <depthos/heap.h>
#include <depthos/idt.h>
#include <depthos/kernel.h>
#include <depthos/mutex.h>
#include <depthos/paging.h>
#include <depthos/pmm.h>
#include <depthos/ports.h>
#include <depthos/sb16.h>
#include <depthos/stdio.h>
#include <depthos/stdtypes.h>
#include <depthos/string.h>

#define SB16_RESET_CYCLES                                                                                              \
  3000 // XXX: how long does inb take? Currently value assumes 3000 cycles of
       // 1ns delays
#define SB16_RESET_RETRIES 10

static bool sb16_dsp_reset(struct sound_card_sb16 *dev) {
  for (int retry = 0; retry < SB16_RESET_CYCLES; retry++) {
    outb(dev->base_port + SB16_DSP_RESET_PORT, 1);
    for (int delay = 0; delay < SB16_RESET_CYCLES; delay++)
      inb(dev->base_port + SB16_DSP_RESET_PORT);

    outb(dev->base_port + SB16_DSP_RESET_PORT, 0);
    if (inb(dev->base_port + SB16_DSP_READ_PORT) == 0xAA) return true;
  }
  return false;
}

static uint8_t sb16_dsp_read(struct sound_card_sb16 *dev) {
  while ((inb(dev->base_port + SB16_DSP_STATUS_PORT) >> 7) != 1)
    ;
  return inb(dev->base_port + SB16_DSP_READ_PORT);
}
static void sb16_dsp_write(struct sound_card_sb16 *dev, uint8_t value) {
  while ((inb(dev->base_port + SB16_DSP_STATUS_PORT) >> 7) != 0)
    ;

  return outb(dev->base_port + SB16_DSP_WRITE_PORT, value);
}

static uint8_t sb16_mixer_read(struct sound_card_sb16 *dev, enum sb16_mixer_register reg) {
  outb(dev->base_port + SB16_MIXER_ADDRESS_PORT, reg);
  return inb(dev->base_port + SB16_MIXER_DATA_PORT);
}

static void sb16_mixer_write(struct sound_card_sb16 *dev, enum sb16_mixer_register reg, uint8_t value) {
  outb(dev->base_port + SB16_MIXER_ADDRESS_PORT, reg);
  outb(dev->base_port + SB16_MIXER_DATA_PORT, value);
}

bool sb16_mix_compat_set_master_vol(struct sound_card_sb16 *dev, uint8_t level_l, uint8_t level_r) {
  if (level_l > 15 || level_r > 15) return false;

  sb16_mixer_write(dev, SB16_MIXER_REG_COMPAT_MASTER_VOL, level_r | (level_l << 4));
  return true;
}




bool sb16_mix_set_master_vol(struct sound_card_sb16 *dev, uint8_t level_l, uint8_t level_r) {
  if (level_l > 31 || level_r > 31) return false;
  sb16_mixer_write(dev, SB16_MIXER_REG_MASTER_VOL_L, level_l);
  sb16_mixer_write(dev, SB16_MIXER_REG_MASTER_VOL_R, level_r);

}

void sb16_dsp_program_dma_16(
    struct sound_card_sb16 *dev, bool ad, bool autoinit, bool stereo, bool signed_data, uint16_t nsamples) {
  uint8_t dsp_command = SB16_DSP_CMD_16BIT_DMA_PROGRAM_BASE | (ad & 1) << 3 | (autoinit & 1) << 2;

  uint8_t mode = ((stereo & 1) << 5) | ((signed_data & 1) << 4);
  // klogf("program 16: %x %x", dsp_command, mode);
  sb16_dsp_write(dev, dsp_command);
  sb16_dsp_write(dev, mode);
  sb16_dsp_write(dev, nsamples & 0xFF);        // Low byte
  sb16_dsp_write(dev, (nsamples >> 8) & 0xFF); // High byte
}

void sb16_dsp_set_sampling_rate(struct sound_card_sb16 *dev, uint16_t sampling_rate, bool input) {
  sb16_dsp_write(dev, input ? SB16_DSP_CMD_SET_SAMPLE_RATE_IN : SB16_DSP_CMD_SET_SAMPLE_RATE_OUT);
  sb16_dsp_write(dev, (sampling_rate >> 8) & 0xFF); // High byte
  sb16_dsp_write(dev, sampling_rate & 0xFF);        // Low byte
}

void sb16_dsp_set_trblock_size(struct sound_card_sb16 *dev, uint16_t v) {
  sb16_dsp_write(dev, SB16_DSP_CMD_SET_BLOCK_TRANSFER_SIZE);
  sb16_dsp_write(dev, v & 0xFF);
  sb16_dsp_write(dev, (v >> 8) & 0xFF);
}
#define SB16_MAX_BLK_SIZE ISA_DMA_MAX_LEN

void sb16_prepare_dma16(struct sound_card_sb16 *dev, uintptr_t buffer, size_t length, bool autoinit) {
  mutex_acquire_yield(&isa_dma_channel_mutexes[5]);
  mutex_acquire_yield(&isa_dma_mutex);
  assert(isa_dma_set_state(5, false));
  assert(isa_dma_set_mode(5, ISA_DMA_MODE_DIR_PERIPHERAL_READING, autoinit, false, ISA_DMA_MODE_TYPE_SINGLE));
  assert(isa_dma_set_address(5, (uint32_t)buffer));
  assert(isa_dma_set_length(5, length));
  assert(isa_dma_set_state(5, true));
  mutex_release(&isa_dma_mutex);
}

void sb16_conclude_dma16(struct sound_card_sb16 *dev) { mutex_release(&isa_dma_channel_mutexes[5]); }

void sb16_start_transfer(struct sound_card_sb16 *dev, size_t nbytes, bool autoinit) {
  nbytes = MIN(nbytes, dev->dma_size);
  klogf("nbytes: %ld", nbytes);
  size_t nsamples = nbytes / sizeof(int16_t);

  if (dev->stereo) nsamples /= 2;

  sb16_prepare_dma16(dev, ADDR_TO_PHYS(dev->dma_data), nbytes - 1, autoinit);
  sb16_dsp_set_sampling_rate(dev, dev->sample_rate, false);
  sb16_dsp_program_dma_16(dev, false, autoinit, dev->stereo, true, nsamples - 1);
}

#define IMPL(dev) ((struct sound_card_sb16 *)dev->impl)

/*
sys_write can be implemented with a few behaviors for allowing smooth playback:
* (nsamples - period_size)
* period_size is available in dma buffer

This either could be hardcoded, or made into an ioctl setting.

For both cases period_size*2 writes are a must.

A write with smaller buffer than that can be still allowed, but on the discretion of the developer.

UPD: currently write is implemented to support the second option.

*/

#define rbuf_dump(card)                                                                                                \
  klogf("rbuf: max=%ld size=%ld write_idx=%ld read_idx=%ld elem_size=%ld", card->dma_rbuf.max_size,                                  \
      ringbuffer_size(&card->dma_rbuf), card->dma_rbuf.write_idx, card->dma_rbuf.read_idx, card->dma_rbuf.elem_size);


int sb16_dev_write(struct device *dev, void *buffer, size_t nbytes, off_t *offset) {
  struct sound_card_sb16 *card = IMPL(dev);


  if (nbytes == 0)
    return 0;

  if (card->curr_start) {
    // klogf("eagain");
    return -EAGAIN;
  }

  memcpy(card->dma_data, buffer, MIN(nbytes, card->dma_size));
  card->curr_start = buffer;
  card->curr_end = buffer + nbytes;
  card->curr_pgd = get_current_pgd();

  klogf("write %ld", nbytes);
  sb16_start_transfer(card, MIN(nbytes, card->dma_size), false); // TODO: remove if autoinit
  return MIN(nbytes, card->dma_size);
}

struct device *current_sb16_dev;

void sb16_irq_handler(regs_t *regs) {
  struct sound_card_sb16 *card = IMPL(current_sb16_dev);
  inb(card->base_port + SB16_DSP_ACK16_PORT);
  card->curr_start = NULL;
  card->curr_end = NULL;
  // memset(card->dma_data, 30, 1000*2);
  sb16_conclude_dma16(card);
  klogf("write concluded");
  return;
}

long sb16_dev_ioctl(struct device *dev, unsigned long request, void *data) {
  struct sound_card_sb16 *card = IMPL(dev);
  switch (request) {
  // TODO: file locking and fd's in devices
  // case SB16_IOCTL_GRAB: {
  //   if (card->owner && card->owner_fd != -1)
  //     return -EBUSY;
  //   card->owner = current_task;
  //   card->owner_fd = (uint16_t)(uint64_t)data;
  //   break;
  // }
  // case SB16_IOCTL_UNGRAB: {
  //   card->owner = NULL;
  //   card->owner_fd = -1;
  //   break;
  // }
  case SB16_IOCTL_SET_SAMPLERATE: {
    card->sample_rate = (uint16_t)(uintptr_t)data;
    break;
  }
  case SB16_IOCTL_SET_STEREO:
    card->stereo = (bool)(uintptr_t)data;
    break;
  case SB16_IOCTL_GET_PERIOD:
    return ISA_DMA_MAX_LEN / 2;
  }

  return 0;
}

struct sound_card_sb16 *create_sb16_card(int base_port) {
  struct sound_card_sb16 *ret = (struct sound_card_sb16 *)kmalloc(sizeof(struct sound_card_sb16));
  MUTEX_INIT(ret->mu);
  ret->base_port = base_port;
  uintptr_t dma = pmm_alloc(CEIL_DIV(SB16_MAX_BLK_SIZE, PAGE_SIZE));
  assert(dma != INVALID_ADDR);
  assert(dma < 0x1000000 /* 16 MB, the limit of DMA addressing */);
  // FIXME: very q&d!
  map_addr_fixed(kernel_pgd, ADDR_TO_VIRT(dma), dma, CEIL_DIV(SB16_MAX_BLK_SIZE, PAGE_SIZE), false, false);
  ret->dma_data = (void *)ADDR_TO_VIRT(dma);
  ret->dma_size = SB16_MAX_BLK_SIZE;
  memset(ret->dma_data, 0, ret->dma_size);
  // ringbuffer_init(&ret->dma_rbuf, true);
  ret->curr_start = ret->curr_end = NULL;
  ret->owner = NULL;
  ret->owner_fd = -1;
  return ret;
}

struct device *create_sb16_device(struct sound_card_sb16 *card) {
  struct device *dev = (struct device *)kmalloc(sizeof(struct device));
  dev->name = "sb16";
  dev->seek = NULL;
  dev->read = NULL;
  dev->write = sb16_dev_write;
  dev->ioctl = sb16_dev_ioctl;
  dev->mmap = NULL; // XXX: User space DMA?
  dev->class = DEV_C_GENERIC;
  dev->type = DEV_BLOCK;
  dev->block_size = 1;
  dev->pos = 0;

  dev->impl = card;
}

void sb16_init() {
  struct sound_card_sb16 *card = create_sb16_card(SB16_BASE_JS1);
  struct device *dev = create_sb16_device(card);

  int reset = sb16_dsp_reset(IMPL(dev));
  klogf("sb16 reset: %d", reset);

  sb16_dsp_write(card, SB16_DSP_CMD_GET_VERSION);
  card->dsp_major = sb16_dsp_read(card);
  card->dsp_minor = sb16_dsp_read(card);
  klogf("sb16 version: %d.%d", card->dsp_major, card->dsp_minor);

  assert(card->dsp_major == 4);

  // klogf("mixer: l=%d, r=%d", sb16_mixer_read(card, SB16_MIXER_REG_MASTER_VOL_L), sb16_mixer_read(card, SB16_MIXER_REG_MASTER_VOL_R));
  // sb16_mix_set_master_vol(card, 0, 0);
  // klogf("mixer: l=%d, r=%d", sb16_mixer_read(card, SB16_MIXER_REG_MASTER_VOL_L), sb16_mixer_read(card, SB16_MIXER_REG_MASTER_VOL_R));

  register_device(dev);
  current_sb16_dev = dev;
  idt_register_interrupt(0x20 + 5 /* FIXME: proper detection */, sb16_irq_handler);

  // printk("sb16: resetting: %d\n", sb16_dsp_reset(IMPL(dev)));
}
