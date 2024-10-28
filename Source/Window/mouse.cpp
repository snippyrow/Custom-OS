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
    if (!mouse_enabled) {
        return;
    }
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
    if (!mouse_enabled) {
        return;
    }
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