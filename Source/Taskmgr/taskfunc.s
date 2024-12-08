section .bss

section .text


[extern tasklist]
[extern process_check]
[extern foundTask]
[extern foundIndex]
[extern state]
[extern ret_addr]
[extern taskstates]
[extern processEnabled]
[extern var_esp]
[extern tmp_esp]
[extern concurrent_PID]

[global proc_start]

jump_addr:
    resb 4

proc_start:
    ; Back up CPU state
    ; back up eax, ebx, ecx, edx, esp, eflags, edi, esi (in order)
    mov [state], eax
    mov [state + 4], ebx
    mov [state + 8], ecx
    mov [state + 12], edx
    ; add 4 to the stack ptr, to before interrupt
    mov ebx, esp
    add ebx, 12
    mov [state + 16], ebx
    pushfd
    pop ebx
    mov [state + 20], ebx
    mov [state + 24], edi
    mov [state + 28], esi

    ; back up ret addr
    pop eax
    mov [ret_addr], eax
    sub esp, 4
    ; If there is a process ongoing, update the esp too
    ; Step back 12 bytes to leave interrupt IRET

    ; If the process manager is disabled, return
    mov eax, [processEnabled]
    cmp eax, 0
    je endl

    ; Back up the esp if there is an ongoing process
    mov eax, [concurrent_PID]
    cmp eax, 0
    je next

    mov eax, esp
    add eax, 12
    mov [tmp_esp], eax
    
next:
    call process_check
    mov eax, [foundTask]
    cmp eax, 0
    je endl ; exit if no new process found, resume old one

    ; now alter return addr
    ; Since we technically came from the interrupt directly, modify the iret addr

    ; restore registers
    mov eax, [foundIndex]
    mov ebx, 20
    mul ebx ; find offset by size of task_object, esp is the first uint32_t. This multiples eax
    add eax, tasklist ; eax has ptr to the current state struct to be restored

    mov ebx, [eax + 4] ; offset for ret_addr, load the stack
    mov [jump_addr], ebx
    mov esp, [eax]
    

    ; now load state for process
    mov eax, [foundIndex]
    mov ebx, 32
    mul ebx
    add eax, taskstates

    ; mov esp, [eax + 16]
    ; eflags
    mov edx, [eax + 20]
    push edx
    popfd
    ; now do edx
    mov edx, [eax + 12]

    mov edi, [eax + 24]
    mov esi, [eax + 28]
    mov eax, [eax]

    ; Adding 12 is not needed, as we already modified it

    mov al, 0x20
    out 0x20, al
    sti
    jmp [jump_addr]

endl:
; EOI for the PIT interrupt
    mov al, 0x20
    out 0x20, al
    iret

teststr:
    db 10,"Done!",0