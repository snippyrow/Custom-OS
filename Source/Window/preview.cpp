// Primary functions for a window mover preview.
// A window preview is a hollow box around the window, usually black. This is to reduce drawing cycles, and to show you user what they are doing.

// Rectangular masks
uint8_t* mask_ptr_top;
uint8_t* mask_ptr_left;
uint8_t* mask_ptr_bottom;
uint8_t* mask_ptr_right;

uint16_t size_width;
uint16_t size_height;

uint8_t iee;

uint16_t p_startx;
uint16_t p_starty;

// TODO For tomorrow:
// Develop mask updating, removing a preview, and moving a preview via updating.

void preview_render_masks() {
    // Top
    for (int x=0;x<(current_window_previewer.box_size_x - current_window_previewer.box_border_width);x++) {
        for (int y=0;y<current_window_previewer.box_border_width;y++) {
            WORK_BUFF[((y + current_window_previewer.box_position_y) * WIN_WIDTH) + (x + current_window_previewer.box_position_x)] = mask_ptr_top[y * (current_window_previewer.box_size_x - current_window_previewer.box_border_width) + x];
        }
    }

    //Left
    for (int y = 0; y < (current_window_previewer.box_size_y); y++) {
        for (int x = 0; x < current_window_previewer.box_border_width; x++) {
            WORK_BUFF[((y + current_window_previewer.box_position_y + current_window_previewer.box_border_width) * WIN_WIDTH) + (x + current_window_previewer.box_position_x)] = mask_ptr_left[y * current_window_previewer.box_border_width + x];
        }
    }

    // Bottom
    for (int x=0; x<(current_window_previewer.box_size_x - current_window_previewer.box_border_width); x++) {
        for (int y=0; y<current_window_previewer.box_border_width; y++) {
            WORK_BUFF[((y + current_window_previewer.box_position_y + current_window_previewer.box_size_y - current_window_previewer.box_border_width) * WIN_WIDTH) + (x + current_window_previewer.box_position_x + current_window_previewer.box_border_width)] = mask_ptr_bottom[y * (current_window_previewer.box_size_x - current_window_previewer.box_border_width) + x];
        }
    }

    // Right
    for (int y = 0; y < (current_window_previewer.box_size_y); y++) {
        for (int x = 0; x < current_window_previewer.box_border_width; x++) {
            WORK_BUFF[((y + current_window_previewer.box_position_y) * WIN_WIDTH) + (x + current_window_previewer.box_position_x + current_window_previewer.box_size_x - current_window_previewer.box_border_width)] = mask_ptr_right[y * current_window_previewer.box_border_width + x];
        }
    }
}

void preview_update_masks() {
    // Top
    for (int x=0;x<(current_window_previewer.box_size_x - current_window_previewer.box_border_width);x++) {
        for (int y=0;y<current_window_previewer.box_border_width;y++) {
            mask_ptr_top[y * (current_window_previewer.box_size_x - current_window_previewer.box_border_width) + x] = WORK_BUFF[((y + current_window_previewer.box_position_y) * WIN_WIDTH) + (x + current_window_previewer.box_position_x)];
        }
    }

    //Left
    for (int y = 0; y < (current_window_previewer.box_size_y * current_window_previewer.box_border_width); y++) {
        for (int x = 0; x < current_window_previewer.box_border_width; x++) {
            mask_ptr_left[y * current_window_previewer.box_border_width + x] = WORK_BUFF[((y + current_window_previewer.box_position_y + current_window_previewer.box_border_width) * WIN_WIDTH) + (x + current_window_previewer.box_position_x)];
        }
    }

    // Bottom
    for (int x=0;x<(current_window_previewer.box_size_x - current_window_previewer.box_border_width);x++) {
        for (int y=0;y<current_window_previewer.box_border_width;y++) {
            mask_ptr_bottom[y * (current_window_previewer.box_size_x - current_window_previewer.box_border_width) + x] = WORK_BUFF[((y + current_window_previewer.box_position_y + current_window_previewer.box_size_y - current_window_previewer.box_border_width) * WIN_WIDTH) + (x + current_window_previewer.box_position_x + current_window_previewer.box_border_width)];

        }
    }

    // Right
    for (int y = 0; y < (current_window_previewer.box_size_y); y++) {
        for (int x = 0; x < current_window_previewer.box_border_width; x++) {
            mask_ptr_right[y * current_window_previewer.box_border_width + x] = WORK_BUFF[((y + current_window_previewer.box_position_y) * WIN_WIDTH) + (x + current_window_previewer.box_position_x + current_window_previewer.box_size_x - current_window_previewer.box_border_width)];
        }
    }
}

