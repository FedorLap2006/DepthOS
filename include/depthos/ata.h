#pragma once

#include <depthos/dev.h>
#include <depthos/stdtypes.h>
#include <depthos/tools.h>

// typedef enum {
//   ATA_DEV_UNKNOWN,
//   ATA_DEV_PATA,
//   ATA_DEV_SATA,
//   ATA_DEV_PATAPI,
//   ATA_DEV_SATAPI
// } ata_dev_t;

#define ATA_REG_DATA 0
#define ATA_REG_ERROR 1
#define ATA_REG_FEATURES 1
#define ATA_REG_SECTOR_COUNT 2
#define ATA_REG_LBA_LOW 3
#define ATA_REG_LBA_MID 4
#define ATA_REG_LBA_HI 5
#define ATA_REG_DRIVE_HEAD 6 // Drive/head register
#define ATA_REG_STAT 7
#define ATA_REG_CMD 7

#define ATA_REG_DEV_CTL 0
#define ATA_REG_ALT_STAT 0

#define ATA_REG_BM_CMD 0x0
#define ATA_REG_BM_STATUS 0x2
#define ATA_REG_BM_PRDT 0x4

#define ATA_BM_SECONDARY_OFFSET 0x8

#define ATA_BM_STATUS_INTR (1 << 2)
#define ATA_BM_STATUS_ERR (1 << 1)

#define ATA_STATUS_BSY (1 << 7) // Busy, prepating to send or receive data.
#define ATA_STATUS_RDY (1 << 6) // Drive is on
#define ATA_STATUS_DFA (1 << 5) // Drive fault
#define ATA_STATUS_SRV (1 << 4) // Overlapped mode service request
#define ATA_STATUS_DRQ (1 << 3) // Data is ready
#define ATA_STATUS_COR (1 << 2) // Corrected data. Always 0
#define ATA_STATUS_IDX (1 << 1) // Index. Always 0
#define ATA_STATUS_ERR (1 << 0) // Error

#define ATA_CMD_IDENTIFY 0xEC
#define ATA_CMD_FLUSH 0xE7
#define ATA_CMD_READ 0x20
#define ATA_CMD_WRITE 0x30
#define ATA_CMD_READ_DMA 0xC8
#define ATA_CMD_WRITE_DMA 0xCA

struct ata_identify {
  struct {
    uint16_t reserved1 : 1;
    uint16_t obsolete1 : 1;
    uint16_t response_incomplete : 1;
    uint16_t obsolete2 : 3;
    uint16_t fixed_device : 1;
    uint16_t removable_media : 1;
    uint16_t obsolete3 : 7;
    uint16_t atapi : 1;
  } general_info;
  uint16_t num_cylinders;
  uint16_t specific_configuration;
  uint16_t num_heads;
  uint16_t retired1[2];
  uint16_t num_sectors_per_track;
  uint16_t vendor_unique1[3];
  char serial_number[20];
  uint16_t deprecated[2];
  uint16_t obsolete1;
  char firmware_revision[8];
  char model_number[40];
  uint16_t maximum_block_transfer;
  struct {
    uint16_t feature_supported : 1;
    uint16_t reserved : 15;
  } trusted_computing;
  struct {
    uint16_t vendor_unique1 : 8;
    uint16_t obsolete1 : 2;
    uint16_t iordy_toggleable : 1;
    uint16_t iordy_support : 1;
    uint16_t obsolete2 : 1;
    uint16_t standy_timer_support : 1;
    uint16_t obsolete3 : 2;
  } capabilities;
  uint16_t reserved1;
  uint8_t vendor_unique2[2];
  uint8_t obsolete;
  uint8_t pio_data_transfer_cycle_timing_mode;
  uint16_t valid_fields : 3;
  uint16_t reserved2 : 5;
  uint16_t freefall_control_sensitivity : 8;
  uint16_t num_current_cylinders;
  uint16_t num_current_heads;
  uint16_t current_sectors_per_track;
  uint32_t current_sector_capacity;
  uint8_t sectors_per_interrupt;
  uint8_t multi_sectors_setting_valid : 1;
  uint8_t reserved3 : 3;
  uint16_t sanitize_feature_supported : 1;
  uint8_t crypto_scramble_ext_command_supported : 1;
  uint8_t overwrite_ext_command_supported : 1;
  uint8_t block_erase_ext_command_supported : 1;
  uint32_t max_addressable_sectors;
} __pack;

typedef struct ata_identify ata_identify_data_t;

struct __pack ata_prd_entry {
  uint32_t phys_addr;
  uint16_t nbytes;
  uint16_t : 15;
  uint16_t is_end : 1;
};

struct ata_port {
  uint16_t io_base, ctl_base;
  uint8_t current_drive_head;
  bool dma;
  uint16_t busmaster;
  void *dma_region;
  struct ata_prd_entry *prdt;
};

ata_identify_data_t *ata_identify(struct ata_port *dev, int drive);
void ata_reset(struct ata_port *dev);
void ata_drive_select(struct ata_port *dev, int drive);
int ata_get_selected_drive(struct ata_port *dev);

void ata_io_wait(struct ata_port *dev, bool alternate);
int ata_pio_read(struct ata_port *dev, uint16_t *buf, size_t lba,
                 uint8_t sector_count);
int ata_pio_write(struct ata_port *dev, uint16_t *buf, size_t lba,
                  uint8_t sector_count);

struct device *create_ata_device(struct ata_port *port, int drive);
struct ata_port *create_ata_port(uint16_t ctl, uint16_t io);

void ata_init();
