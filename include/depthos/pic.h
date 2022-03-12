#pragma once

#include <depthos/stdtypes.h>

typedef struct pic_config {
  /**
   * Dirty old mask flag
   *  - If cleared then old mask will be cleared and will be replaced to new
   * when config will load
   *  - If setted then new mask only does OR operations with old mask
   */
  bool dirty_mask;

  /**
   * IOwait mode flag
   *  - If setted then all PIC API functions will use iowait mode after every
   * PIC command execution
   *  - If cleared then PIC API functions won't work in this mode
   * NOTE: It must be setted to true, if you machine is old
   */
  bool iowait_mode;
  /**
   * Speciall Fully Nested Mode (SFNM) flag
   *  - If setted then PIC will work in SFNM
   *  - If cleared then PIC won't work in this mode
   * NOTE: Recommended set it to false
   */
  bool sfn_mode;
  /**
   * Buffered mode
   *  - If setted then PIC will work in the buffered mode
   *  - If cleared then PIC won't work in this mode
   * NOTE: It must be setted to false for the Intel ICH4
   */
  bool bufmode;
  /**
   * Primary-secondary (master-slave) buffered mode
   *  - If setted then PIC will work in the primary-secondary buffered mode
   *  - If cleared then PIC won't work in this mode
   * NOTE: Should always be setted to false on the Intel architecture
   */
  bool ps_bufmode;
  /**
   * Auto EOI
   *  - If setted then PIC will send the EOI automatically after the interrupt
   * call
   *  - If cleared then PIC won't send the EOI automatically after the interrupt
   * call
   */
  bool auto_eoi;
  /**
   * Microprocessor mode
   *  - If setted then PIC will work in the microprocessor mode
   *  - If cleared then PIC won't work in this mode
   * NOTE: Must be be setted to true on the Intel architecture
   */
  bool mp_mode;

  /**
   * Offest of interrupts in the IDT, which controled by primary controller
   * NOTE: Recommended set it to 0x20 on the Intel architecture
   */
  uint8_t primary_offset;
  /**
   * Offeset of interrupts in the IDT, which controled by secondary controller
   * NOTE: Recommended set it to 0x28
   */
  uint8_t secondary_offset;
  /**
   * IRQ line number where secondary controller is connected
   * NOTE: Must be setted to 0x2 on the PC
   */
  uint8_t sec_irq_line_num;
  /**
   * IRQ lines where secondary controller(s) is connected
   * Must be setted to 0x4 on the PC
   */
  uint8_t sconn_irq_line;
  /**
   * Enabled IRQ's bitmask
   */
  uint16_t mask;
} pic_config_t;

void pic_init(struct pic_config cfg);
void pic_eoi(uint32_t int_num);