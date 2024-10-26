#define SB_BASE 0x220 // Default base address for Sound Blaster
#define SB_DSP 0x22C  // DSP port

// Function to write to Sound Blaster
void sb_write(unsigned char reg, unsigned char value) {
    outb(SB_BASE + reg, value);
}

// Function to read from Sound Blaster
unsigned char sb_read(unsigned char reg) {
    return inb(SB_BASE + reg);
}

// Function to initialize the Sound Blaster
void sb_init() {
    sb_write(0x20, 0x01); // Reset the DSP
    //delay here
    sb_write(0x22, 0x01); // Set the DSP version
}

void sb_play_tone(int frequency) {
    // Calculate the period in samples
    int period = 1193180 / frequency; // 1193180 is the standard frequency for the timer

    sb_write(0x40, period & 0xFF);      // Send low byte
    sb_write(0x40, (period >> 8) & 0xFF); // Send high byte
    sb_write(0xC0, 0x01);                // Set the channel to square wave
    sb_write(0xC0, 0x00);                // Enable channel
}

// Function to stop the sound
void sb_stop() {
    sb_write(0xC0, 0x00); // Disable channel
}