#include "idt.h"
#include "ps2.h"

#include "../Video/vesa.h"

void init_buffer();
void WIN_SwitchFrame(uint16_t boundx_0, uint16_t boundy_0, uint16_t boundx_1, uint16_t boundy_1);
void WIN_DrawMouse();

int_queue_object int_queue[256];
mouse_dapacket packet;
uint8_t mouse_packet[3];
uint8_t mouse_dapack_cnt = 0;

// When an interrupt for the PS/2 data bus, first check if there is nothing on the queue. If not, add it to the queue and call it. Afterwards remove it.

void ps2_empty() {
    return;
}
// When the mouse interrupt is fired, it goes to a default hook insetad of a user. Mouse movement is controlled by drivers.


void ps2_int_finish() { // go back for more queud stuff

}


// begin mouse functionality
void mouse_wait_write() {
    while(!(inb(0x64) & 0x02))
    return;
}

void mouse_wait_read() {
    while(inb(0x64) & 0x01)
    return;
}

unsigned char mouse_read() {
    // Wait for the output buffer to be full
    mouse_wait_write();
    return inb(0x60);  // Read the mouse response
}

void mouse_write(unsigned char write) {
    // Wait for the input buffer to be ready
    mouse_wait_read();
    outb(0x64, 0xD4);  // Tell the controller we're sending data to the mouse
    mouse_wait_read();
    outb(0x60, write);  // Send the data to the mouse
}

// How the mouse works:
// Sends three IRQs, each containing one byte that we need. Three in all. After the three, assemble the packet and send it down to the handler.
void initMouse() {
    uint8_t status;
    //Enable the interrupts
    outb(0x64, 0xA8);
    mouse_wait_read();
    outb(0x64, 0x20);
    mouse_wait_write();
    status = inb(0x60);         
    status = (status | 2) ;
    mouse_wait_read();
    outb(0x64, 0x60);
    mouse_wait_read();
    outb(0x60, status);

    //mouse_write(0xF6);
    //mouse_read();

    mouse_write(0xF4);
    mouse_read();  //Acknowledge

    mouse_position.pos_x = 500;
    mouse_position.pos_y = 100;

    // DANGER: Increasing mouse scale seems to "drag" parts of the screen through the mask. Fun to play with but needs fixing.

    mouse_position.scale_x = 1;
    mouse_position.scale_y = 1;

    // Fill mouse hooks, can de user-defined later

    mouse_right_hook = ps2_empty;
    mouse_middle_hook = ps2_empty;
    mouse_left_hook = ps2_empty;


    mouse_move_hook = ps2_empty;

    // Set sample rate for testing
    outb(0xE8, 0x3);
    mouse_wait_read();
    mouse_read();
}


// Primary mouse handler/driver. All rendering and movement is controlled by the driver, however buttons are hooked. Hover TBD.
// (0) Button left (1) Button right (2) Button middle (3) 1 (4) X-Axis sign (5) Y-Axis sign (6) X-Axis Overflow (7) Y-Axis Overflow

