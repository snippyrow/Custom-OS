#include "shell.h"
#include "../Drivers/ATA.cpp"
#include "../FAT/cfat32.cpp"

#include "../Taskmgr/task.cpp"

// Pointers for ATA functions..
void ATA_Read(uint32_t lba, uint16_t num_sectors, uint8_t *buffer);
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

void ata_shell_print(uint32_t lba) {
    uint8_t* ata_read_table = (uint8_t*)malloc(512);
    uint32_t ata_read_lba = lba;
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

// Main ATA command. This is used to either set or enumerate serial ATA/ATA devices.

void shell_ata_enum() {
    uint8_t count = 0;
    uint8_t enumen = 0;
    if (strcmp(shell_argtable[1],"-e")) {
        shell_tty_print("\nEnumerating ");
        shell_tty_print(shell_argtable[2]);
        shell_tty_print(" maximum ATA devices..\n");
        enumen = 0xff; // makeshift enable
    } else if (strcmp(shell_argtable[1],"-w")) {
        uint8_t* w_table = (uint8_t*)malloc(512);
        for (uint8_t x = 0;x<0xff;x++) {
            w_table[x] = x;
        }
        ATA_Write(str_int64(shell_argtable[2]), 1, w_table);
        return;
    } else if (strcmp(shell_argtable[1],"-r")) {
        ata_shell_print(str_int64(shell_argtable[2]));
        return;
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

                    if (base_class == 0x01) { // IDE Controller
                        // If found, print stuff

                        count++;
                        char* busln = uhex32_str(bus);
                        char* devln = uhex32_str(device);
                        char* fnln = uhex32_str(function);
                        if (subclass >= 10) {
                            shell_tty_print("Unknown Controller found:\n");
                        } else {
                            shell_tty_print(ATA_SUBCLASSES[subclass]);
                            shell_tty_print(" Controller found:\n");
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


uint32_t task_ret_addr; // test, for programs
fat_object starter;

void shell_format() {
    if (strcmp(shell_argtable[1],"-f")) {
        fat_format(str_int64(shell_argtable[2]));
        shell_tty_print("\nFormatted partition to CFAT32.");
        
        starter = {"home", "", 2, 0, 0, 0, 0, 0}; // empty home directory
        uint32_t home_place = fat_mko(starter, 0);
        starter = {"", "", 0x80, 0, 0, 0, 0, 0}; // navigator to root
        fat_mko(starter, home_place);
        starter = {"README", {'t','x','t'}, 1, 0, 0, 0, 169, 0}; // empty home directory
        fat_mko(starter, 0);

        starter = {"example", {'r','u','n'}, 1, 0, 0, 0, 512, 0}; // empty home directory
        uint32_t begin = fat_mko(starter, 0);
        
        // Area to load example program
        uint8_t* ata_ptr = (uint8_t*)malloc(512);
        ATA_Read(150, 1, ata_ptr);
        fat_file_touch(begin, ata_ptr, 512);
        free(*ata_ptr, 512);

        process_create((uint32_t)memloop, false);
        process_create((uint32_t)memloop, false);

    } else if (strcmp(shell_argtable[1],"-s")) {
        uint32_t fid = fat_search();
        shell_tty_print("\nFirst found: 0x");
        shell_tty_print(uhex32_str(fid));
    } else if (strcmp(shell_argtable[1],"-next")) {
        uint32_t num = fat_search();
        char* str = uhex32_str(num);
        shell_tty_print("\n0x");
        shell_tty_print(str);
        free(*str, 32);
    } else if (strcmp(shell_argtable[1],"-ret")) {
        uint32_t num = fat_ret_next(str_int64(shell_argtable[2]));
        char* str = uhex32_str(num);
        shell_tty_print("\n0x");
        shell_tty_print(str);
        free(*str, 32);
    } else if (strcmp(shell_argtable[1],"-upd")) {
        fat_update(str_int64(shell_argtable[2]), str_int64(shell_argtable[3]));
    } else if (strcmp(shell_argtable[1],"-back")) {
        shell_operating_dir = 0;
        shell_dir_name[0] = 0;
    } else if (strcmp(shell_argtable[1],"-run")) {
        ATA_Read(150, 1, (uint8_t*)0x700000);
        uint32_t start_addr;
        // detect if it has a program header
        if (((uint16_t*)0x700000)[0] != 0x66BB) {
            shell_tty_print("\nNot a valid program!");
            return;
        } else {
            task_object state;
            start_addr = ((uint32_t*)0x700006)[0];
            uint32_t rot = process_create((uint32_t)memloop, false);
            uint32_t pid = process_create(start_addr, false);
            shell_tty_print("\n0x");
            shell_tty_print(uhex32_str(start_addr));
            shell_tty_print("\n-------\n0x");
            shell_tty_print(uhex32_str(rot));
            shell_tty_print("\n0x");
            shell_tty_print(uhex32_str(pid));
            
            // Find space for the stack (1024-byte)
            // task_object.esp = esp + 1024; // since stack decrements to go forward
            

            //task_save_context(&state);
        }
    } else if (strcmp(shell_argtable[1],"-l")) {
        
    }
}

void shell_cd() {
    if (strcmp(shell_argtable[1], "../")) {
        uint32_t cc = fat_dir_search(shell_operating_dir, "", 0x80, true);
        if (cc != -1) {
            shell_operating_dir = cc; // acts as a parent
            shell_tty_print(uhex32_str(cc));
            // copy directory name to the shell
            memcpy(&shell_dir_name, &fat_cd_object.o_name, 8);
            shell_dir_name[8] = '\0';
        }

    } else {
        uint32_t cc = fat_dir_search(shell_operating_dir, shell_argtable[1], 2, true);
        if (cc != -1) {
            shell_operating_dir = cc;
            // copy directory name to the shell
            for (uint8_t i=0;i<8;i++) {
                shell_dir_name[i] = shell_argtable[1][i];
            }
            shell_dir_name[8] = '\0';
        }
    }
}

void shell_ls() {
    fat_list_files(shell_operating_dir);
}

uint32_t shell_mko(uint8_t attrib) {
    fat_object ex_file = {
        "",
        "",
        attrib,
        0,
        0,
        0,
        36,
        0 // Directory structure + FAT usage
    };
    // copy name and extension
    uint8_t f = 0;
    while (shell_argtable[1][f] && f < 8) {
        ex_file.o_name[f] = shell_argtable[1][f];
        f++;
    }

    f = 0;
    while (shell_argtable[2][f] && f < 3) {
        ex_file.o_ext[f] = shell_argtable[2][f];
        f++;
    }
    int code = fat_mko(ex_file, shell_operating_dir);
    if (code == -1) {
        shell_tty_print("\nError creating object");
    }
    if (attrib == 2) {
        return code; // returns result placement of directory, for use of navigator
    }
}

void shell_mkfile() {
    shell_mko(1); // file
}

void shell_mkdir() {
    uint32_t ndir = shell_mko(2); // directory
    // Add the special navigator class
    fat_object ex_nav = {
        "",
        "",
        0x80,
        0,
        0,
        0,
        0,
        0
    };
    // Update the name and cluster of the *current* acting directory, acts as the parent directory
    memcpy(&ex_nav.o_name, &shell_dir_name, 8);
    ex_nav.cluster = shell_operating_dir;
    fat_mko(ex_nav, ndir);
}

void shell_touch() {
    uint32_t start = fat_dir_search(shell_operating_dir, shell_argtable[1], 1, true); // name only, not extension
    int code = fat_file_touch(start, (uint8_t*)shell_argtable[2], strlen(shell_argtable[2]));
    if (code == -1) {
        shell_tty_print("\nFailed to write");
    }
}

// first copy file data from builtin programs

// Continue here. Notes:
// remember to return to system stack before doing anything big like reading files
void shell_exec() {
    uint8_t si = 0;
    uint8_t splitlen;
    char exec_name[9];
    char exec_ext[5] = {0,0,0,0,0};
    while (shell_argtable[1][si]) {
        if (shell_argtable[1][si] == '.') {
            splitlen = si;
            exec_name[si] = 0;
            break;
        }
        si++;
    }
    memcpy(exec_name, shell_argtable[1], splitlen);
    memcpy(exec_ext, shell_argtable[1] + splitlen, 4);

    uint32_t start = fat_dir_search(shell_operating_dir, exec_name, 1, true); // name only, not extension. returns starting cluster
    if (start == 0) {
        shell_tty_print("\nFailed to locate.");
        return;
    }
    // Now we have the start, load our file to memory (based on file size)
    uint32_t exec_size = fat_cd_object.o_size;
    uint8_t* buffer = (uint8_t*)malloc(exec_size); // the thing we got from searching

    fat_file_read(start, buffer, exec_size);

    uint16_t exec_sig = ((uint16_t*)buffer)[0];
    uint32_t start_root = ((uint32_t*)(buffer + 2))[0];
    uint32_t start_offset = ((uint32_t*)(buffer + 6))[0];

    uint8_t *program_load = (uint8_t*)(start_root);
    
    if (exec_sig != 0x66BB) {
        shell_tty_print("\nInvalid or corrupt program");
        free(*buffer, exec_size);
        return;
    }

    memcpy(program_load, buffer, exec_size); // copy program into memory (specified later)
    free(*buffer, exec_size);

    // Now spawn the process
    uint32_t PID = process_create((uint32_t)(start_root + start_offset), false);
    if (PID == 0) {
        shell_tty_print("\nFailed to spawn process");
        return;
    }
    return;

}

void shell_tasklist() {
    shell_tty_clear();
    shell_tty_print("\nList of running tasks..");
    for (uint16_t i=0;i<MAX_PROCESSES;i++) {
        if (tasklist[i].flags) {
            char* p_id = uhex32_str(tasklist[i].PID);
            char* p_esp = uhex32_str(tasklist[i].esp);
            char* p_esp_start = uhex32_str(tasklist[i].esp_ptr);
            char* p_ptr = uhex32_str(tasklist[i].ret_addr);
            shell_tty_print("\n###############################\n\tPID: 0x");
            shell_tty_print(p_id);
            shell_tty_print("\n\tESP PTR: 0x");
            shell_tty_print(p_esp);
            shell_tty_print("\n\tESP START: 0x");
            shell_tty_print(p_esp_start);
            shell_tty_print("\n\tPTR: 0x");
            shell_tty_print(p_ptr);
            free(*p_id, 32);
            free(*p_esp, 32);
            free(*p_esp_start, 32);
            free(*p_ptr, 32);
        }
    }
}