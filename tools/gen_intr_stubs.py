COUNT_INTERRUPTS = 129
TEMPLATE = 'extern void intr{number}(); idt_register_llhandler({number}, (uint32_t)intr{number});'
OUTPUT_FILE = 'arch/x86/idt_handlers.h'

with open(OUTPUT_FILE, 'w') as f:
  for i in range (COUNT_INTERRUPTS):
    f.write(TEMPLATE.format(number=i))
    f.write('\n')
