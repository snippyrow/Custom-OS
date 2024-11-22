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
    char* title; // pointer
    uint8_t win_menu;
} __attribute__ ((packed));

// win_menu:
// 00000000
//      mMC
// m: minimize, M: maximize, C: close

struct window_mover_preview {
    uint16_t box_position_x;
    uint16_t box_position_y;
    uint16_t box_size_x;
    uint16_t box_size_y;
    uint16_t box_border_width;
    uint8_t box_color;
    bool box_hasRendered;
} __attribute__ ((packed));

window_root window_list[32];
window_mover_preview current_window_previewer;

uint8_t window_create();
void windows_init();

// Attach rendering functions
void WIN_FillRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t color);
void WIN_SwitchFrame(uint16_t boundx_0, uint16_t boundy_0, uint16_t boundx_1, uint16_t boundy_1);
void WIN_DrawString(uint16_t x, uint16_t y, uint16_t xscale, uint16_t yscale, char* str, uint8_t color);
void WIN_DrawChar(uint16_t x, uint16_t y, uint16_t xscale, uint16_t yscale, uint8_t character, uint8_t color, uint8_t bgcolor, bool clear);


uint16_t window_creation_offset_x;
uint16_t window_creation_offset_y;

uint16_t window_shadow_offset_x = 3;
uint16_t window_shadow_offset_y = 3;

uint16_t window_topbar_height = 30;
uint16_t window_border_width = 3;

uint8_t num_windows;
uint8_t new_preview_id;

uint16_t window_bounding_width = 5;
uint16_t desktop_padding = 5;

uint8_t window_mover_selection = 0xff;

uint8_t desktop_bg = 0xaf;
uint8_t window_root_bg = 0x1d; // window background (unless edited by user)
uint8_t window_border_color = 0x18;
uint8_t window_box_shadow = 0x11;