ROOT equ 0x700000


; Start by defining a file header
dw 0x66BB            ; signature
dd 0x700000          ; root address for load
dd (main - 0x700000) ; starting execution offset

[bits 32]
[org 0x700000]

cnt:
    db 0

main:
    ; Move arguments for printing a string
loop:
    mov ebx, startstring
    mov eax, 0x10
    int 0x80

    xor eax, eax
    int 0x80 ; exit syscall (0), kills process

    jmp $

startstring:
    db 10,"Hello world",0