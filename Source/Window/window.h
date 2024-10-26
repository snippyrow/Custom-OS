struct window_root {
    uint16_t win_pos_x;
    uint16_t win_pos_y;
    uint16_t win_size_x;
    uint16_t win_size_y;
    uint8_t z_index;
    uint8_t win_id;
    bool win_visible;
    bool win_draggable;
    bool win_sizable;
    char title[64];
} __attribute__ ((packed));

window_root window_list[32];

uint8_t window_create();
void windows_init();