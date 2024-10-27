#include "window.h"

uint16_t window_creation_offset_x;
uint16_t window_creation_offset_y;

uint16_t window_shadow_offset_x = 3;
uint16_t window_shadow_offset_y = 3;

uint16_t window_topbar_height = 30;
uint16_t window_border_width = 3;

uint8_t new_id;

uint16_t window_bounding_width = 5;
uint16_t desktop_padding = 5;

uint8_t window_mover_selection = 0xff;

uint8_t desktop_bg = 0xaf;

// Specs for windows:
// Each window has a "box of effect", or a bounding box, extending window_bounding_width pixels from the edge of the window. Includes shadow
// In a window, it has a drop shadow is selected, and none if not. To select it, merely select and re-render the window.


void windows_init() {
    window_creation_offset_x = (WIN_WIDTH / 4);
    window_creation_offset_y = (WIN_HEIGHT / 4);
    new_id = 0;
    return;
}

char* str_truncate(char* a, uint16_t width, uint16_t textscale) { // width in pixels
    uint16_t char_width = textscale * 8;           // Width of each character in pixels
    uint16_t max_chars = width / char_width;       // Maximum number of characters that can fit in the given width

    // Truncate the string if it exceeds the maximum allowed characters
    if (strlen(a) > max_chars) {
        a[max_chars] = '\0';                       // Truncate the string at max_chars
    }

    return a;
}

// 0x1d = window background
// 0x18 = window border
// 0x11 = window shadow
void window_render(uint8_t win_id) {
    window_root win_sel = window_list[win_id];
    if (win_sel.win_selected) {
        // Draw a dropshadow
        WIN_FillRect(
            win_sel.win_pos_x + window_shadow_offset_x,
            win_sel.win_pos_y + window_shadow_offset_y,
            win_sel.win_pos_x + window_shadow_offset_x + win_sel.win_size_x,
            win_sel.win_pos_y + window_shadow_offset_y + win_sel.win_size_y,
            0x11
        );
    };
    
    WIN_FillRect(
        win_sel.win_pos_x,
        win_sel.win_pos_y,
        win_sel.win_pos_x + win_sel.win_size_x,
        win_sel.win_pos_y + win_sel.win_size_y,
        0x18
    );

    // Draw primary window sections (root, topbar)

    WIN_FillRect(
        win_sel.win_pos_x + window_border_width,
        win_sel.win_pos_y + window_border_width,
        win_sel.win_pos_x + win_sel.win_size_x - window_border_width,
        win_sel.win_pos_y + window_topbar_height - window_border_width,
        0x1d
    );

    WIN_FillRect(
        win_sel.win_pos_x + window_border_width,
        win_sel.win_pos_y + window_border_width + window_topbar_height,
        win_sel.win_pos_x + win_sel.win_size_x - window_border_width,
        win_sel.win_pos_y + (win_sel.win_size_y - window_border_width),
        0x1d
    );

    char* win_title = str_truncate(win_sel.title, win_sel.win_size_x, 2);

    WIN_DrawString(win_sel.win_pos_x + window_border_width + 2, win_sel.win_pos_y + window_border_width + 2, 2, 2, win_title, 0x0);

    // Render bounding box
    WIN_SwitchFrame(win_sel.win_pos_x - window_bounding_width, win_sel.win_pos_y - window_bounding_width, win_sel.win_pos_x + win_sel.win_size_x + window_bounding_width, win_sel.win_pos_y + win_sel.win_size_y + window_bounding_width);
}

