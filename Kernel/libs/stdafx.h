#pragma once

#define nil ((void *)0)


// #define gasm(...) #__VA_ARGS__
#define pgasm(...) _asm volatile(__VA_ARGS__)
#define gasm(...) _asm(__VA_ARGS__)

//void syscall(int num) { gasm("int %0h" : : "M" (num) : ); }


#define intr(num) gasm("int $" ## #num);
// or
// #define intr(num) gasm("int $##num##");