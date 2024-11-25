struct fat_object {
    char o_name[9];
    char o_ext[4];
    uint8_t attributes;
    uint16_t cluster_l;
    uint8_t cluster_h;
    uint32_t modified;
    uint32_t created;
    uint32_t o_size;
    uint32_t always0;
} __attribute__ ((packed));

struct fat_object* presup_object;
uint16_t FAT_Offset = 200;
uint32_t FAT_Sweep = 6; // start at the root directory starting in cluster 6
uint32_t FAT_Length = 1024;