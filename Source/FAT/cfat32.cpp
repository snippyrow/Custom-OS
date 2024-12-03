#include "FAT.h"

// CFAT24: Basic structure
// All clusters, like FAT32 rely on clusters. For simplicity here each cluster is 512B, or one sector of the disk.
// There are two core parts to the filesystem, the FAT (File Allocation Table) and the data region.
// When referring to a file, it is the cluster offset by the size of the FAT table offset by the total offset.
// The total offset is to seperate it from the boot code, usually 200 sectors is a good number.

// FAT structure:
// Each four bytes specify the properties of that cluster. For example, the first entry would corrospond to cluster 0.
// Files and directories appear the same, as it's just a file that gets treated differently
// Special FAT definitions:
// 0xFFFFFFFF - End-of-chain, for both directories and files.
// 0x00FFFFFF - Empty cluster. This can be written to.
// All FAT clusters defined are little-endian, where the smallest byte comes first.
// Clusters in the data region can be used by both indexing and raw data for files and directories

// When you jump to a directory, it simply reloads. Otherwise locate the first cluster
// Keep in mind that the starting cluster is in the directory object attributes

// Just below the FAT offset, there is a table

// In order to travel up to a parent directory, we find where we just came from and go there, since that's saved to memory.
// If we need to go up twice, each directory has the cluster start of it's parent.

// Inside a user directory, there is a special type of object called a navigator. It links up with info towards the parent directory struct.

// FAT Object Attributes:
// 0x00 - Empty
// 0x01 - User File
// 0x02 - User Directory
// 0x04 - System File
// 0x08 - System Directory
// 0x10 - Raw

uint32_t fat_ret_next(uint32_t cluster) {
    uint32_t lba = FAT_Offset + (cluster / 128);
    uint32_t* ata_buffer = (uint32_t*)malloc(512);
    uint32_t num;
    ATA_Read(lba, 1, (uint8_t*)ata_buffer);
    num = ata_buffer[(cluster) % 128];
    free(*ata_buffer, 512);
    return num;
}

void fat_update(uint32_t index, uint32_t new_cluster) {
    uint32_t lba = FAT_Offset + (index / 128);
    uint32_t* ata_buffer = (uint32_t*)malloc(512);
    ATA_Read(lba, 1, (uint8_t*)ata_buffer);

    ata_buffer[(index) % 128] = new_cluster;
    ATA_Write(lba, 1, (uint8_t*)ata_buffer);
    free(*ata_buffer, 512);
}

void fat_list_files(uint32_t dir_entry) { // dir_entry is the starting cluster of the directory
    bool nextFound = true;
    uint32_t cdir = dir_entry;
    uint32_t FAT_Size = FAT_Length / 128; // in sectors of the disk
    uint32_t fcount = 0;
    char oname[9];
    char oext[4];
    fat_object object;
    fat_object* ata_buffer = (fat_object*)malloc(512); // this works??
    while (nextFound) {
        // Go through the first item
        uint32_t lba = FAT_Offset + FAT_Size + cdir;
        ATA_Read(lba, 1, (uint8_t*)ata_buffer);
        for (uint8_t o=0;o<16;o++) { // can fit 16 32-byte objects into each sector of a chain
            object = ata_buffer[o];
            switch (object.attributes) {
                case 0:
                    break; // no file exists here
                case 1: {
                    memcpy(oname, &object.o_name, 8);
                    oname[8] = '\0';
                    memcpy(oext, &object.o_ext, 3);
                    oext[3] = '\0';
                    char* fsize = int64_str(object.o_size);
                    shell_tty_print("\n\t");
                    shell_tty_print(oname);
                    shell_tty_print(".");
                    shell_tty_print(oext);
                    shell_tty_print(" (");
                    shell_tty_print(fsize);
                    shell_tty_print("B)");
                    free(*fsize, 32);
                    fcount++;
                    break;
                }
                    // Regular user file
                case 2: {
                    memcpy(oname, &object.o_name, 8);
                    oname[8] = '\0';
                    char* fsize = int64_str(object.o_size);
                    shell_tty_print("\n\t--> /");
                    shell_tty_print(oname);
                    shell_tty_print("/ (");
                    shell_tty_print(fsize);
                    shell_tty_print("B)");
                    free(*fsize, 32);
                    fcount++;
                    break;
                }
                case 0x80: {
                    shell_tty_print("\n\t../");
                }
                    // Regular user directory
                default:
                    break;
            }
        }
        // Now look for the next item in the chain
        uint32_t next = fat_ret_next(cdir);
        if (next == EOC) {
            nextFound = false;
            //free((uint64_t*)ata_buffer, 512);
            return;
        } else {
            cdir = next;
        }
    }
}

