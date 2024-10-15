#include "../Include/Util.h"

void initPIT(int frequency) { // frequency is in hz
    uint32_t divisor = 1193182/frequency; // Divisor must be a maximum of 65535, or 0xffff. Frequency must be 18.2hz or higher.

    // Send command byte to PIT
    outb(0x43, 0x36); // Binary, mode 3, low/high byte

    // Send the low byte
    outb(0x40, divisor & 0xFF);
    // Send the high byte
    outb(0x40, (divisor >> 8) & 0xFF);
    return;
}