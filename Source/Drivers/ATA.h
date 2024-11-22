#define ATA_IO_BASE 0x1F0   // Primary ATA I/O base
#define ATA_PRIMARY_CONTROL 0x3F6   // Control port for primary ATA

// ATA registers (offsets from I/O base)
#define ATA_REG_DATA 0x00    // Data register (16 bits)
#define ATA_REG_ERROR 0x01    // Error register (read) / Features (write)
#define ATA_REG_SECTOR_COUNT 0x02    // Sector count register
#define ATA_REG_LBA_LOW 0x03    // LBA low byte
#define ATA_REG_LBA_MID 0x04    // LBA mid byte
#define ATA_REG_LBA_HIGH 0x05    // LBA high byte
#define ATA_REG_DRIVE_SELECT 0x06    // Drive select
#define ATA_REG_COMMAND 0x07    // Command register (write)
#define ATA_REG_STATUS 0x07    // Status register (read)

// ATA commands
#define ATA_CMD_READ_SECTORS 0x20    // Read with retries

// Status flags
#define ATA_SR_BSY 0x80    // Busy
#define ATA_SR_DRDY 0x40    // Drive ready
#define ATA_SR_DRQ 0x08    // Data request ready

// PCI stuff
#define PCI_CONFIG_ADDR    0xCF8
#define PCI_CONFIG_DATA    0xCFC