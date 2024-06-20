#pragma once

///// General settings /////

#define CONFIG_SYS_LINUX_COMPAT
// #define CONFIG_SYS_TRACE
#define CONFIG_TRACE_TASK_INFO
// #define CONFIG_TEST_MODE
// #define

///// Scoped Logging /////

// #define CONFIG_LOG_ALLOCS 1
// #define CONFIG_HEAP_LOG_ENABLE 1
#define CONFIG_ATA_LOG_ENABLE 0
#define CONFIG_EXT2_LOG_ENABLE 0
// #define CONFIG_VMM_LOG_ENABLE
#define KLOG_ENABLED 1
#define KLOG_PRINT_LOC 1

///// Scoped debugging /////

// #define CONFIG_SYS_DEBUG(r) 0

// #define CONFIG_INTERRUPTS_DEBUG
// #define CONFIG_SPECIFIC_INTERRUPTS_DEBUG(i) (i == 0x20 + 15 || i == 0x20 + 14)
// #define CONFIG_SPECIFIC_INTERRUPTS_DEBUG(i) (i != 0x20 & i != 0x30)
// #define CONFIG_SPECIFIC_INTERRUPTS_DEBUG(i) (i != 0x20)
// #define CONFIG_SERIAL_DEBUG


#define CONFIG_VMA_MAP_WHOLE
