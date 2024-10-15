[bits 32]

KMAIN equ 0x8000

start:
    jmp Start_Long


;%include "Source/Loader/gdt.s"

begin:
    
    call check_cpuid
    jz nocpuid
    ;call check_long
    
    mov edi, 0x1000 ; page table starts here
    mov cr3, edi

    mov dword [edi], 0x2003
    add edi, 0x1000
    mov dword [edi], 0x3003
    add edi, 0x1000
    mov dword [edi], 0x4003
    add edi, 0x1000

    ; loop 512 times
    mov ebx, 0x00000003
    mov ecx, 512

    setEntry:
        mov dword [edi], ebx
        add ebx, 0x1000
        add edi, 8
        loop setEntry
    
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax
    
    mov ecx, 0xc000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    
    ;call edit_gdt
    
    ;lgdt [gdt_desc]

[bits 32]
Start_Long:
    ;mov rax, KMAIN
    call 0x08:KMAIN
    jmp $

[bits 32]
check_cpuid:
    pushfd                               ;Save EFLAGS
    pushfd                               ;Store EFLAGS
    xor dword [esp],0x00200000           ;Invert the ID bit in stored EFLAGS
    popfd                                ;Load stored EFLAGS (with ID bit inverted)
    pushfd                               ;Store EFLAGS again (ID bit may or may not be inverted)
    pop eax                              ;eax = modified EFLAGS (ID bit may or may not be inverted)
    xor eax,[esp]                        ;eax = whichever bits were changed
    popfd                                ;Restore original EFLAGS
    and eax,0x00200000                   ;eax = zero if ID bit can't be changed, else non-zero
    ret

check_long:
    mov eax, 0x80000001     ; Extended Processor Info and Feature Bits
    cpuid                   ; Execute CPUID instruction
    test edx, 1 << 29       ; Check if bit 29 (Long Mode) is set
    jz NoLongMode         ; If not set, long mode is not supported
    
NoLongMode:
    mov al, 'E'
    mov [0xb8000], al
    jmp $

nocpuid:
    jmp $