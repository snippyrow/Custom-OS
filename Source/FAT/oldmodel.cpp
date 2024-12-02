#include "FAT.h"

// Define a FAT. Specs:
// Each cluster is exactly one sector long, and the FAT has three-byte entries.
// The FAT can hold a maximum of 1024 clusters
// The FAT exists at LBA 202-208
// Root directory entry starts in sector (LBA 209)
// All FAT entries are little-endian
// If a FAT cluster entry is 0, it is avalible. If it is 0xFFFFFF then it is the end of a chain
// Actual sectors (clusters) are offset quite a lot, beyond the code.

// For testing, all sectors are offset by 200. Therefore the root directory entry file is at sector 200.
// Each item in an entry directory is stored as a 32-byte object
// 9 Bytes for a name, 4 end bytes for the file extension
// 1 Byte for attributes (directory, file, system, etc)
// 3 Bytes for the starting cluster in the chain
// 4 Bytes for last modified
// 4 Bytes for creation date
// 4 Bytes for filesize
// 3 Bytes are reserved

// Memory map (offset by 200 sectors):
// 0-5: FAT
// 6: First sector for root directory entry
// 7: README test file
// 8: Example subdirectory

char rootname[2] = "/";

void fat_format() {
    uint8_t* format_sector = (uint8_t*)malloc(512); // Re-usable
    for (uint16_t x = 0;x<512;x++) { // Clear out any existing stuff
        if (x%3 == 2) {
            format_sector[x] = 0x00;
        } else {
            format_sector[x] = 0xff;
        }
    }

    for (uint8_t s=1;s<6;s++) {
        ATA_Write(s+FAT_Offset, 1, format_sector);
    }
    format_sector[6*3] = 0xff; // little endian (first byte of 7th cluster/sector)
    format_sector[6*3+1] = 0xff;
    format_sector[6*3+2] = 0xff; // end of chain for root directory

    // Now for README file
    format_sector[7*3] = 0xff;
    format_sector[7*3+1] = 0xff;
    format_sector[7*3+2] = 0xff; // end of file

    ATA_Write(FAT_Offset, 1, format_sector);

    for (uint16_t x = 0;x<512;x++) { // Clear out any existing stuff in prep to add file
        format_sector[x] = 0;
    }

    // Write a basic root '/' directory
    // When reading this, consult the FAT table and look for the next sector.
    // * The FAT table is really short right now and only handles 1024 sectors
    fat_object example_readme = {
        "EXTREAD", // this is to exclude a null terminator as it is not necessary when handling
        "txt",
        0b00000001, // this is a regular user file for attributes. If 0 it does not exist
        7, // actually LBA 210
        0, // out of range anyways
        0,
        0,
        512 // example, not practically used by the filesystem
    };
    fat_object example_dir = {
        "Subdir", // this is to exclude a null terminator as it is not necessary when handling
        "",
        0b00000010, // this is a regular user file for attributes. If 0 it does not exist
        8, // actually LBA 210
        0, // out of range anyways
        0,
        0,
        0 // example, not practically used by the filesystem
    };
    memcpy(format_sector,&example_readme,sizeof(example_readme)); // copy our file here into the directory
    memcpy(format_sector+32,&example_dir,sizeof(example_readme));
    ATA_Write(206, 1, format_sector);

    for (uint16_t x = 0;x<512;x++) { // Write W for the entire readme file
        format_sector[x] = 'W';
    }
    ATA_Write(207, 1, format_sector);

}

// retrieve the next FAT entry from disk
// Basically, cluster is the current cluster and we are looking for the next one.
// 170 clusters per sector of FAT floor(512/3)

uint32_t fat_ret_next(uint32_t cluster) {
    uint32_t lba = (cluster / 170) + FAT_Offset;
    uint8_t* ata_read_buffer = (uint8_t*)malloc(512);
    ATA_Read(lba, 1, ata_read_buffer);
    uint32_t next = 0;
    uint16_t start_addr = (cluster % 170) * 3;
    next = next | ata_read_buffer[start_addr];
    next = next | (ata_read_buffer[start_addr+1] << 8);
    next = next | (ata_read_buffer[start_addr+2] << 16);
    free(*ata_read_buffer, 512);
    return next;
}