void mouse_handle() {
    // First, render the previous mask. After we handle all the events needed, we update the mouse mask before rendering the sprite.
    uint16_t start;
    
    
    mouse_mask_render();
    WIN_SwitchFrame(mouse_position.pos_x,mouse_position.pos_y,mouse_position.pos_x+(8 * mouse_position.scale_x),mouse_position.pos_y+(11 * mouse_position.scale_y));

    // if sign is 1 then minus
    short newpos_x = mouse_position.pos_x;
    short newpos_y = mouse_position.pos_y;
    uint8_t coef_x;
    uint8_t coef_y;

    if ((packet.flags & 0b00010000)) {
        coef_x = (0xff - (packet.mov_x));
        if (coef_x < 200) {
            newpos_x -= coef_x;
        }
    } else {
        coef_x = packet.mov_x;
        if (coef_x < 200) {
            newpos_x += coef_x;
        }
    }

    if ((packet.flags & 0b00100000)) {
        coef_y = (0xff - (packet.mov_y));
        if (coef_y < 200) {
            newpos_y += coef_y;
        }
    } else {
        coef_y = packet.mov_y;
        if (coef_y < 200) {
            newpos_y -= coef_y;
        }
    }

    if (newpos_x < 0) {
        newpos_x = 0;
    } else if (newpos_x >= WIN_WIDTH-8) {
        newpos_x = WIN_WIDTH - 8;
    }

    if (newpos_y < 0) {
        newpos_y = 0;
    } else if (newpos_y >= WIN_HEIGHT-11) {
        newpos_y = WIN_HEIGHT - 11;
    }
    mouse_position.pos_x = newpos_x;
    mouse_position.pos_y = newpos_y;
    mouse_move_hook();
    
    uint8_t combined = !(packet.mov_x & packet.mov_y & packet.flags);

    bool fire_left;
    bool fire_middle;
    bool fire_right;

    // After mouse has finsihed calcualting, hook into buttons
    if ((packet.flags & 0b00001000) && !((packet.flags & 0b11000000))) { // If no invalid packets were sent, update
        if (!(combined == 0xff) && packet.mov_x == 0 && packet.mov_y == 0) { // i all three bytes were maxed out, invalid

            if ((packet.flags & 1) != mouse_position.button_left_pressed) {
                // check if button changed
                fire_left = true;
            }
            if ((packet.flags & 4) != mouse_position.button_middle_pressed) {
                // check if button changed
                fire_middle = true;
            }
            if ((packet.flags & 2) != mouse_position.button_right_pressed) {
                // check if button changed
                fire_right = true;
            }
        
            mouse_position.button_right_pressed = packet.button_right;
            mouse_position.button_middle_pressed = packet.button_middle;
            mouse_position.button_left_pressed = packet.button_left;

            if (fire_right && mouse_enabled) {
                mouse_right_hook();
            }
            if (fire_middle && mouse_enabled) {
                mouse_middle_hook();
            }
            if (fire_left && mouse_enabled) {
                mouse_left_hook();
            }
            fire_left = false;
            fire_middle = false;
            fire_right = false;
        }

    }

    WIN_DrawMouse();

    // To fix the mouse!
    if (!mouse_enabled) {
        return;
    }
    shell_tty_print("\n0b");
    for (int x=7;x>=0;x--) {
        if (packet.mov_x & (1 << x)) {
            shell_tty_print("1");
        } else {
            shell_tty_print("0");
        }
    }
    shell_tty_print("\n0b");
    for (int x=7;x>=0;x--) {
        if (packet.mov_y & (1 << x)) {
            shell_tty_print("1");
        } else {
            shell_tty_print("0");
        }
    }
    shell_tty_print("\n0b");
    for (int x=7;x>=0;x--) {
        if (packet.flags & (1 << x)) {
            shell_tty_print("1");
        } else {
            shell_tty_print("0");
        }
    }
    shell_tty_print("\n---------------");
    shell_memory_render();
    return;
}

void mouse_isr() {
    uint8_t status = inb(0x64);
    if (status & 0x20) { // Check if mouse data is available
        mouse_packet[mouse_dapack_cnt++] = inb(0x60);
        if (mouse_dapack_cnt == 3) {
            mouse_dapack_cnt = 0; // Reset packet cycle
            // Handle the packet (serialize into a neat format for ease of use)
            packet.mov_x = mouse_packet[1];
            packet.mov_y = mouse_packet[2];
            packet.flags = mouse_packet[0];

            packet.button_left = mouse_packet[0] & 1;
            packet.button_middle = mouse_packet[0] & 4;
            packet.button_right = mouse_packet[0] & 2;
            mouse_handle();
        }
    }
}

void ps2_int_process(uint8_t id) {
    if (id) {
        // Data is from the mouse
        mouse_isr();
        outb(0x20, 0x20); // EOI
        outb(0xa0, 0x20); // Second EOI for slave PIC
    } else {
        // Data is from the keyboard
        kbd_hook();
        outb(0x20, 0x20); // EOI
    }
}