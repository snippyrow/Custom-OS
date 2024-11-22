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
    mouse_move_hook = window_preview_mover;
    //windows_init();
    uint8_t w_id = window_create(50,50,700,400,true,true,"Large Window",0b000001);
    uint8_t w_id2 = window_create(100,200,200,200,true,true,"Medium Test",0b000001);
    uint8_t w_id3 = window_create(600,300,100,75,true,true,"Small",0b00000001);
    mouse_enabled = true;
    WIN_DrawMouse();
}

void shell_ata_enum() {
    uint8_t count = 0;
    if (strcmp(shell_argtable[1],"-m")) {
        shell_tty_print("\nEnumerating ");
        shell_tty_print(shell_argtable[2]);
        shell_tty_print(" maximum ATA devices..\n");
    } else {
        return;
    }
    // Enumerate over ALL PCI devices
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t device = 0; device < 32; device++) {
            for (uint8_t function = 0; function < 8; function++) {
                uint32_t vendor_device = read_pci_config(bus, device, function, 0x00);
                if ((vendor_device & 0xFFFF) != 0xFFFF) { // Check if device exists
                    uint32_t class_code = read_pci_config(bus, device, function, 0x08);
                    uint8_t base_class = (class_code >> 24) & 0xFF;
                    uint8_t subclass = (class_code >> 16) & 0xFF;

                    if (base_class == 0x01 && (subclass == 0x01 || subclass == 0x06)) { // IDE Controller
                        // If found, print stuff

                        count++;
                        char* busln = uhex32_str(bus);
                        char* devln = uhex32_str(device);
                        char* fnln = uhex32_str(function);
                        
                        if (subclass == 0x1) {
                            shell_tty_print("IDE Controller found:\n");
                        } else if (subclass == 0x6) {
                            shell_tty_print("SATA Controller found:\n");
                        }
                        shell_tty_print("\tBus: [0x");
                        shell_tty_print(busln);
                        shell_tty_print("]\n\tDevice: [0x");
                        shell_tty_print(devln);
                        shell_tty_print("]\n\tFunction: [0x");
                        shell_tty_print(fnln);
                        shell_tty_print("]\n");

                        free(*busln, 32);
                        free(*devln, 32);
                        free(*fnln, 32);

                        // Read BARs
                        for (uint8_t bar_index = 0; bar_index < 6; bar_index++) {
                            uint32_t bar = read_pci_config(bus, device, function, 0x10 + bar_index * 4);
                            if (bar & 0x01) {
                                // I/O BAR
                                shell_tty_print("\tBAR");
                                char* io_bar_index = int64_str(bar_index);
                                char* io_bar = uhex32_str(bar & 0xFFFFFFFC);
                                shell_tty_print(io_bar_index);
                                shell_tty_print(" (I/O): [0x");
                                shell_tty_print(io_bar);
                                shell_tty_print("]\n");

                                free(*io_bar_index, 32);
                                free(*io_bar, 32);
                            } else {
                                // Memory BAR
                                shell_tty_print("\tBAR");
                                char* io_bar_index = int64_str(bar_index);
                                char* io_bar = uhex32_str(bar & 0xFFFFFFFC);
                                shell_tty_print(io_bar_index);
                                shell_tty_print(" (Memory): [0x");
                                shell_tty_print(io_bar);
                                shell_tty_print("]\n");

                                free(*io_bar_index, 32);
                                free(*io_bar, 32);
                            }
                        }

                        // Programming interface
                        uint8_t prog_int = (read_pci_config(bus, device, function, 0x08) >> 8) & 0xFF;
                        char* prog_int_ln = hex64_str(prog_int);
                        shell_tty_print("\tProgramming Interface: [0x");
                        shell_tty_print(prog_int_ln);
                        shell_tty_print("]\n");

                    }
                }
            }
        }
    }
    if (!count) {
        shell_tty_print("No avalible ATA devices found.\n");
    }
}