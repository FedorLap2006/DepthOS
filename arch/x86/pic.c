#include <depthos/pic.h>

#define PIC_PRIMARY_CMD_PORT 0x20
#define PIC_SECONDARY_CMD_PORT 0x20
#define PIC_PRIMARY_DATA_PORT 0x21
#define PIC_SECONDARY_DATA_PORT 0xA1

static void pic_iowait() { __asm__ volatile("outb %%al, $0x80" : : "a"(0)); }

struct pic_config current_pic_config;

void pic_init(struct pic_config cfg) {
  uint8_t primary_mask, secondary_mask;
  /**
   * Saving old IRQs mask
   */

  if (cfg.dirty_mask) {
    primary_mask = inb(PIC_PRIMARY_DATA_PORT) | (~(cfg.mask) & 0xFF);
    secondary_mask = inb(PIC_SECONDARY_DATA_PORT) | (~(cfg.mask) >> 0x8);

  } else {
    primary_mask = ~(cfg.mask & 0xFF);
    secondary_mask = ~(cfg.mask >> 0x8);
  }

  /**
   * Remaping interrupts table
   */

  outb(PIC_PRIMARY_CMD_PORT,
       0x11); /* Initialisation signal + ICW4 mode [primary] */
  if (cfg.iowait_mode)
    pic_iowait();
  outb(PIC_SECONDARY_CMD_PORT,
       0x11); /* Initialisation signal + ICW4 mode [secondary] */
  if (cfg.iowait_mode)
    pic_iowait();

  outb(PIC_PRIMARY_DATA_PORT,
       cfg.primary_offset); /* Interrupts offset [primary] */
  if (cfg.iowait_mode)
    pic_iowait();
  outb(PIC_SECONDARY_DATA_PORT,
       cfg.secondary_offset); /* Interrupts offset [secondary] */
  if (cfg.iowait_mode)
    pic_iowait();

  outb(PIC_PRIMARY_DATA_PORT,
       cfg.sconn_irq_line); /* IRQ lines where secondary controller is
                               connected [primary] */
  if (cfg.iowait_mode)
    pic_iowait();
  outb(PIC_SECONDARY_DATA_PORT,
       cfg.sec_irq_line_num); /* IRQ line number where secondary
                                 controller is connected [secondary] */
  if (cfg.iowait_mode)
    pic_iowait();

  outb(PIC_PRIMARY_DATA_PORT,
       (uint8_t)(0 | ((cfg.mp_mode & 0x1) << 0) | ((cfg.auto_eoi & 0x1) << 1) |
                 ((cfg.bufmode & 0x1) << 2) | ((cfg.ps_bufmode & 0x1) << 3) |
                 ((cfg.sfn_mode & 0x1) << 4)));
  if (cfg.iowait_mode)
    pic_iowait();
  outb(PIC_SECONDARY_DATA_PORT,
       (uint8_t)(0 | ((cfg.mp_mode & 0x1) << 0) | ((cfg.auto_eoi & 0x1) << 1) |
                 ((cfg.bufmode & 0x1) << 2) | ((cfg.ps_bufmode & 0x1) << 3) |
                 ((cfg.sfn_mode & 0x1) << 4)));
  if (cfg.iowait_mode)
    pic_iowait();

  outb(PIC_PRIMARY_DATA_PORT, primary_mask);
  if (cfg.iowait_mode)
    pic_iowait();
  outb(PIC_SECONDARY_DATA_PORT, secondary_mask);
  if (cfg.iowait_mode)
    pic_iowait();

  current_pic_config = cfg;
}

void pic_eoi(uint32_t int_num) {
  if (current_pic_config.auto_eoi)
    return;

  if (int_num >= 40) {
    outb(0xA0, 0x20);
  }
  outb(0x20, 0x20);
}