void fat_list_files() {
    uint8_t* fat_read_table = (uint8_t*)malloc(512);
    shell_tty_print("\nFiles for '");
    shell_tty_print(rootname);
    shell_tty_print("':");
    uint32_t fcount = 0;
    uint32_t fat_c = 6;
    uint32_t nextDirectoryCluster = fat_c;
    // List files in current directory (from start sector of current directory)
    bool locatedNext = true;
    while (locatedNext) {
        ATA_Read(FAT_Offset+fat_c, 1, fat_read_table);
        for (uint16_t id=0;id<170;id++) {
            presup_object = (struct fat_object*)(fat_read_table + (32 * id));
            if (presup_object -> attributes & 1) { // If it exists and is a normal file
                fcount++;
                char* fsize = int64_str(presup_object -> o_size);
                shell_tty_print("\n\t");
                shell_tty_print(presup_object -> o_name);
                shell_tty_print(".");
                shell_tty_print(presup_object -> o_ext);
                shell_tty_print(" (");
                shell_tty_print(fsize);
                shell_tty_print("B)");
                free(*fsize, 32);
            } else if (presup_object -> attributes & 2) { // If it exists and is a normal directory
                fcount++;
                char* fsize = int64_str(presup_object -> o_size);
                shell_tty_print("\n\t/");
                shell_tty_print(presup_object -> o_name);
                shell_tty_print("/");
                shell_tty_print(" (");
                shell_tty_print(fsize);
                shell_tty_print("B)");
                free(*fsize, 32);
            }
        }
        nextDirectoryCluster = fat_ret_next(fat_c);
        if (nextDirectoryCluster == 0x00FFFFFF || !nextDirectoryCluster) { // end of chain
            locatedNext = false;
        } else {
            fat_c = nextDirectoryCluster;
        }
    }
    if (!fcount) {
        shell_tty_print("\n\tNo files located.");
    }
    free(*fat_read_table,512);
}

uint32_t fat_search() { // search the FAT for the nearest unused cluster
    uint32_t c_sec = 0;
    uint8_t* fat_tmp = (uint8_t*)malloc(512);
    ATA_Read(FAT_Offset, 1, fat_tmp); // read first item
    for (uint32_t c=0;c<FAT_Length;c++) {
        uint32_t ce = c / 170;
        if (ce != c_sec) {
            c_sec = ce;
            ATA_Read(FAT_Offset + c_sec, 1, fat_tmp);
        }

        // Now go through it
        uint16_t ptr_start = (c % 170) * 3;
        if (fat_tmp[ptr_start] == 0xFF && fat_tmp[ptr_start+1] == 0xFF && fat_tmp[ptr_start+2] == 0x00) { // if the item is 0x00FFFF (24-bit)
            free(*fat_tmp, 512);
            return c;
        }
    }
    free(*fat_tmp, 512);
    return 0xffffffff;
}

void fat_update(uint32_t fat_cluster, uint32_t new_cluster) {
    // Find the right disk dector, pull it, update and put it back
    uint32_t ce = (fat_cluster / 170) + FAT_Offset;
    uint8_t* fat_tmp = (uint8_t*)malloc(512);
    uint16_t ptr_start = (ce % 170) * 3;
    ATA_Read(ce, 1, fat_tmp);
    fat_tmp[ptr_start] = new_cluster;
    fat_tmp[ptr_start+1] = new_cluster >> 8;
    fat_tmp[ptr_start+2] = new_cluster >> 16;
    ATA_Write(ce, 1, fat_tmp);
    free(*fat_tmp, 512);
}

int fat_mkfile(fat_object new_o, uint32_t dir_entry) {
    // Just be sure that this is a real directory..
    // Loop through all directory contents and look for an avalible file spot.
    // If none is found, and it is the end of the chain, add a new link.
    uint8_t* fat_read_table = (uint8_t*)malloc(512);
    bool locatedNext = false;
    uint8_t current_cluster = dir_entry;
    uint32_t lba = current_cluster + FAT_Offset;
    uint32_t next_avalible;

    while (!locatedNext) { // keep searching until we find it
        ATA_Read(lba, 1, fat_read_table);
        for (uint8_t id=0;id<16;id++) {
            presup_object = (struct fat_object*)(fat_read_table + (32 * id));
            if (!presup_object -> attributes) { // file does not exist here, free realestate. Change contents
                // Also link it with a spare cluster
                uint32_t free_cluster = fat_search();
                if (free_cluster = 0xFFFFFFFF) { // none found
                    return -1; // failed to make file
                } else { // update it
                    fat_update(free_cluster, 0xFFFFFF); // new end of chain for file
                }
                presup_object -> cluster_l = free_cluster;
                presup_object -> cluster_h = (free_cluster >> 16);

                memcpy(fat_read_table + (32 * id), &new_o, sizeof(new_o));
                locatedNext = true;
                ATA_Write(lba, 1, fat_read_table);
                free(*fat_read_table, 512);
                return 1; // done, written file, return
            }
        }
        // None here, go to the next one
        uint32_t nextDirectoryCluster = fat_ret_next(current_cluster);
        shell_tty_print(uhex32_str(nextDirectoryCluster));
        if (nextDirectoryCluster == 0x00FFFFFF || !nextDirectoryCluster) {
            // End of chain, add another item and set the first entry.
            // When a new cluster is found, it assumes that it's completely empty. A FAT object with 0x00FFFF is an unused block.
            // A good chain should *never* contain a cluster with 0x00FFFF

            // Find the closest avalible cluster and link it.
            // Means setting our end of chain to another cluster and making that one the new end
            next_avalible = fat_search();
            if (next_avalible = 0xFFFFFFFF) { // none found
                return -1; // failed to make file
            } else {
                fat_update(current_cluster, next_avalible);
                fat_update(next_avalible, 0xFFFFFF); // new end of chain
                current_cluster = next_avalible;
                lba = current_cluster + FAT_Offset;
            }

        } else {
            current_cluster = nextDirectoryCluster;
            lba = current_cluster + FAT_Offset;
        }
    }
}