#include "Include/stdint.h"
#include "Interrupt/idt.cpp"
#include "Include/Util.h"
#include "Interrupt/pit.cpp"
#include "Video/vga.cpp"
#include "Drivers/ATA.cpp"
#include "Shell/shell.cpp"
#include "Interrupt/error.cpp"

int i=0;
void empty() {
    i = i + 1;
    for (int x=0;x<WIN_WIDTH*5;x++) {
        WORK_BUFF[x] = i;
    }
    int ie = 0;
    if (i == 100) {
        for (int x=0;x<5;x++) {
            ie = 5/x;
        }
    }
    WIN_SwitchFrame();
    return;
}

// For some reason interrupts cause a fault when triggered.. Probably for long mode
// Switch GDT to long struct
// Fix severe hardware issues, probably caused by extra video modes

extern "C" void kmain() {
    init_buffer();
    WIN_RenderClear(0x0);
    //*(uint8_t *)0xa0000 = 0xc;
    
    ch0_hook = empty;
    kbd_hook = shell_kbd;
    err_hook = error_handler;

    initPIT(30);
    idt_install();

    uint8_t ATA_Data[512];
    ATA_Read(0,1,ATA_Data);

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
            WIN_DrawChar(xC+(i*16), yC, scale, scale, str[i], 0xf);
        }
        free(*str,32);
    }
    WIN_SwitchFrame();
    sti();
    

    
    while(1) {}
    return;
}

// The things I need to do:
// Write an ATA driver
// Get 64-bit code working well
// Write a sound driver
// Memory Allocator (done)