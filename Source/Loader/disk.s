disk_reset: ; issue a disk reset
    mov ah, 0
    int 0x13
    ret

disk_err_str:
    db "Error reading disk [0x80]. Stopping.",10,13,0

mem_read_err:
    print_str disk_err_str
    jmp $