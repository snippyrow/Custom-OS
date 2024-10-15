[bits 32]
; checks whether CPUID is avalible, found on osdever
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