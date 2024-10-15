%macro print_str 1
    mov si, %1
    call p_start
%endmacro

; 0, 0, 2, 63, KERNEL
%macro disk_read 6
    pusha
    ; macro to read the primary drive to memory
    ; (1) cylinder, (2) head, (3) start, (4) #num, (5) BX buffer pointer, (6) ES

    ; Load pointer
    mov ax, %5
    mov bx, ax
    mov ax, %6
    mov es, ax

    ; Load reading arguments
    mov al, %4
    mov ch, %1
    mov cl, %3
    mov dh, %2
    mov dl, 0x80 ; primary hard disk

    mov ah, 2 ; BIOS code
    int 0x13

    mov ax, 0
    mov es, ax

    jc mem_read_err ; carry set if error

    popa
%endmacro