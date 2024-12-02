// TTY Resolution: 100x75 (scale 1, 8x8)
// TTY Resolution: 51x38 (scale 2, 8x8)

#include "shell.h"
#include "runner.cpp"

uint16_t shell_line = 0;
uint16_t shell_column = 0;

uint16_t shell_tablen = 4;
bool shell_kbd_enabled = true;
uint8_t tg;

char root_prompt[] = "/ ";
char shell_kbd_buffer[256];
bool upper = false;
uint8_t numthemes = sizeof(devshell_themes)/sizeof(devshell_themes[0]);

char SHELL_VideoMemory[SHELL_TTY_WIDTH][SHELL_TTY_HEIGHT][2];

void shell_memory_render() {
    if (!shell_tty_enabled) {
        return;
    }
    for (uint8_t x = 0;x<SHELL_TTY_WIDTH;x++) {
        for (uint8_t y = 0;y<SHELL_TTY_HEIGHT;y++) {
            char character = SHELL_VideoMemory[x][y][0];
            if (character && !SHELL_VideoMemory[x][y][1]) {
                uint16_t win_render_x = (x * 8 * SHELL_TTY_FSIZE) + SHELL_TTY_PADDING;
                uint16_t win_render_y = (y * 10 * SHELL_TTY_FSIZE) + SHELL_TTY_PADDING;
                WIN_DrawChar(win_render_x, win_render_y, SHELL_TTY_FSIZE, SHELL_TTY_FSIZE, character, shell_text_color, shell_bg, true);
                SHELL_VideoMemory[x][y][1] = true;
                WIN_SwitchFrame(win_render_x, win_render_y, win_render_x + 16, win_render_y + 20);
            }
        }
    }
    //WIN_DrawMouse();
}

// Scrolls the video memory up once, and re-renders. Irreversable!
void shell_memory_scroll() {
    for (int x=0;x<SHELL_TTY_WIDTH;x++) {
        for (int y=1;y<SHELL_TTY_HEIGHT;y++) {
            SHELL_VideoMemory[x][y-1][0] = SHELL_VideoMemory[x][y][0];
            SHELL_VideoMemory[x][y-1][1] = false;
        }
    }
    for (int x = 0; x < SHELL_TTY_WIDTH; x++) {
        SHELL_VideoMemory[x][SHELL_TTY_HEIGHT-1][0] = ' ';
        SHELL_VideoMemory[x][SHELL_TTY_HEIGHT-1][1] = false;
    }
}

void shell_tty_print(char* string) {
    if (!shell_tty_enabled) {
        return;
    }
    uint32_t i = 0;
    while (string[i]) {
        switch(string[i]) {
            case '\n':
                shell_line++;
                shell_column = 0;
                break;
            case '\r':
                shell_column = 0;
                break;
            case '\t':
                if (shell_column + 4 >= SHELL_TTY_WIDTH){
                    shell_line++;
                    shell_column = 0;
                }
                tg = shell_tablen;
                while (tg) {
                    SHELL_VideoMemory[shell_column][shell_line][0] = ' ';
                    SHELL_VideoMemory[shell_column][shell_line][1] = false;
                    tg--;
                    shell_column++;
                }
                break;
            default:
                if (shell_column >= SHELL_TTY_WIDTH){
                    shell_line++;
                    shell_column = 0;
                }

                SHELL_VideoMemory[shell_column][shell_line][0] = string[i];
                SHELL_VideoMemory[shell_column][shell_line][1] = false;
                shell_column++;
                break;
        }

        i++;

        if (shell_line > SHELL_TTY_HEIGHT-1) {
            shell_memory_scroll();
            shell_line = SHELL_TTY_HEIGHT-1;
        }

    }
}

void shell_tty_clear() {
    if (!shell_tty_enabled) {
        return;
    }
    for (uint8_t x = 0;x<SHELL_TTY_WIDTH;x++) {
        for (uint8_t y = 0;y<SHELL_TTY_HEIGHT;y++) {
            SHELL_VideoMemory[x][y][0] = ' ';
            SHELL_VideoMemory[x][y][1] = false;
        }
    }
    shell_line = 0;
    shell_column = 0;
    shell_memory_render();
}

void shell_tty_set(uint16_t x, uint16_t y, char character) {
    SHELL_VideoMemory[x][y][0] = character;
    SHELL_VideoMemory[x][y][1] = false;
}


