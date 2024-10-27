// Position x/y = absolute position of root window
// Size x/y = absolute size of root window
// Z-index = unused
// window id = window root ID
// win_visible = whether it can be rendered
// win_draggable = if the window is fixed or not
// win_sizable = if you can re-scale the window
// win_selected = If the window is selected and on-top
// win_active = Whether the window is being dragged/sized

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
    bool win_selected;
    bool win_active;
    char title[64];
} __attribute__ ((packed));

window_root window_list[32];

uint8_t window_create();
void windows_init();

// Attach rendering functions
void WIN_FillRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t color);
void WIN_SwitchFrame(uint16_t boundx_0, uint16_t boundy_0, uint16_t boundx_1, uint16_t boundy_1);
void WIN_DrawString(uint16_t x, uint16_t y, uint16_t xscale, uint16_t yscale, char* str, uint8_t color);