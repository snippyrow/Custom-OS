#include "idt.h"
#include "../Include/Util.h"
#include "../Include/Util.cpp"
#include "ps2.cpp"


void set_idt_gate(uint8_t gate, uint32_t hook) {
    idt[gate].offset_lo = low_16(hook);
    idt[gate].offset_hi = high_16(hook);
    idt[gate].selector = 0x8;
    idt[gate].flags = 0x8e; // flags always the same
    idt[gate].always0 = 0;
}

extern "C" void isr_ghandler(int int_no, int err_code) {
    err_hook(int_no, err_code);
    return;
}

extern "C" void pit_ghandler() {
    ch0_hook();
    outb(0x20, 0x20); // EOI
    return;
}

extern "C" void kbd_ghandler() {
    ps2_int_process(0);
    ps2_int_finish();
    //outb(0x20, 0x20); // EOI
    return;
}

extern "C" void mouse_ghandler() {
    ps2_int_process(1);
    ps2_int_finish();
    //outb(0x20, 0x20); // EOI
    //outb(0xa0, 0x20); // Second EOI for slave PIC
    return;
}

// unused for now
void syscall_handler() {
    //iret_gen();
}



// Add required fields
void idt_install() {

    set_idt_gate(0,(uint32_t)div0);
    set_idt_gate(8,(uint32_t)doublefault);

    set_idt_gate(32, (uint32_t)proc_start); // This is the system timer and MUST be enabled, or else we receive double/tripple faults
    set_idt_gate(33,(uint32_t)kbd_stub);

    set_idt_gate(44,(uint32_t)mouse_stub);

    set_idt_gate(0x80, (uint32_t)syscall_gen);

    // blank functions



    // Re-map the master & slave PIC. How does it work?
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    // ICW2
    outb(0x21, 0x20);
    outb(0xA1, 0x28);

    // ICW3
    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    // ICW4
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    // OCW1
    outb(0x21, 0x0);
    outb(0xA1, 0x0);

    outb(0x21, 0b11111000); // only unmask PIT, Cascade PIC and keyboard (IRQ0 and IRQ1 and IRQ2)
    outb(0xA1, 0b11101111); // Only unmask PS/2 mouse


    idt_desc.base = (uint32_t) &idt;
    idt_desc.limit = 0xff * sizeof(idt_gate) - 1;
    asm volatile("lidt (%0)" : : "r" (&idt_desc));

    
}