// **COMMAND PARSER IS FLIMSY AND DOES NOT WORK!**
void shell_enter_handler() {
    if (!shell_tty_enabled) {
        return;
    }
    uint16_t numsplit = strsplit(shell_kbd_buffer, ' ', shell_argtable, SHELL_MAX_ARGS);
    char* uppercmd = strup(shell_argtable[0]);

    bool found = false;
    for (int cmd = 0;cmd<sizeof(commands)/sizeof(commands[0]);cmd++) {
        if (strcmp(commands[cmd][0], uppercmd)) {
            found = true;
            shell_handlers[cmd]();
        }
    }

    if (!found) {
        shell_tty_print(shell_notfound[0]);
        shell_tty_print(shell_argtable[0]);
        shell_tty_print(shell_notfound[1]);
    }

    free(*uppercmd, strlen(uppercmd)+2);
    shell_tty_print("\n/");
    shell_tty_print(shell_dir_name);
    shell_tty_print("/ ");
    shell_memory_render();



    // clear the keyboard buffer and argtables
    for (int i=0;i<256;i++) {
        shell_kbd_buffer[i] = '\0';
    }
    for (int arg=0;arg<SHELL_MAX_ARGS;arg++) {
        for (int c=0;c<64;c++) {
            shell_argtable[arg][c] = '\0';
        }
    }
}

// todo: add backspace, propper tab, enter handler, caps lock, basic argument and command reader

void shell_kbd_hook() {
    uint8_t scancode = inb(0x60);
    if (!shell_tty_enabled) {
        return;
    }
    if (!shell_kbd_enabled) {
        return;
    }
    if (scancode == 0x2a || scancode == 0x36) {
        upper = true;
    } else if (scancode == 0xaa || scancode == 0xb6) {
        upper = false;
    } else if (scancode == 0xe) {
        uint16_t bufferlen = strlen(shell_kbd_buffer);
        if (bufferlen > 0) { // so that you cannot exit the text buffer
            if (shell_column - 1 < 0) {
                shell_line--;
                shell_column = SHELL_TTY_WIDTH - 1;
            } else {
                shell_column--;
            }
            shell_tty_set(shell_column, shell_line, ' ');
            shell_kbd_buffer[bufferlen - 1] = '\0';
            shell_memory_render();
            // move the cursor back one spot and white out the character
        }
    } else if (scancode == 0x1c) {
        shell_enter_handler();
    } else if (scancode < 58) {
        if (strlen(shell_kbd_buffer) < 256) {
            char letter;
            if (upper) {
                letter = keymap_shift[(int) scancode];
            } else {
                letter = keymap[(int) scancode];
            }
            char str[2] = {letter, '\0'};
            strcat_m(shell_kbd_buffer,str);
            shell_tty_print(str);
            shell_memory_render();
        }
    }
}

// Re-draw entire screen
void shell_newtheme() {
    WIN_RenderClear(shell_bg);
    for (uint8_t x = 0;x<SHELL_TTY_WIDTH;x++) {
        for (uint8_t y = 0;y<SHELL_TTY_HEIGHT;y++) {
            SHELL_VideoMemory[x][y][1] = false;
        }
    }
    shell_memory_render();
    WIN_SwitchFrame_A();
}

void shell_theme_set() {
    if (strcmp(shell_argtable[1],"-l")) {
        // list avalible themes
        for (uint8_t t = 0;t<numthemes;t++) {
            char* tnumln = int64_str(t + 1);
            char* bgln = hex64_str(devshell_themes[t][0]);
            char* txln = hex64_str(devshell_themes[t][1]);
            shell_tty_print("\nTheme #");
            shell_tty_print(tnumln);
            shell_tty_print(":\n\t Background: [0x");
            shell_tty_print(bgln);
            shell_tty_print("]\n\t Textcolor: [0x");
            shell_tty_print(txln);
            shell_tty_print("]");

            free(*tnumln, 32);
            free(*bgln, 32);
            free(*txln, 32);
        }
    } else if (strcmp(shell_argtable[1],"-m")) {
        uint8_t themeid = str_int64(shell_argtable[2]);
        if (themeid <= 0 || themeid > numthemes) {
            shell_tty_print("\nInvalid theme selected.");
        } else {
            uint8_t bgcolor = str_int64(shell_argtable[3]);
            uint8_t textcolor = str_int64(shell_argtable[4]);
            devshell_themes[themeid-1][0] = bgcolor;
            devshell_themes[themeid-1][1] = textcolor;
            shell_tty_print("\nCreated theme #");
            shell_tty_print(int64_str(themeid));
        }
        
    } else {
        uint8_t selected = str_int64(shell_argtable[1]);
        if (selected >= (numthemes + 1) || selected <= 0) {
            shell_tty_print("\nInvalid theme selected.");
        } else {
            selected--;
            shell_bg = devshell_themes[selected][0];
            shell_text_color = devshell_themes[selected][1];
            shell_newtheme();
        }
    }
}

void shell_mouse_en() {
    if (shell_argtable[1][0] == '1') {
        mouse_enabled = true;
        shell_tty_print("\nEnabled the mouse");
    } else {
        mouse_enabled = false;
        shell_tty_print("\nDisabled the mouse");
    }
    shell_memory_render();
}