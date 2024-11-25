// TTY Resolution: 51x38 (scale 2, 8x8)

#ifndef MYHEADER_H
#define MYHEADER_H

#define SHELL_TTY_WIDTH 63
#define SHELL_TTY_HEIGHT 38

#define SHELL_TTY_FSIZE 2

// Side padding
#define SHELL_TTY_PADDING 4
#define SHELL_MAX_ARGS 6

char shell_argtable[SHELL_MAX_ARGS][64];

char shell_notfound[2][13] = {"\nCommand '","' not found."};

const char keymap[] = {
    '?', '?', '1', '2', '3', '4', '5',
    '6', '7', '8', '9', '0', '-', '=',
    '?', '?', 'q', 'w', 'e', 'r', 't',
    'y', 'u', 'i', 'o', 'p', '[', ']',
    '?', '?', 'a', 's', 'd', 'f', 'g',
    'h', 'j', 'k', 'l', ';', '\\', '`',
    '?', '\\', 'z', 'x', 'c', 'v', 'b',
    'n', 'm', ',', '.', '/', '?', '?',
    '?', ' '
};

const char keymap_shift[] = {
    '?', '?', '!', '@', '#', '$', '%',
    '^', '&', '*', '(', ')', '_', '+',
    '?', '?', 'Q', 'W', 'E', 'R', 'T',
    'Y', 'U', 'I', 'O', 'P', '[', ']',
    '?', '?', 'A', 'S', 'D', 'F', 'G',
    'H', 'J', 'K', 'L', ':', '\\', '~',
    '?', '\\', 'Z', 'X', 'C', 'V', 'B',
    'N', 'M', '<', '>', '?', '?', '?',
    '?', ' '
};

void shell_tty_print(char* string);
void shell_memory_render();
void shell_tty_clear();
void shell_win_test();
uint8_t window_create(uint16_t win_pos_x, uint16_t win_pos_y, uint16_t win_size_x, uint16_t win_size_y, bool win_draggable, bool win_sizable, char* title, uint8_t context);
void window_left();
void windows_init();

// Formal definitions of commands
void shell_div();
void shell_help();
void ata_test_device();
void shell_mouse_en();
void shell_ata_enum();
void shell_theme_set();
void fat_format();
void shell_fat();
void fat_list_files();

void window_preview_mover();

uint32_t pci_config_address(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
uint32_t read_pci_config(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);

bool shell_tty_enabled = true;

char commands[][2][128] = {
    {"ATA","[-e uint] List workable ATA devices"},
    {"BOOT","Bring up boot manager"},
    {"CLEAR","Clear TTY"},
    {"DIV","[a int, b int] Divide two numbers"},
    {"FAT","[-f, -ls]Testing for CFAT24"},
    {"HELP","Get help on commands"},
    {"MOUSE","[0/1 bool] Enable/disable mouse"},
    {"THEME","[[-l, -m], #num, BG, TX] List, modify or set theme"}

};

fn_ptr shell_handlers[] = {
    shell_ata_enum, shell_win_test, shell_tty_clear, shell_div, shell_fat, shell_help, shell_mouse_en, shell_theme_set
};

uint8_t shell_bg = 0x0;
uint8_t shell_text_color = 0x2a;

// Background, textcolor
uint8_t devshell_themes[5][2] = {
    {0xB0, 0x0F}, // classic blue
    {0x00, 0x2A}, // new orange
    {0x24, 0x0A} // ass.
};

#endif  // End of include guard

// TTY Resolution: 100x75 (scale 1, 8x8)