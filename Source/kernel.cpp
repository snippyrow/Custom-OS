#include "Interrupt/idt.cpp"
#include "Interrupt/pit.cpp"
#include "Video/vga.cpp"
#include "Shell/shell.cpp"
#include "Interrupt/error.cpp"
#include "Interrupt/sb16.cpp"

#include "Window/window.cpp"

int i=0;
void testbar() {
    return;
    i = i + 1;
    uint32_t begin = (WIN_HEIGHT - 5) * WIN_WIDTH;
    for (int x=0;x<WIN_WIDTH*5;x++) {
        WORK_BUFF[begin+x] = i;
    }
    WIN_SwitchFrame(0,WIN_HEIGHT-5,WIN_WIDTH,WIN_HEIGHT);
    return;
}

// For some reason interrupts cause a fault when triggered.. Probably for long mode
// Switch GDT to long struct
// Fix severe hardware issues, probably caused by extra video modes

// window functions arent fast enough and cause issues
// KVM Must be enabled for normal operation
// Mouse itnerferes with keyboard. Fix by doing some pre-processing and my own keyboard buffer

void mouse_middle_test() {
    shell_tty_print("Middle!");
    shell_tty_print("\n");
    shell_memory_render();
}

extern "C" void kmain() {
    init_buffer();
    WIN_RenderClear(0xb0);
    WIN_SwitchFrame_A();
    
    ch0_hook = testbar;
    kbd_hook = shell_kbd_hook;
    err_hook = error_handler;


    WORK_BUFF[0] = 0x2;

    initPIT(30);
    idt_install();
    initMouse();

    mouse_middle_hook = mouse_middle_test;

    sb_init();            // Initialize the Sound Blaster
    sb_play_tone(1000);  // Play a 1000 Hz tone

    shell_tty_print("Type 'help' for a list of commands.\n");
    shell_tty_print(shell_prompt);
    shell_memory_render();

    //window_render(w_id);

    sti();

    mouse_left_hook = window_left;
    
    while(1){}

    return;
}

// The things I need to do:

// Write an ATA driver**
// Get 64-bit code working well (?)
// Write a sound driver
// Memory Allocator (done)
// Filesystem
// PS/2 Mouse** (done)
// Windowing
// Multi-processing (?)

// Boot sequence:
// use a command to list bootable addresses, then boot into that address with another command (or alias)
// For this purpose use a good ATA driver

/*
The primary issue is that calling a frame switch too often can drop interrupts.
Required changes:
    - Re-implement switch frame with bounds, so not the entire frame is re-drawn, only specific windows.
    - Enable preemptive multitasking
*/