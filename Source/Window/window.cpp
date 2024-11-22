#include "window.h"
#include "mouse.cpp"
#include "preview.cpp"

// Specs for windows:
// Each window has a "box of effect", or a bounding box, extending window_bounding_width pixels from the edge of the window. Includes shadow
// In a window, it has a drop shadow is selected, and none if not. To select it, merely select and re-render the window.


void windows_init() {
    window_creation_offset_x = (WIN_WIDTH / 4);
    window_creation_offset_y = (WIN_HEIGHT / 4);
    num_windows = 0;
    new_preview_id = 0;
    return;
}

char* str_truncate(char* a, uint16_t width, uint16_t textscale) { // width in pixels
    uint16_t char_width = textscale * 8;           // Width of each character in pixels
    uint16_t max_chars = width / char_width;       // Maximum number of characters that can fit in the given width

    // Truncate the string if it exceeds the maximum allowed characters
    if (strlen(a) > max_chars) {
        a[max_chars] = '\0'; // Truncate the string at max_chars
    }

    return a;
}

// 0x1d = window background
// 0x18 = window border
// 0x11 = window shadow
void window_render(uint8_t win_id) {
    window_root win_sel = window_list[win_id];
    // If the window is destroyed and can be overwritten
    if (!win_sel.win_active) {
        return;
    }
    if (win_sel.win_selected) {
        // Draw a dropshadow
        WIN_FillRect(
            win_sel.win_pos_x + window_shadow_offset_x,
            win_sel.win_pos_y + window_shadow_offset_y,
            win_sel.win_pos_x + window_shadow_offset_x + win_sel.win_size_x,
            win_sel.win_pos_y + window_shadow_offset_y + win_sel.win_size_y,
            window_box_shadow
        );
    };
    
    WIN_FillRect(
        win_sel.win_pos_x,
        win_sel.win_pos_y,
        win_sel.win_pos_x + win_sel.win_size_x,
        win_sel.win_pos_y + win_sel.win_size_y,
        window_border_color
    );

    // Draw primary window sections (root, topbar)

    WIN_FillRect(
        win_sel.win_pos_x + window_border_width,
        win_sel.win_pos_y + window_border_width,
        win_sel.win_pos_x + win_sel.win_size_x - window_border_width,
        win_sel.win_pos_y + window_topbar_height - window_border_width,
        window_root_bg
    );

    WIN_FillRect(
        win_sel.win_pos_x + window_border_width,
        win_sel.win_pos_y + window_border_width + window_topbar_height,
        win_sel.win_pos_x + win_sel.win_size_x - window_border_width,
        win_sel.win_pos_y + (win_sel.win_size_y - window_border_width),
        window_root_bg
    );

    char* win_title = str_truncate(win_sel.title, win_sel.win_size_x, 2);

    WIN_DrawString(win_sel.win_pos_x + window_border_width + 2, win_sel.win_pos_y + window_border_width + 2, 2, 2, win_title, 0x0);
    
    // Draw context menu (close, minimize, etc.)
    WIN_FillRect(
        win_sel.win_pos_x + win_sel.win_size_x - 30,
        win_sel.win_pos_y,
        win_sel.win_pos_x + win_sel.win_size_x,
        win_sel.win_pos_y + 30,
        window_border_color
    );
    WIN_FillRect(
        win_sel.win_pos_x + win_sel.win_size_x - (30 - window_border_width),
        win_sel.win_pos_y + window_border_width,
        win_sel.win_pos_x + win_sel.win_size_x - (window_border_width),
        win_sel.win_pos_y + 30 - window_border_width,
        window_root_bg
    );
    WIN_DrawChar(
        win_sel.win_pos_x + win_sel.win_size_x - (30 - window_border_width) + 4,
        win_sel.win_pos_y + window_border_width + 3,
        2, 2, 'X', 0x0, 0, false
    );


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

// Create a new window. De-select other windows and focus on the newly created one, while adding it.
uint8_t window_create(uint16_t win_pos_x, uint16_t win_pos_y, uint16_t win_size_x, uint16_t win_size_y, bool win_draggable, bool win_sizable, char* title, uint8_t context) {
    // If too many windows have been created
    uint8_t temp_id = 0;
    if (num_windows >= 31) {
        return 0xff;
    }

    // De-select all other windows in preperation
    for (int id=0;id<31;id++) {
        if (window_list[id].win_selected && window_list[id].win_active) {
            window_list[id].win_selected = false;
            //window_erase(id);
            window_render(id);
        }
        if (!window_list[id].win_active) {
            temp_id = id;
            break;
        }
    }

    // Fill in our window
    window_root new_window = {win_pos_x, win_pos_y, win_size_x, win_size_y, 5, temp_id, true, win_draggable, win_sizable, true, false, title, context};
    new_window.win_active = true;
    window_list[temp_id] = new_window;
    // Now all windows are not selected, draw ours.
    window_render(temp_id);
    num_windows++;
 
    return temp_id;
}

void window_destroy(uint8_t win_id) {
    window_list[win_id].win_active = false;
    window_erase(win_id);
    // re-draw lower windows
    for (int id=0;id<31;id++) {
        window_root cwin = window_list[id];
        bool win_collision = rect_2_collide(
            window_list[id].win_pos_x,
            window_list[id].win_pos_y,
            window_list[id].win_pos_x + window_list[id].win_size_x,
            window_list[id].win_pos_y + window_list[id].win_size_y,
            cwin.win_pos_x,
            cwin.win_pos_y,
            cwin.win_pos_x + cwin.win_size_x,
            cwin.win_pos_y + cwin.win_size_y   
        );
        if (id != win_id && win_collision) {
            window_render(id);
        }
    }

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

    for (int id=0;id<31;id++) {
        window_root cwin = window_list[id];
        if (cwin.win_active) {
            bool win_collision = rect_2_collide(
                window_list[id].win_pos_x,
                window_list[id].win_pos_y,
                window_list[id].win_pos_x + window_list[id].win_size_x,
                window_list[id].win_pos_y + window_list[id].win_size_y,
                cwin.win_pos_x,
                cwin.win_pos_y,
                cwin.win_pos_x + cwin.win_size_x,
                cwin.win_pos_y + cwin.win_size_y
                
            );
            if (id != win_id && win_collision) {
                window_render(id);
            }
        }
    }

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
uint8_t win_top_offset;

void window_left() {
    if (!press_exists) { // initiating a click, find affected windows
        press_exists = true;
        for (int win_id=0;win_id<31;win_id++) {
            // Now the selected window is found, check for a topbar intersection
            // Be sure to avoid context buttons on the top, so offset the topbar size. Check one by one.
            // All buttons are 30x30 px
            win_top_offset = 0;
            for (uint8_t b=0;b<8;b++) {
                if (window_list[win_id].win_menu & (1 << b)) {
                    // For each button we find, as denoted by a 1, increase the fofset to 30 pixels.
                    win_top_offset = win_top_offset + 30;
                }
            }
            // If collided with the topbar, interact with it
            if (rect_collide(
                window_list[win_id].win_pos_x,
                window_list[win_id].win_pos_y,
                window_list[win_id].win_pos_x + window_list[win_id].win_size_x - win_top_offset,
                window_list[win_id].win_pos_y + window_topbar_height + window_border_width,
                mouse_position.pos_x,
                mouse_position.pos_y
            )) {
                // If so, check if the window is already selected. If not, select it and wait for the next turn.
                if (window_list[win_id].win_selected) {
                    window_mover_selection = win_id;
                    mouse_start_x = mouse_position.pos_x;
                    mouse_start_y = mouse_position.pos_y;

                    // Set a preview
                    current_window_previewer.box_position_x = window_list[win_id].win_pos_x;
                    current_window_previewer.box_position_y = window_list[win_id].win_pos_y;
                    current_window_previewer.box_size_x = window_list[win_id].win_size_x;
                    current_window_previewer.box_size_y = window_list[win_id].win_size_y;

                    current_window_previewer.box_border_width = 3;
                    current_window_previewer.box_color = 0x0;
                    current_window_previewer.box_hasRendered = false; // set to false, as we started rendering

                    preview_start();
                } else {
                    uint8_t selected = 0xff;
                    for (int id=0;id<31;id++) {
                        if (window_list[id].win_selected && id != win_id) {
                            window_list[id].win_selected = false;
                            window_erase(id);
                            window_render(id);
                        }
                        if (!window_list[id].win_selected && id == win_id) {
                            selected = id;
                        }
                    }
                    if (selected != 0xff) {
                        window_list[selected].win_selected = true;
                        window_erase(selected);
                        window_render(selected);
                    }
                }
                break;
            }

            //Now check for context menu (if collided with any of the buttons)
            if (win_top_offset) {
                if (rect_collide(
                    window_list[win_id].win_pos_x + window_list[win_id].win_size_x - win_top_offset,
                    window_list[win_id].win_pos_y,
                    window_list[win_id].win_pos_x + window_list[win_id].win_size_x,
                    window_list[win_id].win_pos_y + window_topbar_height + window_border_width,
                    mouse_position.pos_x,
                    mouse_position.pos_y
                )) {
                    // Destroy the window, and completely erase it
                    window_destroy(win_id);
                    break;
                }
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

void window_preview_mover() {
    preview_update(mouse_position.pos_x, mouse_position.pos_y);
}

// TODO
// When moving a window, draw back any that were underneath it, before rendering it again.
// Mover preview