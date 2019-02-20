%macro ISR_NOERRCODE 1
[GLOBAL isr%1]
isr%1:
    cli
    push byte 0
    push byte %1
    jmp isr_common_stub
%endmacro

%macro ISR_ERRCODE 1
[GLOBAL isr%1]
isr%1:
    cli
    push byte %1
    jmp isr_common_stub
%endmacro


ISR_NOERRCODE 0
ISR_NOERRCODE 1
; ... -> 31

[EXTERN isr_handler]

isr_common_stub:
    pusha ; save edi,esi,ebp,esp,ebx,edx,ecx,eax

    mov ax, ds
    push eax
    
    mov ax, 0x10 
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call isr_handler

    pop eax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    popa
    add esp,8
    sti
    iret