void window_erase(uint8_t win_id) {
    WIN_FillRect(
        window_list[win_id].win_pos_x - window_bounding_width,
        window_list[win_id].win_pos_y - window_bounding_width,
        window_list[win_id].win_pos_x + window_list[win_id].win_size_x + window_bounding_width,
        window_list[win_id].win_pos_y + window_list[win_id].win_size_y + window_bounding_width,
        desktop_bg
    );
    //WIN_SwitchFrame(0,0,WIN_WIDTH,WIN_HEIGHT);
    //return;
    WIN_SwitchFrame(
        window_list[win_id].win_pos_x - window_bounding_width,
        window_list[win_id].win_pos_y - window_bounding_width,
        window_list[win_id].win_pos_x + window_list[win_id].win_size_x + window_bounding_width,
        window_list[win_id].win_pos_y + window_list[win_id].win_size_y + window_bounding_width
    );
    return;
}

uint8_t window_create(uint16_t win_pos_x, uint16_t win_pos_y, uint16_t win_size_x, uint16_t win_size_y, bool win_draggable = true, bool win_sizable = true) {
    // If too many windows have been created
    if (new_id >= 31) {
        return 0xff;
    }

    // De-select all other windows in preperation
    for (int id=0;id<31;id++) {
        if (window_list[id].win_selected) {
            window_list[id].win_selected = false;
            window_erase(id);
            window_render(id);
        }
    }

    // Fill in our window
    window_root new_window = {win_pos_x, win_pos_y, win_size_x, win_size_y, 5, new_id, true, win_draggable, win_sizable, true, false, "Test window"};
    window_list[new_id] = new_window;
    // Now all windows are not selected, draw ours.
    window_render(new_id);
    new_id++;



    return new_id - 1;
}

void window_deselect(uint8_t win_id) {
    window_root win_sel = window_list[win_id];
    win_sel.win_selected = false;
    window_render(win_id);
}

// Left-click handelr for desktop/windows
// When an active window bar is clicked on, it causes it go to into "active" mode. Then you can move it by dragging. De-dragging fixes it.

void window_move(uint16_t startx, uint16_t starty, uint16_t endx, uint16_t endy, uint8_t win_id) {
    // Instead of absolute positioning, take a relative position for smoother travel.
    // First erase at the old position, then update the position and re-render
    window_erase(window_mover_selection); // Erase the window render BEFORE resetting position!

    short new_pos_x;
    short new_pos_y;

    if (endx < startx) {
        new_pos_x = window_list[win_id].win_pos_x - (startx - endx);
    } else {
        new_pos_x = window_list[win_id].win_pos_x + (endx - startx);
    }

    if (endy < starty) {
        new_pos_y = window_list[win_id].win_pos_y - (starty - endy);
    } else {
        new_pos_y = window_list[win_id].win_pos_y + (endy - starty);
    }

    if (new_pos_x <= desktop_padding) {
        new_pos_x = desktop_padding;
    } else if (new_pos_x >= (WIN_WIDTH - desktop_padding - window_list[win_id].win_size_x - window_bounding_width)) {
        new_pos_x = (WIN_WIDTH - desktop_padding - window_list[win_id].win_size_x - window_bounding_width);
    }

    if (new_pos_y <= desktop_padding) {
        new_pos_y = desktop_padding;
    } else if (new_pos_y >= (WIN_HEIGHT - desktop_padding - window_list[win_id].win_size_y - window_bounding_width)) {
        new_pos_y = (WIN_HEIGHT - desktop_padding - window_list[win_id].win_size_y - window_bounding_width);
    }

    window_list[win_id].win_pos_x = new_pos_x;
    window_list[win_id].win_pos_y = new_pos_y;

    window_render(window_mover_selection);
}

bool press_exists = false;
uint16_t mouse_start_x;
uint16_t mouse_start_y;

