struct fat_object {
    char o_name[8];
    char o_ext[3];
    uint8_t attributes;
    uint32_t cluster; // The cluster the file starts at, so the next can be found
    uint32_t modified;
    uint32_t created;
    uint32_t o_size;
    uint32_t parent_cluster;
} __attribute__ ((packed));

fat_object fat_cd_object;

struct fat_object* presup_object;
uint16_t FAT_Offset = 200;
uint32_t Root_Cluster = 0; // start at the root directory starting in cluster 6
uint32_t FAT_Length = 1024; // 1024 clusters defined in FAT
uint32_t Sectors_Per_Cluster = 1;
uint32_t Max_Objects_Per = 256;

#define EOC 0xFFFFFFFF
#define NONE 0x00FFFFFF