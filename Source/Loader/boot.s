[bits 16]
[org 0x7c00]

KERNEL equ 0x7e00

; Inclusions are from reference of compiler
; Reminder: using bios to read the disk is all fucked up on the desktop, use a builtin driver

%include "Source/Loader/mac.s"

start:

    print_str test_str
    call disk_reset

    disk_read 0, 0, 22, 80, KERNEL >> 4, 0
    ; disk_read 0, 0, 63, (63+41), 0x7000, 0 ; (THIS WORKS FOR REAL HARDWARE!!!)
    disk_read 0, 1, 37, 41, 0x7000, 0
    ;disk_read 0, 1, 17, 41, 0x7000, 0 ; <--

    ; try reading using LBA (works on qemu, not real hardware..)
    ; mov ah, 0x42             ; Extended read
    ; mov dl, 0x80             ; Drive (0x80 = first HDD)
    ; mov si, disk_packet      ; Address of the Disk Address Packet
    ; int 0x13


    print_str success



    ; Now transfer to protected mode

    ; Enable A20 Line (thanks osdever)
    in al, 0x92
    or al, 2
    out 0x92, al

    mov ax, 0x4F02 ; VESA set video mode function
    mov bx, 0x105
    int 0x10
    
    ; The VESA information is loaded into address 0x6000, since the binary cannot use references til kernel_entry
    ; Now query for VBE Mode Information
    mov ax, 0x4F01
    mov cx, 0x105
    mov di, 0x1000 ; Address to store the mode info structure
    int 0x10 ; Call BIOS
    
    ; Check if the function succeeded
    cmp ax, 0x004F
    jne vid_err ; Jump if function failed


    print_str other_str
    print_str success
    
    cli
    xor ax, ax
    mov ds, ax ; clear ds
    lgdt [gdt_desc]

    mov eax, cr0
    or eax, 1
    mov cr0, eax
    
    jmp code_seg:M32_Start

[bits 32]
M32_Start:
    ; Point registers towards our new GDT

    mov ax, data_seg
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov esp, 0x90000 ; set stack far away, inside the data segment


    jmp code_seg:KERNEL

    jmp $


test_str:
    db "Reading disk..",10,13,0
other_str:
    db "Now testing 32/64-bit operations..",10,13,0
vid_err_str:
    db "Error requesting video info. Stopping.",10,13,0
success:
    db "Success.",10,13,0

vid_err:
    print_str vid_err_str
    jmp $


; data structures and functions
%include "Source/Loader/gdt.s"

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

disk_reset: ; issue a disk reset
    mov ah, 0
    int 0x13
    ret

disk_err_str:
    db "Error reading disk [0x80]. Stopping.",10,13,0

mem_read_err:
    print_str disk_err_str
    jmp $

%macro print_str 1
    mov si, %1
    call p_start
%endmacro

disk_packet:
    db 0x10              ; Packet size (16 bytes)
    db 0x00              ; Reserved
    dw 41                ; Number of sectors to read
    dw 0x0000            ; Destination offset
    dw 0x7000            ; Destination segment
    dq 79                ; LBA address of the first sector

times 510-($-$$) db 0
dw 0xaa55

;
FAT:
    resb 128




; TODO List
; Load protected mode
; Enable paging
; Enable 64-bit mode before using C/detect CPUID