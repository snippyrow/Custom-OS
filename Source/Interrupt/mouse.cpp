#include "mouse.h"

void mouse_wait_write() {
    while((inb(0x64) & 2) != 0);
    return;
}

void mouse_wait_read() {
    while((inb(0x64) & 1) == 0);
    return;
}


void initMouse() {
    
    

}

mouse_dapacket mouse_read() {
    mouse_dapacket packet;
    uint8_t mouse_bytes[3];
    for (int i=0;i<3;i++) {
        mouse_bytes[i] = inb(0x60);
    }
    packet.pos_x = mouse_bytes[1];
    packet.pos_y = mouse_bytes[2];
    packet.button_left = mouse_bytes[0] & 1;
    packet.button_middle = mouse_bytes[0] & 4;
    packet.button_right = mouse_bytes[0] & 2;
    return packet;
}