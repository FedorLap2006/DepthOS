#pragma once

#include <depthos/stdtypes.h>
#include <depthos/tools.h>

#define PCI_CONF_ADDR_PORT 0xCF8
#define PCI_CONF_DATA_PORT 0xCFC

#define PCI_NUM_BUSES 256
#define PCI_NUM_DEVICES 32
#define PCI_NUM_DEVICES_LEGACY 16

#define PCI_INVALID_VENDOR 0xFFFF

enum pci_header_type {
  PCI_HEADER_GENERAL_DEV = 0x0,
  PCI_HEADER_PCI_TO_PCI = 0x1,
  PCI_HEADER_PCI_TO_CARDBUS = 0x2
};

struct pci_header_general_dev {
  uint32_t bars[6];
  uint32_t cardbus_cis_ptr;
  uint16_t subsystem_vendor_id;
  uint16_t subsystem_id;
  uint32_t expansion_rom_addr;
  uint8_t capabilities_ptr;
  uint8_t intr_line, intr_pin;
  uint8_t min_grant;
  uint8_t max_latency;
};

struct pci_header {
  uint16_t vendor_id;
  uint16_t device_id;
  uint16_t status_reg;
  uint16_t command_reg;
  uint8_t class;
  uint8_t subclass;
  uint8_t prog_iface;
  uint8_t revision_id;
  uint8_t bist;
  enum pci_header_type header_type;
  bool multifunction;
  uint8_t latency_timer;
  uint8_t cache_line_size;
  union {
    struct pci_header_general_dev general_device;
  };
};

// #define PCI_CONF_HDR_VENDOR_ID

struct pci_device {
  uint8_t bus, slot, func;
};

uint32_t pci_conf_read8(struct pci_device dev, uint8_t offset);
uint16_t pci_conf_read16(struct pci_device dev, uint8_t offset);
uint32_t pci_conf_read32(struct pci_device dev, uint8_t offset);

void pci_conf_write8(struct pci_device dev, uint8_t offset, uint8_t value);
void pci_conf_write16(struct pci_device dev, uint8_t offset, uint16_t value);
void pci_conf_write32(struct pci_device dev, uint8_t offset, uint32_t value);

struct pci_driver {
  uint16_t vendor_id, device_id;
  void (*init)(struct pci_device dev, struct pci_header);
};

void pci_register_driver(struct pci_driver *d);

// TODO: use offsets instead of parsing it into a struct.

// NOTE: to check for PCI_INVALID_VENDOR in vendor_id to determine whether
// device exists.
struct pci_header pci_get_header(struct pci_device dev);

void pci_init();
void pci_enum();