uint32_t fat_search() { // search the FAT for the nearest unused cluster
    uint32_t* ata_buffer = (uint32_t*)malloc(512);
    uint32_t lba;
    uint32_t fat_index;
    for (uint32_t c=0;c<FAT_Length;c++) {
        // Load more
        fat_index = c % 128;
        if (fat_index == 0) {
            lba = FAT_Offset + (c / 128);
            ATA_Read(lba, 1, (uint8_t*)ata_buffer);
        }
        //shell_tty_print(uhex32_str(ata_buffer[fat_index]));
        
        if (ata_buffer[fat_index] == NONE) { // empty slot found, return
            free(*ata_buffer, 512);

            return c;
        }

    }
    free(*ata_buffer, 512);
    
    return EOC; // failed
}

// Creates a new file or subdirectory within a current directory
int fat_mko(fat_object new_o, uint32_t dir_entry) {
    // Usage for making a new user file
    fat_object* dir_sector = (fat_object*)malloc(512);
    uint8_t* dir_ptr = (uint8_t*)dir_sector; // used for ATA buffering
    uint32_t current_directory = dir_entry; // the current reading sector
    uint32_t FAT_Size = FAT_Length / 128; // in sectors of the disk
    uint32_t next_cluster;
    uint32_t data_avalible;
    bool newBlock = false;
    bool doWrite = false;

    for (uint32_t directory_index = 0; directory_index < Max_Objects_Per; directory_index++) { // recursively search directory structure
        if (directory_index == 0) { // start by populating
            ATA_Read(
                FAT_Offset + FAT_Size + current_directory, // where to read from
                1, // one sector
                dir_ptr
            );
        } else if (directory_index != 0 && (directory_index % 16 == 0)) { // If it is not starting and we get to the end load the next chunk
            next_cluster = fat_ret_next(current_directory);
            if (next_cluster != EOC) { // search the new chunk for space
                current_directory = next_cluster;
                ATA_Read(
                    FAT_Offset + FAT_Size + current_directory, // where to read from
                    1, // one sector
                    dir_ptr
                );
            } else {
                // Search for a new block to expand into
                next_cluster = fat_search();
                if (next_cluster == EOC) {
                    // Failed to find space in volume
                    free((uint64_t)dir_sector, 512);
                    return -1;
                }
                fat_update(current_directory, next_cluster);
                fat_update(next_cluster, EOC);
                current_directory = next_cluster;
                newBlock = true; // special since we know it's empty
            }
        }

        if (dir_sector[directory_index % 16].attributes && !newBlock) { // If there is an object then skip
            continue;
        } else {
            doWrite = true;
        }

        // Now we actually make our file
        // If this is a new block we can insert at the first position and end
        if (new_o.attributes != 0x80) {
            data_avalible = fat_search();
            if (data_avalible == EOC) {
                free((uint64_t)dir_sector, 512);
                return -1; // failed since no space
            }
            // If this is a navigator class, then this acts as the parent directory to save space
            new_o.cluster = data_avalible;
        }
        if (newBlock) {
            // First clear the buffer to rid it of residual (since no initial read)
            for (uint16_t x=0;x<512;x++) {
                dir_ptr[x] = 0;
            }

            memcpy(&dir_sector[0], &new_o, sizeof(fat_object));
            newBlock = false;
        } else {
            ATA_Read(
                FAT_Offset + FAT_Size + current_directory,
                1,
                dir_ptr
            );
            memcpy(&dir_sector[directory_index % 16], &new_o, sizeof(fat_object));
        }
        // Update data region
        if (doWrite) {
            ATA_Write(
                FAT_Offset + FAT_Size + current_directory,
                1,
                dir_ptr
            );
            if (new_o.attributes != 0x80) {
                fat_update(data_avalible, EOC);

                // Kindly erase the first sector just in case
                for (uint16_t c=0;c<512;c++) {
                    dir_ptr[c] = 0;
                }
                ATA_Write(FAT_Offset + FAT_Size + data_avalible, 1, dir_ptr);
            }
            free((uint64_t)dir_sector, 512);

            return data_avalible; // finished
        }
    }
    free((uint64_t)dir_sector, 512);
    return -1;
}

