#pragma once

#include <heap.h>
#include <memory.h>

#define iret asm volatile("iret")

#define PROC_ALIVE 1
#define PROC_KILL 0

typedef struct __stdkernel_proc {
	struct __stdkernel_proc *prev;
	char* label;
	uint32_t pid;
	uint32_t esp;
	uint32_t sstop;
	uint32_t ip;
	uint32_t cr3;
	uint32_t state;

	void (*notify)(int);

	struct __stdkernel_proc *next;
}kproc_t;

kproc_t* createProcess(char *name,uint32_t addr);
int addProcess(kproc_t *proc);

void __prockill();
void __execProcess(kproc_t* proc);
// void __execProcessId(uint32_t pid);
void execProcess();

void __notified(int);


void tasks_run();
