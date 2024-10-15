gdt_begin:
    gdt_null:
        dq 0
    gdt_code:
        dw 0xffff     ; Limit
        dw 0x0000     ; Base addr
        db 0x00       ; Base addr
        db 0b10011010 ; Access byte
        db 0b11001111 ; Flags & Limit
        db 0x00       ; Base
    gdt_data:
        dw 0xffff     ; Limit
        dw 0x0000     ; Base addr
        db 0x00       ; Base addr
        db 0b10010010 ; Access byte
        db 0b11001111 ; Flags & Limit
        db 0x00       ; Base
gdt_end:

gdt_desc:
    dw gdt_end-gdt_begin
    dq gdt_begin

code_seg equ gdt_code-gdt_begin
data_seg equ gdt_data-gdt_begin

[bits 32]
edit_gdt:
    mov [gdt_code + 6], byte 0b10101111
    mov [gdt_data + 6], byte 0b10101111
    ret
