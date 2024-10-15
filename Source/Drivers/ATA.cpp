#include "ATA.h"

// Wait until the drive is not busy
void ATA_Wait_Busy(uint16_t io_base) {
    while (inb(io_base + ATA_REG_STATUS) & ATA_SR_BSY);
}

// Wait until the drive is ready to transfer data
void ATA_Wait_Ready(uint16_t io_base) {
    while (!(inb(io_base + ATA_REG_STATUS) & ATA_SR_DRQ));
}

// Read a sector from the primary ATA drive to a 512-byte buffer
void ATA_Read(uint32_t lba, uint8_t sectors, uint8_t *buffer) {
    // Select the master drive and specify the LBA
    outb(ATA_IO_BASE + ATA_REG_DRIVE_SELECT, 0xE0 | ((lba >> 24) & 0x0F));  // 0xE0 selects master drive
    outb(ATA_IO_BASE + ATA_REG_SECTOR_COUNT, sectors);                      // Read sectors
    outb(ATA_IO_BASE + ATA_REG_LBA_LOW, lba & 0xFF);                        // LBA low byte
    outb(ATA_IO_BASE + ATA_REG_LBA_MID, (lba >> 8) & 0xFF);                 // LBA mid byte
    outb(ATA_IO_BASE + ATA_REG_LBA_HIGH, (lba >> 16) & 0xFF);               // LBA high byte

    // Send the "read sectors" command
    outb(ATA_IO_BASE + ATA_REG_COMMAND, ATA_CMD_READ_SECTORS);

    // Wait for the drive to signal that data is ready
    ATA_Wait_Busy(ATA_IO_BASE);
    ATA_Wait_Ready(ATA_IO_BASE);

    // Read the 512 bytes from the data register into the buffer
    for (int i = 0; i < (512 / 2) * sectors; i++) {
        uint16_t data = inw(ATA_IO_BASE + ATA_REG_DATA);
        ((uint16_t *)buffer)[i] = data;  // Read 16 bits at a time
    }
}