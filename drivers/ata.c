#include "depthos/dev.h"
#include "depthos/heap.h"
#include "depthos/idt.h"
#include <depthos/ata.h>
#include <depthos/logging.h>
#include <depthos/ports.h>
#include <depthos/strconv.h>
#include <depthos/string.h>

struct ata_dev_impl {
  struct ata_port *port;
  int drive;
};

#define IMPL(dev) ((struct ata_dev_impl *)dev->impl)
void ata_io_wait(struct ata_port *port,
                 bool alternate) { // XXX: should an IRQ be used?
  // https://wiki.osdev.org/ATA_PIO_Mode#400ns_delays
  for (int i = 0; i < 14; i++)
    inb(alternate ? port->ctl_base + ATA_REG_ALT_STAT
                  : port->io_base + ATA_REG_STAT);
}

static void ata_wait(struct ata_port *port, bool alternate) {
  while (1) {
    uint8_t v = inb(alternate ? port->ctl_base + ATA_REG_ALT_STAT
                              : port->io_base + ATA_REG_STAT);
    klogf("wait: 0x%x (rdy=%d bsy=%d)", v, v & ATA_STATUS_RDY != 0,
          v & ATA_STATUS_BSY != 0);
    if (v & (ATA_STATUS_RDY | ATA_STATUS_BSY) != ATA_STATUS_RDY)
      continue;
    break;
  }
}

void ata_drive_select(struct ata_port *port, int drive) {
  outb(port->io_base + ATA_REG_DRIVE_HEAD, 0xA0 | (drive & 0x1) << 4);
}

int ata_get_selected_drive(struct ata_port *port) {
  return (inb(port->io_base + ATA_REG_DRIVE_HEAD) >> 4) & 0x1;
}

void ata_reset(struct ata_port *port) {
  outb(port->ctl_base + ATA_REG_DEV_CTL, 0x4);
  for (int i = 0; i < 12; i++)
    ata_io_wait(
        port,
        false); // 5us delay.
                // https://wiki.osdev.org/ATA_PIO_Mode#Device_Control_Register_.28Control_base_.2B_0.29
  outb(port->ctl_base + ATA_REG_DEV_CTL, 0x0);
  ata_wait(port, false);
}

void ata_send_command(struct ata_port *port, uint8_t command) {
  outb(port->io_base + ATA_REG_CMD, command);
}

uint8_t ata_read_status(struct ata_port *port, bool alternate) {
  return inb(alternate ? port->ctl_base + ATA_REG_ALT_STAT
                       : port->io_base + ATA_REG_STAT);
}

static void ata_reorder_string(char *str, int sz) {
  uint16_t *rstr = str;
  for (int i = 0; i < sz / 2; i++) {
    uint16_t v = rstr[i];
    *(str++) = (v >> 8) & 0xFF;
    *(str++) = v & 0xFF;
  }
}

ata_identify_data_t *ata_identify(struct ata_port *port, int drive) {
  ata_identify_data_t *data = kmalloc(256 * sizeof(uint16_t));
  uint16_t *rdata = (uint16_t *)data;
  memset(data, 0, 256 * sizeof(uint16_t));
  ata_reset(port);
  ata_io_wait(port, false);
  ata_drive_select(port, drive);
  ata_wait(port, false);
  outb(port->io_base + ATA_REG_LBA_LOW, 0x0);
  outb(port->io_base + ATA_REG_LBA_MID, 0x0);
  outb(port->io_base + ATA_REG_LBA_HI, 0x0);
  ata_send_command(port, ATA_CMD_IDENTIFY);
  uint8_t status = ata_read_status(port, false);
  if (!status) {
    kfree(data, 256 * sizeof(uint16_t));
    return NULL;
  }

  uint8_t lba_lo = inb(port->io_base + ATA_REG_LBA_LOW);
  uint8_t lba_hi = inb(port->io_base + ATA_REG_LBA_HI);
  if (lba_lo || lba_hi) {
    klogf("device is not ata");
    kfree(data, 256 * sizeof(uint16_t));
    return NULL;
  }

  bool drq = false, err = false;
  status = ata_read_status(port, false);
  klogf("polling ata status: %d", status);
  while (!(err = (status & ATA_STATUS_ERR) != 0) &&
         !(drq = (status & ATA_STATUS_DRQ) != 0)) {
    klogf("polling ata status: %d", status);
    if (inb(port->io_base + ATA_REG_LBA_MID) ||
        inb(port->io_base + ATA_REG_LBA_HI)) {
      klogf("device is not ata");
      kfree(data, 256 * sizeof(uint16_t));
      return NULL;
    }
    status = ata_read_status(port, false);
  }

  klogf("finished polling ata status: %d (err=%d drq=%d)", status, err, drq);

  if (err) {
    klogf("error reading ata device: err=%d drq=%d", err, drq);
    return NULL;
  }

  for (int i = 0; i < 256; i++) {
    uint16_t v = inw(port->io_base + ATA_REG_DATA);
    rdata[i] = v;
  }
  ata_reorder_string(data->model_number, 40);
  ata_reorder_string(data->serial_number, 20);
  return data;
}

