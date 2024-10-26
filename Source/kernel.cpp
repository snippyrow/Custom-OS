#include "Include/stdint.h"
#include "Interrupt/idt.cpp"
#include "Include/Util.h"
#include "Interrupt/pit.cpp"
#include "Video/vga.cpp"
#include "Drivers/ATA.cpp"
#include "Shell/shell.cpp"
#include "Interrupt/error.cpp"
#include "Interrupt/sb16.cpp"

int i=0;
void testbar() {
    i = i + 1;
    uint32_t begin = (WIN_HEIGHT - 5) * WIN_WIDTH;
    for (int x=0;x<WIN_WIDTH*5;x++) {
        WORK_BUFF[begin+x] = i;
    }
    WIN_SwitchFrame(0,WIN_HEIGHT-5,WIN_WIDTH,WIN_HEIGHT);
    return;
}

void kkk() {
    uint8_t scan = inb(0x60);
    WIN_RenderClear(0xc);
    WIN_SwitchFrame_A();
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

    /*
    for (int x=0;x<144*144;x++) {
        //*(int *)(0x60000 + x) = x/144;
    }

    for (int x=0-144;x<144;x++) {
        for (int y=-144;y<144;y++) {
            WORK_BUFF[(y*WIN_WIDTH)+x] = FONT_BUFF[(y*144)+x];
        }
    }

    WIN_FillRect(0,500,WIN_WIDTH,550, 0xc);
    WIN_SwitchFrame_A(); // clear blue
    while(1){}
    */
    //*(uint8_t *)0xa0000 = 0xc;
    
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

    uint8_t ATA_Data[512] = {0x69, 0x42};
    //ATA_Read(0,1,ATA_Data);

    uint8_t tnum;
    for (int x=0;x<256;x++) {
        tnum = ATA_Data[x];
        uint16_t scale = 2;
        uint16_t xC = 25 + ((x/16) * 24 * scale);
        uint16_t yC = 25 + ((x%16) * 10 * scale);
        char* str = hex64_str(tnum);
        
        if (tnum <= 0xf) {
            str[1] = str[0];  // Shift the first character to the second position
            str[0] = '0';     // Add '0' at the first position
            str[2] = '\0';    // Null-terminate the string
        }
        for (int i=0;i<2;i++) {
            //WIN_DrawChar(xC+(i*16), yC, scale, scale, str[i], 0xf);
        }
        free(*str,32);
    }
    shell_tty_print(shell_prompt);
    shell_memory_render();
    //WIN_SwitchFrame();
    sti();
    
    while(1){}

    return;
}

// The things I need to do:

// Write an ATA driver (?)
// Get 64-bit code working well (?)
// Write a sound driver
// Memory Allocator (done)
// Filesystem
// PS/2 Mouse**

/*
The primary issue is that calling a frame switch too often can drop interrupts.
Required changes:
    - Re-implement switch frame with bounds, so not the entire frame is re-drawn, only specific windows.
    - Enable preemptive multitasking
*/