#ifndef MYHEADER_H2
#define MYHEADER_H2

#include "../Include/stdint.h"

typedef struct {
    uint16_t offset_lo; // Address of function hook, low end
    uint16_t selector;  // Segment selector (0x8 for code seg)
    uint8_t always0;    // Reserved
    uint8_t flags;      // Flags
    uint16_t offset_hi; // Address of function hook, high end
} __attribute__((packed)) idt_gate;

typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_reg;


void (*ch0_hook)();
void (*err_hook)(int interrupt_num, int error_code);



// Define from assembly
extern "C" void div0();
extern "C" void doublefault();
extern "C" void kbd_stub();
extern "C" void mouse_stub();
extern "C" void pit_stub();

char error_list[32][32] = {
    "Division Error",
    "Debug Warning",
    "Non-maskable interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "what the heck?",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "...",
    "x87 Floating-Point Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception",
    "Virtualization Exception",
    "Control Protection Exception",
    "...",
    "Hypervisor Injection Exception",
    "VMM Communication Exception",
    "Security Exception",
    "...",
    "Tripple Fault",
    "FPU Error Interrupt"
};



idt_gate idt[256];

idt_reg idt_desc;

#endif  // End of include guard