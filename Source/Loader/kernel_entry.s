[bits 32]
[extern kmain]

; shared labels here
[global div0]
[global doublefault]
[global kbd_stub]
[global mouse_stub]
[global pit_stub]
[global syscall_gen]
[global gen_stub]

[extern isr_ghandler]
[extern kbd_ghandler]
[extern mouse_ghandler]
[extern pit_ghandler]
[extern shell_tty_print]
[extern concurrent_PID]

[extern process_check]
[extern process_kill]
[global schedule_stub]

[extern shell_memory_render]

kernel_begin:
    mov al, 0xc
    mov [0xa0000], al
    
    call kmain
    jmp $

isr_stub:
    call isr_ghandler
    add esp, 8
    iret

schedule_stub:
    call process_check
    mov al, 0x20
    out 0x20, al
    iret

; syscall handler (better if written in asm)

syscall_gen:
    cmp eax, 0x10
    je syscall_print
    cmp eax, 0
    je syscall_exit
    iret

; Syscall for string printing
; Assume EBX has the pointer to a valid string
syscall_print:
    push ebx
    call shell_tty_print
    call shell_memory_render
    add esp, 4
    iret

; destroy current process and go back to a loop-check
syscall_exit:
    push dword [concurrent_PID]
    call process_kill
    add esp, 4
    iret


div0:
    mov eax, [esp]
    push eax
    push byte 0 ; Interrupt number argument
    jmp isr_stub

doublefault:
    mov eax, [esp]
    push eax
    push byte 8 ; Interrupt number argument
    jmp isr_stub

kbd_stub:
    call kbd_ghandler
    iret

mouse_stub:
    call mouse_ghandler
    iret

pit_stub:
    call pit_ghandler
    iret