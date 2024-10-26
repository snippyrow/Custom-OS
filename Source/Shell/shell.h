// TTY Resolution: 51x38 (scale 2, 8x8)

#ifndef MYHEADER_H
#define MYHEADER_H

#define SHELL_TTY_WIDTH 63
#define SHELL_TTY_HEIGHT 38

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

void shell_div();
void shell_help();
void ata_test_device();

char commands[][2][128] = {
    {"BOOT","Bring up boot manager"},
    {"CLEAR","Clear TTY"},
    {"DIV","(A) (B) Divide two numbers"},
    {"HELP","Get help on commands"},
    {"HEXEDIT","(LBA) Reads the primary ATA device"}
};

typedef void (*shell_ptr)();

shell_ptr shell_handlers[] = {
    (shell_ptr)1,shell_tty_clear, shell_div, shell_help, ata_test_device
};

#endif  // End of include guard

// TTY Resolution: 100x75 (scale 1, 8x8)
