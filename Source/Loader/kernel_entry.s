[bits 32]
[extern kmain]

; shared labels here
[global div0]
[global doublefault]
[global kbd_stub]
[global mouse_stub]
[global pit_stub]

[extern isr_ghandler]
[extern kbd_ghandler]
[extern mouse_ghandler]
[extern pit_ghandler]

kernel_begin:
    mov al, 0xc
    mov [0xa0000], al
    
    call kmain
    jmp $

isr_stub:
    call isr_ghandler
    add esp, 8
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