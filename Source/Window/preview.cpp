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

// TODO For tomorrow:
// Develop mask updating, removing a preview, and moving a preview via updating.

void preview_render() {
    if (!current_window_previewer.box_hasRendered) {
        // We need to create a mask for stuff underneath. Assume we already have a fixed size.
        size_width = (current_window_previewer.box_size_x - current_window_previewer.box_border_width) * current_window_previewer.box_border_width;
        size_height = (current_window_previewer.box_size_y - current_window_previewer.box_border_width) * current_window_previewer.box_border_width;

        mask_ptr_top = (uint8_t*)malloc(size_width);
        mask_ptr_left = (uint8_t*)malloc(size_height);
        mask_ptr_bottom = (uint8_t*)malloc(size_width);
        mask_ptr_right = (uint8_t*)malloc(size_height);

        for (int i=0;i<size_width;i++) {

        }
    }

    iee = 0;
    // First draw top, left, bottom, right
    current_window_previewer.box_hasRendered = true;
    WIN_FillRect(
        current_window_previewer.box_position_x,
        current_window_previewer.box_position_y,
        current_window_previewer.box_position_x + current_window_previewer.box_size_x,
        current_window_previewer.box_position_y + current_window_previewer.box_border_width,
        //current_window_previewer.box_color
        iee
    );
    iee++;
    WIN_FillRect(
        current_window_previewer.box_position_x,
        current_window_previewer.box_position_y + current_window_previewer.box_border_width,
        current_window_previewer.box_position_x + current_window_previewer.box_border_width,
        current_window_previewer.box_position_y + current_window_previewer.box_size_y,
        //current_window_previewer.box_color
        iee
    );
    iee++;
    WIN_FillRect(
        current_window_previewer.box_position_x + current_window_previewer.box_border_width,
        current_window_previewer.box_position_y + current_window_previewer.box_size_y - current_window_previewer.box_border_width,
        current_window_previewer.box_position_x + current_window_previewer.box_size_x,
        current_window_previewer.box_position_y + current_window_previewer.box_size_y,
        //current_window_previewer.box_color
        iee
    );
    iee++;
    WIN_FillRect(
        current_window_previewer.box_position_x + current_window_previewer.box_size_x - current_window_previewer.box_border_width,
        current_window_previewer.box_position_y,
        current_window_previewer.box_position_x + current_window_previewer.box_size_x,
        current_window_previewer.box_position_y + current_window_previewer.box_size_y - current_window_previewer.box_border_width,
        //current_window_previewer.box_color
        iee
    );
    WIN_SwitchFrame_A();
}