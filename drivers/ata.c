#include <depthos/kernel.h>
#include <depthos/paging.h>
#include <depthos/pci.h>
#include <depthos/pmm.h>
#include <depthos/stdtypes.h>
#include <depthos/assert.h>
#include <depthos/ata.h>
#include <depthos/bitmap.h>
#include <depthos/dev.h>
#include <depthos/errno.h>
#include <depthos/heap.h>
#include <depthos/idt.h>
#include <depthos/logging.h>
#include <depthos/ports.h>
#include <depthos/strconv.h>
#include <depthos/string.h>


struct ata_dev_impl {
  struct ata_port *port;
  struct ata_identify *info;
  int drive;
};

#if CONFIG_ATA_LOG_ENABLE == 1
#define ata_log(...) klogf(__VA_ARGS__);
#else
#define ata_log(...)
#endif

#define BOUNDS_CHECK(dev, sector, n)                                           \
  if (sector >= IMPL(dev)->info->current_sector_capacity) {                    \
    ata_log("trying to access with out-of-bounds sector: %d >= %d", sector,    \
            IMPL(dev)->info->current_sector_capacity);                         \
    return 0;                                                                  \
  } else if (sector + n >= IMPL(dev)->info->current_sector_capacity) {         \
    ata_log("trying to access with partially in-bounds sector: %d + %d >= %d", \
            sector, n, IMPL(dev)->info->current_sector_capacity);              \
    n = IMPL(dev)->info->current_sector_capacity - sector;                     \
  }
// #define BOUNDS_CHECK(dev, sector, n) assert(sector + n <
// IMPL(dev)->info->current_sector_capacity);

#define IMPL(dev) ((struct ata_dev_impl *)dev->impl)
void ata_io_wait(struct ata_port *port,
                 bool alternate) { // XXX: should an IRQ be used?
  // https://wiki.osdev.org/ATA_PIO_Mode#400ns_delays
  for (int i = 0; i < 14; i++)
    inb(alternate ? port->ctl_base + ATA_REG_ALT_STAT
                  : port->io_base + ATA_REG_STAT);
}

static void ata_poll(struct ata_port *port, bool alternate, bool data,
                     bool dma) {
  uint8_t v = 0, dv = 0;
  do {
    v = inb(alternate ? port->ctl_base + ATA_REG_ALT_STAT
                      : port->io_base + ATA_REG_STAT);
    if (dma) {
      dv = inb(port->busmaster + ATA_REG_BM_STATUS);
      ata_log("ata poll: dma: inb(%x) = %x (intr=%d err=%d)",
              port->busmaster + ATA_REG_BM_STATUS, dv, dv & ATA_BM_STATUS_INTR,
              dv & ATA_BM_STATUS_ERR);
    }
    ata_log("ata poll: inb(%x) = %x (bsy=%d drq=%d err=%d dfa=%d)",
            alternate ? port->ctl_base + ATA_REG_ALT_STAT
                      : port->io_base + ATA_REG_STAT,
            v, (v & ATA_STATUS_BSY) != 0, (v & ATA_STATUS_DRQ) != 0,
            (v & ATA_STATUS_ERR) != 0, (v & ATA_STATUS_DFA) != 0);
  } while (
      (dma && (dv & ATA_BM_STATUS_INTR) && ((dv & ATA_BM_STATUS_ERR) == 0)) ||
      (((v & ATA_STATUS_BSY) || (data && ((v & ATA_STATUS_DRQ) == 0))) &&
       ((v & ATA_STATUS_ERR) == 0) && ((v & ATA_STATUS_DFA) == 0)));
  // ata_log("stopped polling");

  if (dma && (dv & ATA_BM_STATUS_ERR))
    panicf("encountered IDE DMA error (ctl=%x, io=%x, busmaster=%x) ",
           port->ctl_base, port->io_base, port->busmaster);
  if (v & ATA_STATUS_ERR || v & ATA_STATUS_DFA) {
    panicf("ATA drive (ctl=%x, io=%x) is not working properly.", port->ctl_base,
           port->io_base);
  }
}

// static void ata_poll(struct ata_port *port, bool alternate) {
//   while (1) {
//     uint8_t v = inb(alternate ? port->ctl_base + ATA_REG_ALT_STAT
//                               : port->io_base + ATA_REG_STAT);
//     ata_log("wait: 0x%x (rdy=%d bsy=%d err=%d idx=%d drq=%d)", v,
//           v & ATA_STATUS_RDY, v & ATA_STATUS_BSY, v & ATA_STATUS_ERR,
//           v & ATA_STATUS_IDX, v & ATA_STATUS_DRQ);
//     ata_log("testing %x %x %x", v & (ATA_STATUS_RDY | ATA_STATUS_BSY),
//           v & (ATA_STATUS_RDY | ATA_STATUS_BSY) > 0,
//           v & (ATA_STATUS_RDY | ATA_STATUS_BSY) != ATA_STATUS_RDY);

