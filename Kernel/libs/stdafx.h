#pragma once


// #define gasm(...) #__VA_ARGS__
#define pgasm(...) _asm volatile(__VA_ARGS__)
#define gasm(...) _asm(__VA_ARGS__)

#define packed __attribute__((packed))

//void syscall(int num) { gasm("int %0h" : : "M" (num) : ); }

// #define tostr(arg) #arg

// #define intr(num) asm("int $num" );
// or
// #define intr(num) gasm("int $##num##");

void intr(int num) {
	gasm("int %0" : : "r" (num) :);
}
