[bits 16]
p_start:
    pusha
    mov ah, 0xe ; BIOS code to print a character
p_loop:
    lodsb
    cmp al, 0 ; check if null character
    je p_done
    int 0x10 ; fire BIOS code
    jmp p_loop
p_done:
    popa
    ret

