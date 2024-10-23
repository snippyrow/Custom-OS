struct mouse_dapacket {
    uint8_t pos_x;
    uint8_t pos_y;
    bool button_left;
    bool button_middle;
    bool button_right;
} __attribute__ ((packed));
