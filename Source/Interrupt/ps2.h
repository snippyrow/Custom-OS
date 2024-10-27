void (*kbd_hook)();

void (*mouse_hook)();
void (*mouse_left_hook)();
void (*mouse_middle_hook)();
void (*mouse_right_hook)();

struct mouse_dapacket {
    uint8_t mov_x;
    uint8_t mov_y;
    uint8_t flags;
    bool button_left;
    bool button_middle;
    bool button_right;
} __attribute__ ((packed));

struct int_queue_object {
    uint8_t id; // 0 for keyboard, 1 for mouse, the rest unused
    uint8_t priority; // reserved

} __attribute__ ((packed));

struct mouse_properties {
    short pos_x;
    short pos_y;
    uint8_t scale_x;
    uint8_t scale_y;
    uint8_t button_right_pressed;
    uint8_t button_middle_pressed;
    uint8_t button_left_pressed;
} __attribute__ ((packed));

// When a mouse is moved, the back needs to be re-rendered. This is supposed to do that.
uint8_t mouse_mask[88];

// each pixel only needs three combinations, so 00-11. Four pixels per byte, (6*6)*4 = 144
// Sprite is 11x7, so
// 00 for no color, 01 for black, 10 for white
uint8_t mouse_sprite[22] = {
    0b01000000, 0b00000000,
    0b01010000, 0b00000000,
    0b01100100, 0b00000000,
    0b01101001, 0b00000000,
    0b01101010, 0b01000000,
    0b01101010, 0b10010000,
    0b01101010, 0b01010100,
    0b01100110, 0b01000000,
    0b01010001, 0b10010000,
    0b01000000, 0b01100100,
    0b00000000, 0b00010100
};

mouse_properties mouse_position;

void WIN_FillRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t color);