void ata_pio_prepare_transaction(struct ata_port *port, size_t lba,
                                 uint8_t sector_count, bool write) {
  outb(port->io_base + ATA_REG_DRIVE_HEAD,
       ata_get_selected_drive(port) << 4 | 0xE0 | (uint8_t)(lba >> 24 & 0x0F));

  outb(port->io_base + ATA_REG_SECTOR_COUNT, sector_count);
  outb(port->io_base + ATA_REG_LBA_LOW, lba & 0xFF);
  outb(port->io_base + ATA_REG_LBA_MID, (lba >> 8) & 0xFF);
  outb(port->io_base + ATA_REG_LBA_HI, (lba >> 16) & 0xFF);
  outb(port->io_base + ATA_REG_CMD, write ? ATA_CMD_WRITE : ATA_CMD_READ);
}

void ata_pio_read(struct ata_port *port, uint16_t *buf, size_t lba,
                  uint8_t sector_count) {

  ata_pio_prepare_transaction(port, lba, sector_count, false);

  for (int i = 0; i < sector_count; i++) {
    ata_wait(port, false);

    for (int j = 0; j < 256; j++) {
      buf[i * 256 + j] = inw(port->io_base + ATA_REG_DATA);
    }
    ata_io_wait(port, false);
  }
}

void ata_pio_write(struct ata_port *port, uint16_t *buf, size_t lba,
                   uint8_t sector_count) {
  ata_pio_prepare_transaction(port, lba, sector_count, true);
  ata_wait(port, false);

  uint8_t sec = 0;
  for (int i = 0; i < sector_count; i++) {
    for (int j = 0; j < 256; j++) {
      outw(port->io_base + ATA_REG_DATA, buf[i * 256 + j]);
      __asm volatile("nop; nop; nop");
    }
    outb(port->io_base + ATA_REG_CMD, ATA_CMD_FLUSH);
    ata_wait(port, false);
    klogf("write status: 0x%x", ata_read_status(port, false));
  }
}

int ata_write(struct device *dev, void *buf, unsigned long count,
              off_t *offset) {
  ata_drive_select(IMPL(dev)->port, IMPL(dev)->drive);
  ata_pio_write(IMPL(dev)->port, buf, *offset, count);
  *offset += count;
  return count;
}

int ata_read(struct device *dev, void *buf, unsigned long count,
             off_t *offset) {
  ata_drive_select(IMPL(dev)->port, IMPL(dev)->drive);
  ata_pio_read(IMPL(dev)->port, buf, *offset, count);
  *offset += count;
  return count;
}

struct device *create_ata_device(struct ata_port *port, int drive) {
  struct device *dev = kmalloc(sizeof(struct device));
  dev->write = ata_write;
  dev->read = ata_read;
  dev->seek = NULL;
  dev->impl = kmalloc(sizeof(struct ata_dev_impl));
  IMPL(dev)->port = port;
  IMPL(dev)->drive = drive;
  dev->type = DEV_BLOCK;
  dev->block_size = 512;
  dev->pos = 0;

  return dev;
}

struct ata_port *create_ata_port(uint16_t ctl, uint16_t io) {
  struct ata_port *port = kmalloc(sizeof(struct ata_port));
  port->ctl_base = ctl;
  port->io_base = io;
  return port;
}
struct ata_port *ata_primary_port;
struct ata_port *ata_secondary_port;

void ata_init() {
  ata_primary_port = create_ata_port(0x3F6, 0x1F0);
  ata_secondary_port = create_ata_port(0x376, 0x170);
  int i = 0;
#define ATA_REGDEV(P, D)                                                       \
  if (ata_identify(P, D)) {                                                    \
    char *buf = kmalloc(8);                                                    \
    itoa(i++, 10, buf + strlen("ata"));                                        \
    memcpy(buf, "ata", strlen("ata"));                                         \
    devfs_register(buf, create_ata_device(P, D));                              \
  }

  ATA_REGDEV(ata_primary_port, 0);
  ATA_REGDEV(ata_primary_port, 1);
  ATA_REGDEV(ata_secondary_port, 0);
  ATA_REGDEV(ata_secondary_port, 1);

  // devfs_register("ata0", device);
}