void preview_start() {
    p_startx = mouse_position.pos_x;
    p_starty = mouse_position.pos_y;

    size_width = (current_window_previewer.box_size_x) * current_window_previewer.box_border_width;
    size_height = (current_window_previewer.box_size_y) * current_window_previewer.box_border_width;

    mask_ptr_top = (uint8_t*)malloc(size_width);
    mask_ptr_bottom = (uint8_t*)malloc(size_width);
    mask_ptr_left = (uint8_t*)malloc(size_height);
    mask_ptr_right = (uint8_t*)malloc(size_height);

    preview_update_masks();
}

void preview_render() {
    preview_render_masks();
    // Update mask, prevent chronic unhappiness
    WIN_SwitchFrame(
        current_window_previewer.box_position_x - window_bounding_width,
        current_window_previewer.box_position_y - window_bounding_width,
        current_window_previewer.box_position_x + current_window_previewer.box_size_x + window_bounding_width,
        current_window_previewer.box_position_y + current_window_previewer.box_size_y + window_bounding_width
    );

    // check bounds, as if it were a window
    window_root current_win = window_list[window_mover_selection];
    short new_pos_x = current_win.win_pos_x + (mouse_position.pos_x - p_startx);
    short new_pos_y = current_win.win_pos_y + (mouse_position.pos_y - p_starty);

    if (new_pos_x <= desktop_padding) {
        new_pos_x = desktop_padding;
    } else if (new_pos_x >= (WIN_WIDTH - desktop_padding - current_window_previewer.box_size_x - window_bounding_width)) {
        new_pos_x = (WIN_WIDTH - desktop_padding - current_window_previewer.box_size_x - window_bounding_width);  // Constrain to right boundary
    }

    if (new_pos_y <= desktop_padding) {
        new_pos_y = desktop_padding;
    } else if (new_pos_y >= (WIN_HEIGHT - desktop_padding - current_window_previewer.box_size_y - window_bounding_width)) {
        new_pos_y = (WIN_HEIGHT - desktop_padding - current_window_previewer.box_size_y - window_bounding_width);  // Constrain to bottom boundary
    }

    current_window_previewer.box_position_x = new_pos_x;
    current_window_previewer.box_position_y = new_pos_y;
    if (!current_window_previewer.box_hasRendered) {
        // We need to create a mask for stuff underneath. Assume we already have a fixed size.
        // Firstly render back the old buffer and then update it again. Do for top first
        preview_update_masks();
        WIN_SwitchFrame(
            current_window_previewer.box_position_x,
            current_window_previewer.box_position_y,
            current_window_previewer.box_position_x + (current_window_previewer.box_size_x - current_window_previewer.box_border_width),
            current_window_previewer.box_position_y + current_window_previewer.box_border_width
        );
    }



    // First draw top, left, bottom, right
    current_window_previewer.box_hasRendered = true;
    WIN_FillRect(
        current_window_previewer.box_position_x,
        current_window_previewer.box_position_y,
        current_window_previewer.box_position_x + current_window_previewer.box_size_x,
        current_window_previewer.box_position_y + current_window_previewer.box_border_width,
        current_window_previewer.box_color
    );
    WIN_FillRect(
        current_window_previewer.box_position_x,
        current_window_previewer.box_position_y + current_window_previewer.box_border_width,
        current_window_previewer.box_position_x + current_window_previewer.box_border_width,
        current_window_previewer.box_position_y + current_window_previewer.box_size_y,
        current_window_previewer.box_color
    );
    WIN_FillRect(
        current_window_previewer.box_position_x + current_window_previewer.box_border_width,
        current_window_previewer.box_position_y + current_window_previewer.box_size_y - current_window_previewer.box_border_width,
        current_window_previewer.box_position_x + current_window_previewer.box_size_x,
        current_window_previewer.box_position_y + current_window_previewer.box_size_y,
        current_window_previewer.box_color
    );
    WIN_FillRect(
        current_window_previewer.box_position_x + current_window_previewer.box_size_x - current_window_previewer.box_border_width,
        current_window_previewer.box_position_y,
        current_window_previewer.box_position_x + current_window_previewer.box_size_x,
        current_window_previewer.box_position_y + current_window_previewer.box_size_y - current_window_previewer.box_border_width,
        current_window_previewer.box_color
    );

    // render window preview lines
    WIN_SwitchFrame(
        current_window_previewer.box_position_x - window_bounding_width,
        current_window_previewer.box_position_y - window_bounding_width,
        current_window_previewer.box_position_x + current_window_previewer.box_size_x + window_bounding_width,
        current_window_previewer.box_position_y + current_window_previewer.box_size_y + window_bounding_width
    );

}

void preview_update(uint16_t mouse_x, uint16_t mouse_y) {
    if (window_mover_selection != 0xff) {
        current_window_previewer.box_hasRendered = false;
        preview_render();
    }
}

void preview_end() {
    preview_render_masks();
    free(*mask_ptr_top, size_width);
    free(*mask_ptr_bottom, size_width);
    free(*mask_ptr_left, size_height);
    free(*mask_ptr_right, size_height);
    return;
}