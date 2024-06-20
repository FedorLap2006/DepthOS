#include "depthos/list.h"
#include <depthos/heap.h>
#include <depthos/logging.h>
#include <depthos/pci.h>
#include <depthos/ports.h>
#include <depthos/string.h>

// TODO: pci v1 config read

static inline uint32_t pci_conf_field_addr(uint8_t bus, uint8_t slot,
                                           uint8_t func, uint8_t offset) {
  return (1 << 31) | (bus << 16) | ((slot & 0x1F) << 11) | ((func & 0x7) << 8) |
         (offset & 0xFC);
}

uint32_t pci_conf_read8(struct pci_device dev,
                        uint8_t offset) {
  outl(PCI_CONF_ADDR_PORT, pci_conf_field_addr(dev.bus, dev.slot, dev.func, offset));
  return inb(PCI_CONF_DATA_PORT + (offset & 3));
}
uint16_t pci_conf_read16(struct pci_device dev,
                         uint8_t offset) {
  outl(PCI_CONF_ADDR_PORT, pci_conf_field_addr(dev.bus, dev.slot, dev.func, offset));
  return inw(PCI_CONF_DATA_PORT + (offset & 2));
}
uint32_t pci_conf_read32(struct pci_device dev,
                         uint8_t offset) {
  outl(PCI_CONF_ADDR_PORT, pci_conf_field_addr(dev.bus, dev.slot, dev.func, offset));
  return inl(PCI_CONF_DATA_PORT);
}

void pci_conf_write8(struct pci_device dev, uint8_t offset,
                     uint8_t value) {
  outl(PCI_CONF_ADDR_PORT, pci_conf_field_addr(dev.bus, dev.slot, dev.func, offset));
  outb(PCI_CONF_DATA_PORT + (offset & 3), value);
}
void pci_conf_write16(struct pci_device dev, uint8_t offset,
                      uint16_t value) {
  outl(PCI_CONF_ADDR_PORT, pci_conf_field_addr(dev.bus, dev.slot, dev.func, offset));
  outw(PCI_CONF_DATA_PORT + (offset & 2), value);
}
void pci_conf_write32(struct pci_device dev, uint8_t offset,
                      uint32_t value) {
  outl(PCI_CONF_ADDR_PORT, pci_conf_field_addr(dev.bus, dev.slot, dev.func, offset));
  outl(PCI_CONF_DATA_PORT, value);
}

struct pci_header pci_get_header(struct pci_device dev) {
  uint16_t vendor_id = pci_conf_read16(dev, 0x0);
  if (vendor_id == PCI_INVALID_VENDOR) // Non-existent device.
    return (struct pci_header){.vendor_id = PCI_INVALID_VENDOR};

  struct pci_header h = (struct pci_header){
      .vendor_id = vendor_id,
      .device_id = pci_conf_read16(dev, 0x2),

      .command_reg = pci_conf_read16(dev, 0x4),
      .status_reg = pci_conf_read16(dev, 0x6),

      .revision_id = pci_conf_read8(dev, 0x8),
      .prog_iface = pci_conf_read8(dev, 0x9),
      .subclass = pci_conf_read8(dev, 0xA),
      .class = pci_conf_read8(dev, 0xB),

      .cache_line_size = pci_conf_read8(dev, 0xC),
      .latency_timer = pci_conf_read8(dev, 0xD),
      .header_type = pci_conf_read8(dev, 0xE) & 0x3,
      .multifunction = (pci_conf_read8(dev, 0xE) & 0x80) != 0,
      .bist = pci_conf_read8(dev, 0xF),
  };

  switch (h.header_type) {
  case PCI_HEADER_GENERAL_DEV:
    h.general_device = (struct pci_header_general_dev){
        .bars =
            {
                [0] = pci_conf_read32(dev, 0x10),
                [1] = pci_conf_read32(dev, 0x14),
                [2] = pci_conf_read32(dev, 0x18),
                [3] = pci_conf_read32(dev, 0x1C),
                [4] = pci_conf_read32(dev, 0x20),
                [5] = pci_conf_read32(dev, 0x24),
            },
        .cardbus_cis_ptr = pci_conf_read32(dev, 0x28),
        .subsystem_vendor_id = pci_conf_read16(dev, 0x2C),
        .subsystem_id = pci_conf_read16(dev, 0x2E),
        .expansion_rom_addr = pci_conf_read32(dev, 0x30),
        .capabilities_ptr = pci_conf_read8(dev, 0x34),
        .intr_line = pci_conf_read8(dev, 0x3C),
        .intr_pin = pci_conf_read8(dev, 0x3D),
        .min_grant = pci_conf_read8(dev, 0x3E),
        .max_latency = pci_conf_read8(dev, 0x3F),
    };
    break;
  default:
    break;
  }

  return h;
}

static const char *pci_vendors[] = {
#define PCI_VENDOR(id, name) [id] = name,
#include "pci-vendors.inc"
#undef PCI_VENDOR
};

struct list pci_drivers;
void pci_register_driver(struct pci_driver *d) {
  list_push(&pci_drivers, (list_value_t)d);
}

static void pci_enum_func(struct pci_device dev,
                           struct pci_header h) {
  klogf("[%d:%d.%d] vendor_id=0x%x (%s) device_id=0x%x class=0x%x "
        "subclass=0x%x header_type=0x%x multifunction=%d",
        dev.bus, dev.slot, dev.func, h.vendor_id, pci_vendors[h.vendor_id], h.device_id,
        h.class, h.subclass, h.header_type, h.multifunction);

  list_foreach(&pci_drivers, item) {
    struct pci_driver * d = list_item(item, struct pci_driver *);
    if (!d || d->vendor_id != h.vendor_id || d->device_id != h.device_id)
      continue;
    
    d->init(dev, h);
  }
}

static void pci_enum_slot(uint8_t bus, uint8_t slot) {
  struct pci_device dev = (struct pci_device) { .bus = bus, .slot = slot, .func = 0 };
  struct pci_header h = pci_get_header(dev);
  if (h.vendor_id == PCI_INVALID_VENDOR)
    return;

  pci_enum_func(dev, h);

  if (h.multifunction) {
    h.vendor_id = 0xFFFF;
    for (int func = 1; func < 8; func++) {
      dev.func = func; 
      h = pci_get_header(dev);
      if (h.vendor_id == PCI_INVALID_VENDOR)
        continue;
      pci_enum_func(dev, h);
    }
  }
}

void pci_enum() {
  klogf("enumerating pci");
  for (int bus = 0; bus < PCI_NUM_BUSES; bus++) {
    for (int slot = 0; slot < PCI_NUM_DEVICES; slot++) {
      pci_enum_slot(bus, slot);
    }
  }
}

void pci_init() {
  list_init(&pci_drivers);
}