void window_left() {
    if (!press_exists) { // initiating a click, find affected windows
        press_exists = true;
        for (int win_id=0;win_id<31;win_id++) {
            // Now the selected window is found, check for a topbar intersection
            if (rect_collide(
                window_list[win_id].win_pos_x,
                window_list[win_id].win_pos_y,
                window_list[win_id].win_pos_x + window_list[win_id].win_size_x,
                window_list[win_id].win_pos_y + window_topbar_height + window_border_width,
                mouse_position.pos_x,
                mouse_position.pos_y
            )) {
                // If so, check if the window is already selected. If not, select it and wait for the next turn.
                if (window_list[win_id].win_selected) {
                    window_mover_selection = win_id;
                    mouse_start_x = mouse_position.pos_x;
                    mouse_start_y = mouse_position.pos_y;
                } else {
                    for (int id=0;id<31;id++) {
                        if (window_list[id].win_selected && id != win_id) {
                            window_list[id].win_selected = false;
                            window_erase(id);
                            window_render(id);
                        }
                        if (!window_list[id].win_selected && id == win_id) {
                            window_list[id].win_selected = true;
                            window_erase(id);
                            window_render(id);
                        }
                    }
                }
                break;
            }
        }
    } else {
        press_exists = false;
        // Check if we are ending a move
        if (window_mover_selection != 0xff) { // found a select
            window_move(mouse_start_x, mouse_start_y, mouse_position.pos_x, mouse_position.pos_y, window_mover_selection);
            window_mover_selection = 0xff;
        }
    }
}

void mouse_mask_update() {
    uint8_t m_scale_x = mouse_position.scale_x;
    uint8_t m_scale_y = mouse_position.scale_y;
    for (int x=mouse_position.pos_x;x<mouse_position.pos_x + (8 * m_scale_x);x++) {
        for (int y=mouse_position.pos_y;y<mouse_position.pos_y + (11 * m_scale_y);y++) {
            uint16_t cX = x - mouse_position.pos_x;
            uint16_t cY = y - mouse_position.pos_y;
            mouse_mask[((cY / m_scale_y) * 8) + (cX / m_scale_x)] = WORK_BUFF[(y * WIN_WIDTH) + x];
        }
    }
}

void mouse_mask_render() {
    uint8_t m_scale_x = mouse_position.scale_x;
    uint8_t m_scale_y = mouse_position.scale_y;
    for (int x=mouse_position.pos_x;x<mouse_position.pos_x + (8 * m_scale_x);x++) {
        for (int y=mouse_position.pos_y;y<mouse_position.pos_y + (11 * m_scale_y);y++) {
            uint16_t cX = x - mouse_position.pos_x;
            uint16_t cY = y - mouse_position.pos_y;
            WORK_BUFF[(y * WIN_WIDTH) + x] = mouse_mask[((cY / m_scale_y) * 8) + (cX / m_scale_x)]; // 0xb0
        }
    }
}

void WIN_DrawMouse() {
    // Fill the mouse mask before re-drawing over it
    mouse_mask_update();
    uint8_t m_scale_x = mouse_position.scale_x;
    uint8_t m_scale_y = mouse_position.scale_y;
    //WIN_FillRect(mouse_position.pos_x,mouse_position.pos_y,mouse_position.pos_x + 8,mouse_position.pos_y + 11, 0xc);
    for (int y = 0;y<(11 * m_scale_y);y++) {
        for (int x = 0;x<(8 * m_scale_x);x++) {
            uint16_t abs_pos_x = mouse_position.pos_x + x;
            uint16_t abs_pos_y = mouse_position.pos_y + y;
            uint8_t ptr = ((y / m_scale_y) * 8 + (x / m_scale_x)) / 4;
            uint8_t shift = (3-((x / m_scale_x)%4))*2;
            uint8_t digit = (mouse_sprite[ptr] & (3 << shift)) >> shift;
            if (digit == 1) {
                WORK_BUFF[abs_pos_y * WIN_WIDTH + abs_pos_x] = 0xf;
            } else if (digit == 2) {
                WORK_BUFF[abs_pos_y * WIN_WIDTH + abs_pos_x] = 0x0;
            }
        }
    }
    WIN_SwitchFrame(mouse_position.pos_x,mouse_position.pos_y,mouse_position.pos_x + (8 * m_scale_x),mouse_position.pos_y + (11 * m_scale_y));
}