#pragma once

#define nil ((void *)0)

// #define gasm(...) #__VA_ARGS__
#define pgasm(...) _asm volatile(__VA_ARGS__)
#define gasm(...) _asm(__VA_ARGS__)