int fat_dir_search(uint32_t dir_entry, char* oname, uint8_t attrib, bool upd = false) {
    // Find first occouring object based on params
    fat_object* dir_sector = (fat_object*)malloc(512);
    uint8_t* dir_ptr = (uint8_t*)dir_sector; // used for ATA buffering
    uint32_t current_directory = dir_entry; // the current reading sector
    uint32_t FAT_Size = FAT_Length / 128; // in sectors of the disk
    uint32_t next_cluster;

    for (uint32_t directory_index = 0; directory_index < Max_Objects_Per; directory_index++) { // recursively search directory structure
        if (directory_index == 0) { // start by populating
            ATA_Read(
                FAT_Offset + FAT_Size + current_directory, // where to read from
                1, // one sector
                dir_ptr
            );
        } else if (directory_index != 0 && (directory_index % 16 == 0)) { // If it is not starting and we get to the end load the next chunk
            next_cluster = fat_ret_next(current_directory);
            if (next_cluster != EOC) { // search the new chunk for space
                current_directory = next_cluster;
                ATA_Read(
                    FAT_Offset + FAT_Size + current_directory, // where to read from
                    1, // one sector
                    dir_ptr
                );
            } else {
                // No more to be found
                free((uint64_t)dir_sector, 512);
                return -1;
            }
        }
        // Since attribute is cheaper, compare that first
        uint8_t o_index = directory_index % 16;
        if (dir_sector[o_index].attributes == attrib) {
            if (strcmp(dir_sector[o_index].o_name, oname) || attrib == 0x80) { // if name matches OR we need to find the navigator for a directory
                free((uint64_t)dir_sector, 512);
                if (upd) {
                    memcpy(&fat_cd_object, &dir_sector[o_index], 32);
                }
                return dir_sector[o_index].cluster;
            }
        }
    }
    free((uint64_t)dir_sector, 512);
    return -1;
}


// Re-writes file contents
int fat_file_touch(uint32_t file_start, uint8_t* data_begin, uint32_t size_t) {
    uint8_t* buffer = (uint8_t*)malloc(512);
    uint32_t c_chunk = file_start;
    uint32_t FAT_Size = FAT_Length / 128; // in sectors of the disk
    uint32_t lba = FAT_Offset + FAT_Size + c_chunk;
    uint32_t next;
    bool end = true;
    for (uint32_t i=0;i<size_t;i++) {
        uint16_t index = i % 512;
        buffer[index] = data_begin[i];
        if (index == 0 && i != 0) {
            ATA_Write(
                lba,
                1,
                buffer
            );
            next = fat_ret_next(c_chunk);
            if (next != EOC) {
                c_chunk = next;
                lba = FAT_Offset + FAT_Size + c_chunk;
                end = false;
            } else {
                // expand the file as needed
                next = fat_search();
                if (next == EOC) {
                    free(*buffer, 512);
                    return -1; // failed to find space in volume
                }
                fat_update(c_chunk, next);
                fat_update(next, EOC);
                c_chunk = next;
                lba = FAT_Offset + FAT_Size + c_chunk;
                end = true;
            }
        }
    }

    // Pad the rest of the remaining cluster with zero
    uint16_t remaining = size_t % 512;
    for (uint16_t i=remaining;i<512;i++) {
        buffer[i] = 0;
    }

    ATA_Write(
        lba,
        1,
        buffer
    );

    // Reduce file if we have now unused clusters. Deallocate all
    bool pruning = !end;
    while (pruning) {
        next = fat_ret_next(c_chunk);
        // read the first next from the writing function
        if (next == EOC) {
            free(*buffer, 512);
            return 0;
        } else {
            fat_update(next, NONE);
            c_chunk = next;
        }
    }
    free(*buffer, 512);
    return 0;
}

// fat length is in clusters (should be some increment of 512)
void fat_format(uint32_t fat_length) {
    // Write in the empty FAT table with a basic root directory

    fat_object starter;
    FAT_Length = fat_length;
    uint32_t FAT_Size = FAT_Length / 128; // in sectors of the disk
    uint32_t* ata_buffer = (uint32_t*)malloc(FAT_Size * 512); // Re-usable

    // First definition for root
    ata_buffer[0] = EOC; // end of chain for root directory (reserved sector)
    for (uint32_t a_id=1;a_id<FAT_Length;a_id++) {
        ata_buffer[a_id] = NONE;
    }
    ATA_Write(FAT_Offset, FAT_Size, (uint8_t*)ata_buffer); // apply changes to disk
    starter = {"home", "", 2, 0, 0, 0, 0, 0}; // empty home directory
    uint32_t home_place = fat_mko(starter, 0);
    starter = {"", "", 0x80, 0, 0, 0, 0, 0}; // navigator to root
    fat_mko(starter, home_place);
    starter = {"README", {'t','x','t'}, 1, 0, 0, 0, 169, 0}; // empty home directory
    fat_mko(starter, 0);
    free(*ata_buffer, FAT_Size * 512);
}