#pragma once

void kloga(const char *file, int line, const char *loc, char *msg, ...);
#define klog(loc, msg, ...) kloga(__FILE__, __LINE__, loc, msg, ##__VA_ARGS__);
#define klogf(...) klog(__func__, ##__VA_ARGS__)
