#include "Interrupt/idt.cpp"
#include "Interrupt/pit.cpp"
#include "Video/vga.cpp"
#include "Shell/shell.cpp"
#include "Interrupt/error.cpp"
#include "Interrupt/sb16.cpp"

#include "Window/window.cpp"

int i=0;
void testbar() {
    //return;
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
    return;
    shell_tty_print("Middle!");
    shell_tty_print("\n");
    shell_memory_render();
}

extern "C" void kmain() {
    init_buffer(); // Initialize framebuffers
    WIN_RenderClear(shell_bg);
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

    // Do a basic intro with some information

    char* res_x = int64_str(WIN_WIDTH);
    char* res_y = int64_str(WIN_HEIGHT);
    char* res_bpp = int64_str(WIN_BPP);
    shell_tty_print("VARA Dev Shell V2.0\n\nRes=");
    shell_tty_print(res_x);
    shell_tty_print("x");
    shell_tty_print(res_y);
    shell_tty_print(" (");
    shell_tty_print(res_bpp);
    shell_tty_print(" bits per pixel)\n");
    shell_tty_print("Currently running a CFAT32 partition.");
    shell_tty_print("\nType 'help' for a list of commands.\n/");
    shell_tty_print(shell_dir_name);
    shell_tty_print("/ ");
    shell_memory_render();

    free(*res_x,32);
    free(*res_y,32);
    free(*res_bpp,32);

    sti();

    


    return;
}

// The things I need to do:
// (in order of importance)

// Fix backend (?)
// Write an ATA driver**
// Improve disk reading using BIOS (?)
// Filesystem
// Multi-processing and userland
// Window Z-index functionality
// Write a sound driver
// Get 64-bit code working well (?)

// Boot sequence:
// use a command to list bootable addresses, then boot into that address with another command (or alias)
// For this purpose use a good ATA driver
// 

// Window todo:
// Get a good mouse cursor
// Window move preview

// NOTE:
// Memory is getting pretty close now, be careful

// Emergancy Todo before anything!
// Find out why it wont boot on desktop

/*
The primary issue is that calling a frame switch too often can drop interrupts.
Required changes:
    - Re-implement switch frame with bounds, so not the entire frame is re-drawn, only specific windows.
    - Enable preemptive multitasking
*/