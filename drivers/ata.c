#include "ata.h"
#include "task.h"

#define ATA_PRIMARY_IRQ 14
#define ATA_PRIMARY_IO 0x1F0
#define ATA_PRIMARY_DCR_AS 0x3F6

#define ATA_SECONDARY_IRQ 15
#define ATA_SECONDARY_IO 0x170
#define ATA_SECONDARY_DCR_AS 0x376

// #define ATA_PRIMARY 0x00
// #define ATA_SECONDARY 0x01

#define ATA_MASTER 0x00
#define ATA_SLAVE  0x01


static struct dev_t *ata_devices[4];
static int n_ata_devices;


uint8_t ata_pm = 0; /* Primary master exists? */
uint8_t ata_ps = 0; /* Primary Slave exists? */
uint8_t ata_sm = 0; /* Secondary master exists? */
uint8_t ata_ss = 0; /* Secondary slave exists? */

uint8_t *ide_buf = 0;

void ide_sel_drive(uint8_t bus, uint8_t i) {
	if ( bus == ATA_PRIMARY ) {
		if ( i == ATA_MASTER ) {
			outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xA0);
		}
		else outb(ATA_PRIMARY_IO + ATA_REG_HDDEVSEL, 0xB0);
	}
	else {
		if ( i == ATA_MASTER ) {
			outb(ATA_SECONDARY_IO + ATA_REG_HDDEVSEL, 0xA0);
		}
		else outb(ATA_SECONDARY_IO + ATA_REG_HDDEVSEL, 0xB0);
	}

}


void ide_poll(uint16_t io) {
	uint8_t status;
	inb(io + ATA_REG_ALTSTATUS);
	inb(io + ATA_REG_ALTSTATUS);
	inb(io + ATA_REG_ALTSTATUS);
	inb(io + ATA_REG_ALTSTATUS);

	while ((status = inb(io + ATA_REG_STATUS)) & ATA_SR_BSY);

	status = inb(io + ATA_REG_STATUS);
	if ( status & ATA_SR_ERR ) {
		// check error
		return;
	}
	return;
}
int ata_status_wait(int io_base, int timeout) {
	int status;
	

	if ( timeout > 0 ) {
		int i = 0;
		while ( (status = inb(io_base + ATA_REG_STATUS)) & ATA_SR_BSY && (i < timeout)) i++;
	} else {
		while ((status = inb(io_base + ATA_REG_STATUS)) & ATA_SR_BSY);
	}
	return status;
}

int ata_wait(int io, int adv)
{
    uint8_t status = 0;

    ata_io_wait(io);

    status = ata_status_wait(io, -1);

    if (adv) {
        status = inb(io + ATA_REG_STATUS);
        if (status & ATA_SR_ERR) return 1;
        if (status & ATA_SR_DF)  return 1;
        if (!(status & ATA_SR_DRQ)) return 1;
    }

    return 0;
}
/*uint8_t ide_ident(uint8_t bus, uint8_t drive) {
	uint16_t io = 0;
	ide_sel_drive(bus,drive);

	if ( bus == ATA_PRIMARY ) io = ATA_PRIMARY_IO;
	else io = ATA_SECONDARY_IO;


	outb(io + ATA_REG_SECCOUNT0,0);
	outb(io + ATA_REG_LBA0,0);
	outb(io + ATA_REG_LBA1,0);
	outb(io + ATA_REG_LBA2,0);

	outb(io + ATA_REG_COMMAND, ATA_CMD_IDENTIFY);

	uint8_t stat = inb(io + ATA_REG_STATUS);
	if ( stat ) {
		while (inb(io + ATA_REG_STATUS) & ATA_SR_BSY != 0);
stat_read:
		status = inb(io + ATA_REG_STATUS);
		if ( status & ATA_SR_ERR ) {
			return 0;
		}

		while(!(status & ATA_SR_DRQ)) goto stat_read;
		
		// set_task(0);
		for (int i = 0; i < 256; i++) {
			*(uint16_t*) ( ide_buf + i*2 ) = inw(io + ATA_REG_DATA);
		}
		// set_task(1);
	}
}
*/

int ata_read_sector_pio(char* buf,size_t lba) {
	uint16_t io = ATA_PRIMARY_IO;
	uint8_t dr = ATA_MASTER;

	uint8_t cmd = 0xE8;
	int err = 0;
	uint8_t slavebit = 0x00;
try_read:
	outb(io + ATA_REG_CONTROL,0x02);

	ata_wait(io,0);
	
	outb(io + ATA_REG_HDDEVSEL, (cmd | (uint8_t)((lba >> 24 & 0x0F))));
    outb(io + ATA_REG_FEATURES, 0x00);
    outb(io + ATA_REG_SECCOUNT0, 1);
    outb(io + ATA_REG_LBA0, (uint8_t)(lba));
    outb(io + ATA_REG_LBA1, (uint8_t)(lba >> 8));
    outb(io + ATA_REG_LBA2, (uint8_t)(lba >> 16));
    outb(io + ATA_REG_COMMAND, ATA_CMD_READ_PIO);

	if (ata_wait(io,1)) {
		err++;
		if ( err > 4 ) return -0x20;
		goto try_read;
	}

	for (int i = 0; i < 256; i++) {
		uint16_t d = inw(io + ATA_REG_DATA);
		*(uint16_t *)(buf + i*2) = d;
	}
	ata_wait(io,0);
	return 0;

}
int ata_write_sector_pio(uint16_t *buf, size_t lba)
{
    uint16_t io = ATA_PRIMARY_IO;
    uint8_t  dr = ATA_MASTER;

    uint8_t cmd = 0xE0;
    uint8_t slavebit = 0x00;

    outportb(io + ATA_REG_CONTROL, 0x02);

    ata_wait(io, 0);

    outb(io + ATA_REG_HDDEVSEL, (cmd | (uint8_t)((lba >> 24 & 0x0F))));
    ata_wait(io, 0);
    outb(io + ATA_REG_FEATURES, 0x00);
    outb(io + ATA_REG_SECCOUNT0, 1);
    outb(io + ATA_REG_LBA0, (uint8_t)(lba));
    outb(io + ATA_REG_LBA1, (uint8_t)(lba >> 8));
    outb(io + ATA_REG_LBA2, (uint8_t)(lba >> 16));
    outb(io + ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);
    ata_wait(io, 0);

    for (int i = 0; i < 256; i++) {
        outw(io + ATA_REG_DATA, buf[i]);
        asm volatile("nop; nop; nop");
    }
    outb(io + 0x07, ATA_CMD_CACHE_FLUSH);

    ata_wait(io, 0);

    return 0;
}
