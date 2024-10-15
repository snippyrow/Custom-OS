#include "vesa.h"

void init_buffer() {
    WIN_FBUFF = (unsigned char *)vbe_info -> framebuffer;
    WIN_WIDTH = vbe_info -> width;
    WIN_HEIGHT = vbe_info -> height;
    return;
}

void WIN_SwitchFrame() {
    for (int i=0;i<(WIN_WIDTH*WIN_HEIGHT);i++) {
        WIN_FBUFF[i] = WORK_BUFF[i];
    }
    return;
}

void WIN_RenderClear(unsigned char color) {
    for (int i=0;i<(WIN_WIDTH*WIN_HEIGHT);i++) {
        WORK_BUFF[i] = color;
    }
    return;
}

void WIN_FillRect(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint8_t color) {
    for (int x=x0;x<x1;x++) {
        for (int y=y0;y<y1;y++) {
            WORK_BUFF[(y*WIN_WIDTH)+x] = color;
        }
    }
    return;
}

void WIN_PutPixel(uint16_t x, uint16_t y, uint8_t color) {
    WORK_BUFF[(y*WIN_WIDTH)+x] = color;
    return;
}

void WIN_DrawChar(uint16_t x, uint16_t y, uint16_t xscale, uint16_t yscale, uint8_t character, uint8_t color) {
    // Pull from buffer 0xa000, bitmap
    int it = ((character/16)*8)+(character/16);
    int cX = (character%16)+(character%16)*8;
    int cY = 144*(it);

    for (int y1=0;y1<8*yscale;y1++) {
        for (int x1=0;x1<8*xscale;x1++) {
            
            int ptr = (((y1/yscale)*144)+cY)+(x1/xscale)+cX;
            uint8_t var = FONT_BUFF[ptr];

            if ((var)) {
                WORK_BUFF[((y1+y)*WIN_WIDTH)+x1+x] = color;
            }
        }
    }
    return;
}

void WIN_DrawString(uint16_t x, uint16_t y, uint16_t xscale, uint16_t yscale, char* str, uint8_t color) {
    int i = 0;
    while (str[i]) {
        WIN_DrawChar(x + (i * 8 * xscale), y, xscale, yscale, str[i], color);
        i++;
    }
    return;
}