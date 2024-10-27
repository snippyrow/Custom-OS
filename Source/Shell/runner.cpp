#include "shell.h"
#include "../Drivers/ATA.cpp"

// Pointers for ATA functions..
void ATA_Read(uint32_t lba, uint8_t sectors, uint8_t *buffer);
void free(uint64_t ptr, uint32_t size_t);
uint64_t malloc(uint32_t size_t);

void shell_div() {
    int num0 = str_int64(shell_argtable[1]);
    int num1 = str_int64(shell_argtable[2]);
    char* result = int64_str(num0/num1);
    char* print = strcat("\n--> ",result);
    shell_tty_print(print);

    free(*result, strlen(result) + 2);
    free(*print, strlen(print) + 2);
}

void shell_help() {
    for (int cmd = 0;cmd<sizeof(commands)/sizeof(commands[0]);cmd++) {
        shell_tty_print("\n");
        uint8_t cmdlen = strlen(commands[cmd][0]);
        shell_tty_print(commands[cmd][0]);
        for (int p=0;p<(8 - cmdlen);p++) {
            shell_tty_print(" ");
        }
        shell_tty_print(commands[cmd][1]);
    }
    shell_memory_render();
}

void ata_test_device() {
    uint8_t* ata_read_table = (uint8_t*)malloc(512);
    uint32_t ata_read_lba = str_int64(shell_argtable[1]);
    ATA_Read(ata_read_lba,1,ata_read_table);
    uint8_t tnum;

    for (int i=0;i<512;i++) {

        if ((i) % 16 == 0) {
            shell_tty_print("\n");
            char* ln = hex64_str((i/16) * 16 + (ata_read_lba * 512));
            shell_tty_print(ln);
            shell_tty_print(": ");
            free(*ln,32);
        }

        tnum = ata_read_table[i];
        char* str = hex64_str(tnum);
    
        if (tnum <= 0xf) {
            str[1] = str[0];  // Shift the first character to the second position
            str[0] = '0';     // Add '0' at the first position
            str[2] = '\0';    // Null-terminate the string
        }

        shell_tty_print(str);
        shell_tty_print(" ");


        free(*str,32);
    }
    shell_memory_render();
    
    free(*ata_read_table,512);
}

void shell_win_test() {
    // Destroy TTY system
    WIN_RenderClear(0xaf);
    WIN_SwitchFrame_A();
    shell_tty_enabled = false;
    mouse_left_hook = window_left;
    windows_init();
    uint8_t w_id = window_create(50,50,700,400,true,true);
    uint8_t w_id2 = window_create(600,300,100,75,true,true);
    WIN_DrawMouse();
}