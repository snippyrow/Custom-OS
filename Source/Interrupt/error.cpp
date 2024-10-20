void WIN_FillRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t color);
void WIN_DrawChar(uint16_t x, uint16_t y, uint16_t xscale, uint16_t yscale, uint8_t character, uint8_t color);

void error_handler(int interrupt_num, int interrupt_addr) {
    uint16_t error_padding_width = 50;
    uint16_t error_padding_height = 300;
    uint16_t error_width = (WIN_WIDTH - error_padding_width);
    uint16_t error_height = (WIN_HEIGHT - error_padding_height);

    uint16_t error_main_translate = 10;

    uint16_t error_topbar_height = (error_height - error_padding_height) * 0.75;

    uint16_t error_topbar_factor = (error_height - error_padding_height) / 4;

    WIN_FillRect(error_padding_width,error_padding_height,error_width,error_height,0x70);

    WIN_FillRect(
        error_padding_width-error_main_translate,
        error_padding_height-error_main_translate,
        error_width-error_main_translate,
        error_height-error_main_translate,
        0x40
    );

    WIN_FillRect(
        error_padding_width-error_main_translate,
        error_padding_height-error_main_translate,
        error_width-error_main_translate,
        error_height-error_main_translate-error_topbar_height,
        0xc
    );

    // render strings
    WIN_DrawString(
        error_padding_width-error_main_translate,
        error_padding_height-error_main_translate,
        4,
        4,
        error_list[interrupt_num],
        0
    );

    char* error_displaycode = hex64_str(interrupt_num);

    char* error_displayaddress = hex64_str(interrupt_addr);

    char* error_code_finalstr = strcat("Interrupt Vector: 0x", error_displaycode);
    char* error_addr_finalstr = strcat("Interrupt Address: 0x", error_displayaddress);

    WIN_DrawString(
        error_padding_width-error_main_translate,
        error_padding_height-error_main_translate + error_topbar_factor,
        2,
        2,
        error_code_finalstr,
        0
    );

    WIN_DrawString(
        error_padding_width-error_main_translate,
        error_padding_height-error_main_translate + error_topbar_factor + (10*2),
        2,
        2,
        error_addr_finalstr,
        0
    );

    WIN_DrawString(
        error_padding_width-error_main_translate,
        error_padding_height-error_main_translate + error_topbar_factor + (10*2)*4,
        2,
        2,
        "I never said it would be easy :)",
        0
    );


    free(*error_code_finalstr,128);
    free(*error_displaycode,128);

    free(*error_displayaddress,128);
    free(*error_addr_finalstr,128);
    
    WIN_SwitchFrame();
    cli();
    while(1);
    return;
}

// 0x42