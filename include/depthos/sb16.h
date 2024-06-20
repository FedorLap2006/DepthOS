// TODO: move this file to a subfolder for drivers

#pragma once

#include "depthos/mutex.h"
#include "depthos/proc.h"
#include "depthos/ringbuffer.h"
#include <depthos/dev.h>

// Standard ports selectable by a jumper
enum sb16_base_port {
  SB16_BASE_JS1 = 0x220, // Jumper setting 1
  SB16_BASE_JS2 = 0x240, // Jumper setting 2
  SB16_BASE_JS3 = 0x260, // Jumper setting 3
  SB16_BASE_JS4 = 0x280, // Jumper setting 4
};

enum sb16_port {
  SB16_MIXER_ADDRESS_PORT = 0x04,
  SB16_MIXER_DATA_PORT = 0x05,
  SB16_DSP_RESET_PORT = 0x06,
  SB16_DSP_READ_PORT = 0x0A,
  SB16_DSP_WRITE_PORT = 0x0C,
  SB16_DSP_STATUS_PORT = 0x0E, // Acts as acknowledgment for 8-bit interrupt
  SB16_DSP_ACK16_PORT = 0x0F
};

enum sb16_mixer_register {
  SB16_MIXER_REG_COMPAT_RESET = 0x00,
  SB16_MIXER_REG_COMPAT_VOICE_VOL = 0x04,
  SB16_MIXER_REG_COMPAT_MIC_VOL = 0x0A,
  SB16_MIXER_REG_COMPAT_MASTER_VOL = 0x22,
  SB16_MIXER_REG_COMPAT_MIDI_VOL = 0x26,
  SB16_MIXER_REG_COMPAT_CD_VOL = 0x28,
  // TODO: registers for controlling L and R separately.

  SB16_MIXER_REG_MASTER_VOL_L = 0x30,
  SB16_MIXER_REG_MASTER_VOL_R = 0x31,
};

#if 0
enum sb16_dma_mode_type {
  SB16_DMA_8BIT_MONO_PCM,
  // SB16_DMA_8BIT_MONO_ADPCM,
  SB16_DMA_8BIT_MONO_PCM_HIGHSPEED,
  SB16_DMA_8BIT_STEREO_PCM_HIGHSPEED,
  SB16_DMA_16BIT_MONO_PCM,   // Allows for 8-bit signed data
  SB16_DMA_16BIT_STEREO_PCM, // Allows for 8-bit signed data
};
#endif

enum sb16_dsp_commands {
  SB16_DSP_CMD_GET_VERSION = 0xE1,

  // Available on 1.xx, 2.xx, 3.xx and 4.xx DSP versions.

  SB16_DSP_CMD_8BIT_DMA_OUT = 0x14,
  SB16_DSP_CMD_8BIT_DMA_IN = 0x24,
  SB16_DSP_CMD_SET_TRANSFER_TIME_CONST = 0x40,

  SB16_DSP_CMD_8BIT_DMA_SINGLECYCLE_OUT = 0x14,
  SB16_DSP_CMD_8BIT_DMA_SINGLECYCLE_IN = 0x24,

  SB16_DSP_CMD_8BIT_DMA_PAUSE = 0xD0,
  SB16_DSP_CMD_8BIT_DMA_CONTINUE = 0xD4,

  SB16_DSP_CMD_8BIT_CTL_TURN_ON = 0xD1,
  SB16_DSP_CMD_8BIT_CTL_TURN_OFF = 0xD3,

  // Available on 2.xx, 3.xx and 4.xx DSP versions.

  SB16_DSP_CMD_8BIT_DMA_AUTOINIT_OUT = 0x1C,
  SB16_DSP_CMD_8BIT_DMA_AUTOINIT_IN = 0x2C,

  SB16_DSP_CMD_8BIT_DMA_AUTOINIT_EXIT = 0xDA,
  SB16_DSP_CMD_SET_BLOCK_TRANSFER_SIZE = 0x48,

  SB16_DSP_CMD_GET_SPEAKER_STATUS,

  // Available on 2.01+ and 3.xx DSP versions.

  SB16_DSP_CMD_8BIT_DMA_AUTOINIT_OUT_HIGHSPEED = 0x90,
  SB16_DSP_CMD_8BIT_DMA_AUTOINIT_IN_HIGHSPEED = 0x98,
  SB16_DSP_CMD_8BIT_DMA_SINGLECYCLE_OUT_HIGHSPEED = 0x91,
  SB16_DSP_CMD_8BIT_DMA_SINGLECYCLE_IN_HIGHSPEED = 0x99,

  // Available only on 3.xx DSP versions.
  SB16_DSP_CMD_CTL_SET_MONO_IN = 0xA0,
  SB16_DSP_CMD_CTL_SET_STEREO_IN = 0xA8,

  SB16_DSP_CMD_SET_SAMPLE_RATE_OUT = 0x41,
  SB16_DSP_CMD_SET_SAMPLE_RATE_IN = 0x42,

  // Available only on 4.xx DSP versions.
  SB16_DSP_CMD_8BIT_DMA_PROGRAM_BASE = 0xC0,

  SB16_DSP_CMD_16BIT_DMA_PROGRAM_BASE = 0xB0,
  SB16_DSP_CMD_16BIT_DMA_PAUSE = 0xD5,
  SB16_DSP_CMD_16BIT_DMA_CONTINUE = 0xD6,
  SB16_DSP_CMD_16BIT_DMA_EXIT = 0xD9,

  // TODO: other missing commands

};

struct sb16_hw_state {};

struct sound_card_sb16 {
  mutex_t mu; // TODO: mutex in device struct
  enum sb16_base_port base_port;
  int dsp_major, dsp_minor;
  struct sb16_hw_state state;

  uint16_t sample_rate;
  bool stereo;
  uint8_t autoinit_exit;

  void *dma_data;
  size_t dma_size;

  void *curr_start, *curr_end;
  pagedir_t curr_pgd;

  // The two following fields describe a current owner of the device.
  // To acquire and release ownership of the device SB16_IOCTL_GRAB and SB16_IOCTL_UNGRAB can be used,
  // with the file descriptor passed as the data parameter.
  //
  // If an SB16_IOCTL_GRAB is issued on any other file descriptor, by any other thread, other than the current owner.
  // An EBUSY will be returned.

  struct task *owner;
  int owner_fd;
};

#define SB16_IOCTL_GRAB 1   // TODO: implement
#define SB16_IOCTL_UNGRAB 2 // TODO: implement
#define SB16_IOCTL_SET_SAMPLERATE 3
#define SB16_IOCTL_SET_STEREO 4
#define SB16_IOCTL_GET_PERIOD 5

void sb16_init();

// NOTE: allowed level values are 0 to 15; 0 = -60 dB, 15 = 0 dB. Each level is
// an represents a 4 dB step.
bool sb16_mixer_set_master_vol(struct sound_card_sb16 *dev, uint8_t level_l, uint8_t level_r);
