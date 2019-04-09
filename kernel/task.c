#include "task.h"

uint32_t curPid = 0;
struct __stdkernel_proc *curProc = 0;

void __notified(int) {  }

kproc_t createProcess(char* name,uint32_t addr) {
	struct __stdkernel_proc *p = (kproc_t*)malloc(sizeof(kproc_t));

	memset(p,0,sizeof(kproc_t));

	p->name = name;
	p->pid = curPid++;
	p->state=PROC_ALIVE;
	p->notify = __notified;
	p->ip = addr;
	p->esp = (uint32_t)malloc(4096);
	asm volatile("mov %%cr3 %%eax",:"=a"(p->cr3));
	uint32_t* stack = (uint32_t *)(p->esp + 4096);
	p->sstop = p->esp;
#define push(val,_) *--stack=val
	push(0x00000202,"eflags");
	push(0x8,"cs");
	push((uint32_t)addr,"ip");
	push(0,"eax");
	push(0,"ebx");
	push(0,"ecx");
	push(0,"edx");
	push(0,"esi");
	push(0,"edi");
	push(p->esp + 4096,"ebp");
	push(0x10,"ds");
	push(0x10,"fs");
	push(0x10,"es");
	push(0x10,"gs");
#undef push
	p->esp = (uint32_t)stack;
	return p;	
}

int addProcess(kproc_t *p) {
	p->next = c->next;
	p->next->prev = p;
	p->prev = c;
	c->next = p;
	return proc->pid;
}

void __execProcess(kproc_t *proc) {
	asm volatile("mov %%eax, %%esp": :"a"(proc->esp));
	asm volatile("pop %gs");
	asm volatile("pop %fs");
	asm volatile("pop %es");
	asm volatile("pop %ds");
	asm volatile("pop %ebp");
	asm volatile("pop %edi");
	asm volatile("pop %esi");
	asm volatile("pop %edx");
	asm volatile("pop %ecx");
	asm volatile("pop %ebx");
	asm volatile("pop %eax");
	asm volatile("iret");
}

void execProcess() { 
	__execProcess(curProc);
}

void tasks_run() {
	curProc = createProcess("main",(uint32_t)mainProc);
	curProc->next = curProc;
	curProc->prev = curProc;
}
