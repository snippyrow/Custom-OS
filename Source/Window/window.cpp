#include "window.h"

uint16_t window_creation_offset_x;
uint16_t window_creation_offset_y;

uint8_t new_id;

void windows_init() {
    window_creation_offset_x = (WIN_WIDTH / 4);
    window_creation_offset_y = (WIN_HEIGHT / 4);
    new_id = 0;
    return;
}

uint8_t window_create(uint16_t win_pos_x, uint16_t win_pos_y, uint16_t win_size_x, uint16_t win_size_y, bool win_draggable = true, bool win_sizable = true) {
    // If too many windows have been created
    if (new_id >= 31) {
        return 0;
    }
    window_root new_window = {win_pos_x, win_pos_y, win_size_x, win_size_y, 5, new_id, true, win_draggable, win_sizable, "Test window"};
    window_list[new_id] = new_window;
    new_id ++;
}