//     if (v & (ATA_STATUS_RDY | ATA_STATUS_BSY) > 0 &&
//         v & (ATA_STATUS_RDY | ATA_STATUS_BSY) != ATA_STATUS_RDY) {
//       ata_log("not going home");
//       continue;
//     }
//     ata_log("going home: actual=%x required=%x eq=%x", v,
//           v & (ATA_STATUS_RDY | ATA_STATUS_BSY), ATA_STATUS_RDY);

//     break;
//   }
// }

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
  ata_poll(port, false, false, false);
}

void ata_send_command(struct ata_port *port, uint8_t command) {
  outb(port->io_base + ATA_REG_CMD, command);
}

uint8_t ata_read_status(struct ata_port *port, bool alternate) {
  return inb(alternate ? port->ctl_base + ATA_REG_ALT_STAT
                       : port->io_base + ATA_REG_STAT);
}

static void ata_reorder_string(char *str, int sz) {
  uint16_t *rstr = (uint16_t *)str;
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
  // XXX: if device isn't present, are we sending this into the void?
  ata_reset(port);
  ata_io_wait(port, false);
  ata_drive_select(port, drive);
  ata_poll(port, false, false, false);
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
    ata_log("device is not ata");
    kfree(data, 256 * sizeof(uint16_t));
    return NULL;
  }

  bool drq = false, err = false;
  status = ata_read_status(port, false);
  ata_log("polling ata status: %d", status);
  while (!(err = (status & ATA_STATUS_ERR) != 0) &&
         !(drq = (status & ATA_STATUS_DRQ) != 0)) {
    ata_log("polling ata status: %d", status);
    if (inb(port->io_base + ATA_REG_LBA_MID) ||
        inb(port->io_base + ATA_REG_LBA_HI)) {
      ata_log("device is not ata");
      kfree(data, 256 * sizeof(uint16_t));
      return NULL;
    }
    status = ata_read_status(port, false);
  }

  ata_log("finished polling ata status: %d (err=%d drq=%d)", status, err, drq);

  if (err) {
    ata_log("error reading ata device: err=%d drq=%d", err, drq);
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

void ata_prepare_transfer(struct ata_port *port, uint8_t command, size_t lba,
                          uint8_t sector_count) {
  outb(port->io_base + ATA_REG_DRIVE_HEAD,
       ata_get_selected_drive(port) << 4 | 0xE0 | (uint8_t)(lba >> 24 & 0x0F));

  outb(port->io_base + ATA_REG_SECTOR_COUNT, sector_count);
  outb(port->io_base + ATA_REG_LBA_LOW, lba & 0xFF);
  outb(port->io_base + ATA_REG_LBA_MID, (lba >> 8) & 0xFF);
  outb(port->io_base + ATA_REG_LBA_HI, (lba >> 16) & 0xFF);

  ata_poll(port, false, false, false);
  outb(port->io_base + ATA_REG_CMD, command);
}

int ata_pio_read(struct ata_port *port, uint16_t *buf, size_t lba,
                 uint8_t sector_count) {
  ata_poll(port, false, false, false);
  ata_prepare_transfer(port, ATA_CMD_READ, lba, sector_count);
  ata_log("ata: reading %ld sectors at %ld", sector_count, lba);
  for (int i = 0; i < sector_count; i++) {

    ata_poll(port, false, true, false); // TODO: error logging
    // ata_log("began reading");
    for (int j = 0; j < 256; j++) {
      buf[i * 256 + j] = inw(port->io_base + ATA_REG_DATA);
    }
    // ata_log("stopped reading");
    // ata_io_wait(port, false);
  }
  ata_log("read sectors");
  return 0;
}

int ata_dma_read(struct ata_port *port, void *buf, size_t lba,
                 uint8_t sector_count) {
  assert(sector_count <= 128);
  ata_log("status before transaction: %x",
          inb(port->busmaster + ATA_REG_BM_STATUS));
  ata_poll(port, false, false, false);
  port->prdt->nbytes = sector_count * 512;
  outb(port->busmaster + ATA_REG_BM_CMD, 0x0); // Abort any previous transfers.
  outl(port->busmaster + ATA_REG_BM_PRDT, ADDR_TO_PHYS(port->prdt));
  ata_log("prdt: %lx %p", inl(port->busmaster + ATA_REG_BM_PRDT), port->prdt);
  outb(port->busmaster + ATA_REG_BM_STATUS,
       inb(port->busmaster + ATA_REG_BM_STATUS) | ATA_BM_STATUS_INTR |
           ATA_BM_STATUS_ERR); // This actually resets the bits
  ata_log("status after reset: %x", inb(port->busmaster + ATA_REG_BM_STATUS));
  outb(port->busmaster + ATA_REG_BM_CMD, 0x8); // Read
  ata_poll(port, false, false, false);

  ata_prepare_transfer(port, ATA_CMD_READ_DMA, lba, sector_count);
  ata_io_wait(port, false);

  outb(port->busmaster + ATA_REG_BM_CMD, 0x08 | 0x1);
  // ata_poll(port, false, true, true);
  // FIXME: correctly poll in ata_poll
  while (1) {
    int dst = inb(port->busmaster + ATA_REG_BM_STATUS);
    int st = inb(port->io_base + ATA_REG_STAT);

    if (!(dst & ATA_BM_STATUS_INTR))
      continue;

    if (!(st & ATA_STATUS_BSY))
      break;
  }

  memcpy(buf, port->dma_region, sector_count * 512);
  outb(port->busmaster + ATA_REG_BM_STATUS,
       inb(port->busmaster + ATA_REG_BM_STATUS) | ATA_BM_STATUS_INTR |
           ATA_BM_STATUS_ERR); // This actually resets the bits
  return 0;
}

int ata_pio_write(struct ata_port *port, uint16_t *buf, size_t lba,
                  uint8_t sector_count) {
  ata_poll(port, false, false, false);
  ata_prepare_transfer(port, ATA_CMD_WRITE, lba, sector_count);
  ata_poll(port, false, false, false); // TODO: error logging

  uint8_t sec = 0;
  for (int i = 0; i < sector_count; i++) {
    for (int j = 0; j < 256; j++) {
      outw(port->io_base + ATA_REG_DATA, buf[i * 256 + j]);
      __asm volatile("nop; nop; nop");
    }
    outb(port->io_base + ATA_REG_CMD, ATA_CMD_FLUSH);
    ata_poll(port, false, true, false);
    ata_log("write status: 0x%x", ata_read_status(port, false));
  }
  return 0;
}

int ata_write(struct device *dev, void *buf, unsigned long count,
              off_t *offset) {
  BOUNDS_CHECK(dev, *offset, count);
  if (count == 0)
    return 0;
  ata_drive_select(IMPL(dev)->port, IMPL(dev)->drive);
  ata_pio_write(IMPL(dev)->port, buf, *offset, count);
  *offset += count;
  return count;
}

int ata_read(struct device *dev, void *buf, unsigned long count,
             off_t *offset) {
  struct ata_port *port = IMPL(dev)->port;

  BOUNDS_CHECK(dev, *offset, count);
  if (count == 0)
    return 0;

  // ata_drive_select(IMPL(dev)->port, IMPL(dev)->drive);
  ata_log("attempting to read (offset: %ld count: %ld)", *offset, count);
  if (port->dma)
    ata_dma_read(port, buf, *offset, count);
  else
    ata_pio_read(port, buf, *offset, count);
  *offset += count;
  ata_log("read successful");
  return count;
}

struct device *create_ata_device(struct ata_port *port, int drive) {
  struct device *dev = kmalloc(sizeof(struct device));
  dev->write = ata_write;
  dev->read = ata_read;
  dev->seek = NULL; // TODO
  dev->impl = kmalloc(sizeof(struct ata_dev_impl));
  IMPL(dev)->port = port;
  IMPL(dev)->drive = drive;
  dev->type = DEV_BLOCK;
  dev->block_size = 512;
  dev->class = DEV_C_STORAGE;
  dev->pos = 0;

  return dev;
}

struct ata_port *create_ata_port(uint16_t ctl, uint16_t io) {
  struct ata_port *port = kmalloc(sizeof(struct ata_port));
  port->ctl_base = ctl;
  port->io_base = io;
  port->busmaster = 0;
  return port;
}
struct ata_port *ata_primary_port;
struct ata_port *ata_secondary_port;

void ata_dma_init(struct ata_port *port, uint16_t busmaster) {
  port->dma = true;
  port->busmaster = busmaster;
  port->prdt = (struct ata_prd_entry *)kmalloc(sizeof(struct ata_prd_entry));
  if (!port->prdt)
    panicf("could not allocate ata prdt");
  assert(((uintptr_t)port->prdt % 4) == 0); // TODO: aligned malloc
  assert(PAGE_SIZE == 4096);
  uintptr_t region = pmm_alloc(16); // Maximum size, 64KB
  map_addr_fixed(kernel_pgd, ADDR_TO_VIRT(region), region, 16, false,
                 false); // TODO: check if it's mapped
  port->dma_region = (void *)ADDR_TO_VIRT(region);
  port->prdt->nbytes = 0; // Set at each request
  port->prdt->phys_addr = region;
  port->prdt->is_end = true;
}

#if 0
void ata_dma_init(struct ata_port *port, uint16_t busmaster) {
  port->busmaster = busmaster;

  int n_presets = DMA_PRESET_MAX_SECTORS + 1;
  port->dma_regions = (struct ata_dma_region *)kmalloc(
      sizeof(struct ata_dma_region) * n_presets);
  port->prdt =
      (struct ata_prd_entry *)kmalloc(sizeof(struct ata_prd_entry) * n_presets);

  for (int i = 1; i <= DMA_PRESET_MAX_SECTORS; i++) {
    void *ptr = kmalloc(512 * i);
    if (!ptr)
      panicf("could not allocate ata dma region (size=%d)", 512 * i);
    port->dma_regions[i - 1].ptr = ptr;
    port->dma_regions[i - 1].size = i * 512;
    port->prdt[i - 1].phys_addr = ADDR_TO_PHYS(ptr);
    port->prdt[i - 1].nbytes = 512 * i;
  }
  assert(PAGE_SIZE == 4096); // Since we make assumptions about page size.
  uintptr_t spare_region = pmm_alloc(16); // 64KB.
  map_addr_fixed(kernel_pgd, ADDR_TO_VIRT(spare_region), spare_region, 16,
                 false, false);
  port->dma_regions[n_presets - 1].ptr = (void *)spare_region;
  port->dma_regions[n_presets - 1].size = 16 * 4096;
  port->prdt[n_presets - 1].phys_addr = spare_region;
  port->prdt[n_presets - 1].nbytes = 0; // 0 is 64KB in this case.
  port->prdt[n_presets - 1].is_end = true;
}
#endif

void ata_pci_init(struct pci_device dev,
                  struct pci_header h) {
  klogf("busmater: 0x%lx", h.general_device.bars[4]);
  uint32_t bar4 = h.general_device.bars[4];
  bar4 &= ~0x3;
  uint16_t cmd = pci_conf_read16(dev, 0x4); // Command register
  cmd |= 0x4;
  pci_conf_write16(dev, 0x4, cmd);
  ata_dma_init(ata_primary_port, bar4);
  ata_dma_init(ata_secondary_port, bar4 + ATA_BM_SECONDARY_OFFSET);
  // ata_primary_port->busmaster = h.general_device.bars[4];
  // ata_secondary_port->busmaster =
  //     h.general_device.bars[4] + ATA_BM_SECONDARY_OFFSET;
}

struct pci_driver ata_pci_driver = {
    .vendor_id = 0x8086,
    .device_id = 0x7010,
    .init = ata_pci_init,
};

void ata_init() {
  ata_primary_port = create_ata_port(0x3F6, 0x1F0);
  ata_secondary_port = create_ata_port(0x376, 0x170);
  int i = 0;
  struct ata_identify *info;
#define ATA_REGDEV(P, D)                                                       \
  if (info = ata_identify(P, D)) {                                             \
    klogf("ata[%d]: %.*s %.*s", i, 20, info->model_number, 40,                 \
          info->serial_number);                                                \
    char *buf = kmalloc(8);                                                    \
    memset(buf, 0, 8);                                                         \
    itoa(i++, 10, buf + strlen("ata"));                                        \
    memcpy(buf, "ata", strlen("ata"));                                         \
    struct device *dev = create_ata_device(P, D);                              \
    IMPL(dev)->info = info;                                                    \
    dev->name = buf;                                                           \
    register_device(dev);                                                      \
  }

  ATA_REGDEV(ata_primary_port, 0);
  ATA_REGDEV(ata_primary_port, 1);
  ATA_REGDEV(ata_secondary_port, 0);
  ATA_REGDEV(ata_secondary_port, 1);

  pci_register_driver(&ata_pci_driver);

  // devfs_register("ata0", device);
}
