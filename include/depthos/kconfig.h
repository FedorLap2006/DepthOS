#pragma once

// TODO: #define CONFIG_ENABLED

#define CONFIG_SYS_LINUX_COMPAT
// #define CONFIG_SYS_TRACE
#define CONFIG_TRACE_TASK_INFO
// #define CONFIG_TESTS_ENABLED
// #define

#ifdef DEBUG
#define KLOG_ENABLED 1
#endif
