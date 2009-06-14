#include <driver.h>

#include <sys/ioctl.h>
#include <sys/tty.h>
#include <prex/ioctl.h>

#include "../../../sys/arch/arm/spmp/platform.h"

/* Forward functions */
static int nand_init(void);
static int nand_open(device_t dev, int mode);
static int nand_close(device_t dev);
static int nand_read(device_t, char *, size_t *, int);
static int nand_write(device_t, char *, size_t *, int);
static int nand_ioctl(device_t, u_long, void *);

#define MAX_PAGE_SIZE		8192

static uint8_t page_buf[MAX_PAGE_SIZE];
static uint16_t translate[0x1F20];

uint32_t le32(const uint8_t *p) {
        return (p[3] << 24) | (p[2] << 16) | (p[1] << 8) | p[0];
}

uint16_t le16(const uint8_t *p) {
        return (p[1] << 8) | p[0];
}

nand_ioc_struct nand_info = {0,0,0,0};

/*
 * Driver structure
 */
struct driver nand_drv = {
	/* name */	"nand",
	/* order */	4,
	/* init */	nand_init,
};

/*
 * Device I/O table
 */
static struct devio nand_io = {
	/* open */	nand_open,
	/* close */	nand_close,
	/* read */	nand_read,
	/* write */	nand_write,
	/* ioctl */	nand_ioctl,
	/* event */	NULL,
};

static device_t nand_dev[16];	/* device objects */

void NAND_Init(void) {
	NAND_ENABLE = 1;
	NAND_CTRL_HI |= 0x3;
	NAND_CTRL_HI &= ~(NAND_CTRL_CLE | NAND_CTRL_ALE);
	DEV_ENABLE |= 0x1;
	DEV_ENABLE_OUT |= 0x1;
	NAND_CTRL_HI &= ~0x1;
}

void NAND_StrobeRead(void) {
	volatile uint8_t* nand_base = ((volatile uint8_t*)NAND_BASE);
	uint8_t temp = NAND_CTRL_LO;
	NAND_CTRL_LO = temp | NAND_CTRL_RE;
	nand_base[0x25] = 0x1;
	NAND_CTRL_LO = temp;
}

uint8_t NAND_ReadByte(void) {	
	return NAND_DATA;
}

void NAND_WriteCmd(uint8_t cmd) {
	NAND_CTRL_HI = (NAND_CTRL_HI & ~NAND_CTRL_ALE) | NAND_CTRL_CLE;
	NAND_DATA = cmd;
	NAND_CTRL_HI = (NAND_CTRL_HI & ~NAND_CTRL_ALE) | NAND_CTRL_CLE;
	NAND_CTRL_HI &= ~NAND_CTRL_CLE;
}

void NAND_WriteAddr(uint32_t page_no, uint32_t col_no) {
	NAND_CTRL_HI = (NAND_CTRL_HI & ~NAND_CTRL_CLE) | NAND_CTRL_ALE;
	NAND_DATA = col_no & 0xFF;
	NAND_DATA = (col_no >> 8);
	NAND_DATA = (page_no & 0xFF);
	NAND_DATA = (page_no >> 8) & 0xFF;
	NAND_DATA = (page_no >> 16) & 0x0F;
	NAND_CTRL_HI = (NAND_CTRL_HI & ~NAND_CTRL_CLE) | NAND_CTRL_ALE;
	NAND_CTRL_HI &= ~NAND_CTRL_ALE;
}

/* Possibly broken, should check in disasm */
int NAND_WaitReadBusy(void) {
	int i = 0;
	for(; i < 3000; i++){
		if(!(NAND_STATUS & NAND_STATUS_READ_BUSY))
			return 0;
	}
	return -1;
}

int NAND_WaitCmdBusy(void) {
	int i = 0;
	for(; i < 50000; i++){
		if((NAND_STATUS2 & NAND_STATUS_CMD_READY))
			return 0;
	}
	return -1;	
}

/* Fills the buf with 5 chars worth of nand_id */
int NAND_ReadID(char* buf) {
	int i = 0, ret = 0;
	NAND_WriteCmd(0x90);
	NAND_WriteAddr(0, 0);
	NAND_WaitCmdBusy();
	NAND_WaitReadBusy();
	
	for(; i < 5; i++){
		NAND_StrobeRead();
		if(NAND_WaitReadBusy())
			ret = -1;
		buf[i] = NAND_DATA;
		ret |= buf[i];
	}
	if(ret)
		return 0;
	return -1;
}

void NAND_ReadPage(void* buf, int page_no) {
	int i = 0;
	uint8_t* charbuf = (uint8_t*)buf;
	NAND_WriteCmd(0x00);
	NAND_WriteAddr(page_no, 0);
	NAND_WriteCmd(0x30);
	NAND_WaitCmdBusy();
	NAND_WaitReadBusy();

	for(; i < nand_info.nand_bytes_per_page + nand_info.nand_spare_per_page; i++) {
		NAND_StrobeRead();
		/* NAND_WaitReadBusy(); */
		charbuf[i] = NAND_ReadByte();
	}
	NAND_Init();
}

void NAND_ReadHead(void* buf, int page_no) {
	int i = 0;
	uint8_t* charbuf = (uint8_t*)buf;
	NAND_WriteCmd(0x00);
	NAND_WriteAddr(page_no, 0);
	NAND_WriteCmd(0x30);
	NAND_WaitCmdBusy();
	NAND_WaitReadBusy();

	for(; i < 0x10; i++) {
		NAND_StrobeRead();
		/* NAND_WaitReadBusy(); */
		charbuf[i] = NAND_ReadByte();
	}
	NAND_Init();
}

void NAND_ReadPageSpare(void * buf, int page_no) {
	int i = 0;
	uint8_t* charbuf = (uint8_t*)buf;
	NAND_WriteCmd(0x00);
	NAND_WriteAddr(page_no, nand_info.nand_bytes_per_page);
	NAND_WriteCmd(0x30);
	NAND_WaitCmdBusy();
	NAND_WaitReadBusy();

	for(; i < nand_info.nand_spare_per_page; i++){
		NAND_StrobeRead();
		charbuf[i] = NAND_ReadByte();
	}
	NAND_Init();
}

void NAND_ReadRsvBlk(void) {
	NAND_ReadPage(page_buf, 0);
	char asciiNamebuf[128];
	strncpy(asciiNamebuf, page_buf, 8);
	asciiNamebuf[8] = '\0';
	printf("Read NANDRsv block: asciiName = [%s] signature = [0x%x] checksum = [%x]\n", 
		asciiNamebuf, le16(page_buf + 8), le16(page_buf + 0xe));
	printf("nrFwBlks = [0x%x] fwStartBlk = [0x%x] nrFwBlks2 = [0x%x] fwStartBlk2 = [0x%x]\n",
		le32(page_buf + 0x14), le32(page_buf + 0x10), le32(page_buf + 0x20), le32(page_buf + 0x1c));
	printf("nrRsvA = [0x%x] nrRsvB = [0x%x] nrRsvC = [0x%x] nrRsvBlks = [0x%x] checksum = [0x%x]\n",
		le16(page_buf + 0x24), le16(page_buf + 0x26), le16(page_buf + 0x28), le16(page_buf + 0xc));
}

/* Returns the page containing the blockmap */
int NAND_FillBlockmap(void) {
	int i = 0, j;
	uint16_t page;
	int* buf = (int*)page_buf;
	for(; i < nand_info.nand_num_blocks; i++) {
		NAND_ReadPageSpare(page_buf, i * nand_info.nand_pages_per_block);
			if(page_buf[0] == 0xFF && page_buf[0x1] != 0xFF){
				page = (((int)page_buf[0x1]) << 8) | page_buf[0x2];
				page &= 0xFFF;
				translate[page] = i;
				printf("spare@%04X: %02X %02X %02X %02X\n",
					i, page_buf[0],page_buf[1],page_buf[2],page_buf[3]);
				printf("Added translation from %04X to %04X\n", page, i);
/*				break; */
			}
/*			NAND_ReadHead(page_buf, i * nand_info.nand_pages_per_block + j);
			if (strncmp(page_buf, "OHAI", 4) == 0) {
				printf("found OHAI @ page %x (block %x)\n", i * nand_info.nand_pages_per_block + j, i);
				return 0;
			} */
		}
	return 0;
}

static int nand_init(void) {
	char nand_id[5];
	int blockmap;
	/* Create NAND device as an alias of the registered device. */
	nand_dev[0] = device_create(&nand_io, "nand", DF_BLK);
	if (nand_dev[0] == DEVICE_NULL)
		return -1;

	nand_dev[1] = device_create(&nand_io, "nand1", DF_BLK);
	if (nand_dev[1] == DEVICE_NULL)
		return -1;
	
	NAND_Init();

	if (NAND_ReadID(nand_id)) {
		printf("No NAND chip found.\n");
		return -1;
	}		

	printf("NAND Chip found, id: %02X %02X %02X %02X %02X\n",
			nand_id[0], nand_id[1],	nand_id[2],	nand_id[3],	nand_id[4]);

	uint32_t page_size = 1024 << (nand_id[3] & 3);
	uint32_t block_size = 65536 << ((nand_id[3] & 0x30) >> 4);
	uint32_t spare_size = page_size * ((nand_id[3] & 4) ? 16:8) / 512;
	uint32_t word_width = (nand_id[3] & 0x40) ? 16:8;
	uint32_t num_planes = 1 << ((nand_id[4] & 0xC) >> 2);
	uint32_t plane_size = 64 << ((nand_id[4] & 0x70) >> 4); /* in megabits */
	uint32_t num_blocks = num_planes * plane_size * (1048576 / 8) / block_size;
	switch (nand_id[0]) {
		case 0xEC: printf("Vendor: Samsung\t"); break;
		case 0xAD: printf("Vendor: Hynix\t"); break;
		default:   printf("Vendor: Unknown\t"); break;
	}
	
	printf("Capacity: %u plane%s x %u MB/plane = %u MB\n", num_planes, num_planes>1?"s":"",
		plane_size/8, num_planes * plane_size/8);
	
	printf("Geometry: (%u + %u) bytes/page, %u pages/block, %u blocks\n",
		page_size, spare_size, block_size / page_size, num_blocks);
	
	nand_info.nand_num_blocks = num_blocks;
	nand_info.nand_pages_per_block = block_size / page_size;
	nand_info.nand_bytes_per_page = page_size;
	nand_info.nand_spare_per_page = spare_size;
	
/*	printf("Capabilities: CachePgm: %s\tInterleave: %s\tSimulPages: %d\t%d levels/cell\tChipNo: %d\n",
		(nand_id[2]&0x80)?"Yes":"No", (nand_id[2]&0x40)?"Yes":"No", 1 << ((nand_id[2]&0x30) >> 4),
		2 << ((nand_id[2]&0x0C) >> 2), 1 << (nand_id[2]&3)); */

		NAND_ReadRsvBlk();
	printf("Creating blockmap...\n");
	blockmap = NAND_FillBlockmap();
	/* NAND_ReadSector(translate, 2); */
	return 0;
}

/* Horribly non-optimized, but it's late right now... */
static int nand_read(device_t dev, char *buf, size_t *nbyte, int blkno) {
	uint8_t* kbuf = kmem_map(buf, *nbyte);
	size_t todo = *nbyte;
	int i = 0;
	int sector, sub_sector, block;
	/* Temporary addition */
	/* blkno += 0x3E5000; This is AIMG */
	/* blkno += 0x30000;  Main FAT partition  */
	printf("Reading %d bytes from page %d.\n",todo, blkno);
	if(dev == nand_dev[0]){ /* Main device, no adjusting */

	}
	
	if(dev == nand_dev[1]){	/* Firmware device, use blockmap */
		/* blkno += 7168; */
		block = translate[(blkno * 512) / (nand_info.nand_pages_per_block * nand_info.nand_bytes_per_page)];
		blkno = (block * (nand_info.nand_pages_per_block * nand_info.nand_bytes_per_page) + 
			(blkno * 512) % (nand_info.nand_pages_per_block * nand_info.nand_bytes_per_page)) / 512;
		printf("Reading from 0x%08X\n", blkno * 512);
	}
	

	if(!kbuf)
		return EFAULT;
/*
	while(todo > 0){
		sector = blkno / 4;
		sub_sector = blkno % 4;
		NAND_ReadPage(page_buf, sector);
		if(todo > nand_info.nand_bytes_per_page) {
			memcpy(kbuf, page_buf, nand_info.nand_bytes_per_page);
			todo -= nand_info.nand_bytes_per_page;
			blkno += 4;
			kbuf += nand_info.nand_bytes_per_page;
		}
		else
		{
			memcpy(kbuf, page_buf + (512 * sub_sector), todo);
			todo = 0;
		}
		blkno++;
	}
	*/

	NAND_ReadPage(page_buf, blkno);
/*	NAND_ReadPageSpare(page_buf + nand_info.nand_bytes_per_page, blkno);*/
	memcpy(kbuf, page_buf, todo);
	return 0;
}

static int nand_write(device_t dev, char *buf, size_t *nbyte, int blkno) {
	return *nbyte;
}

static int nand_ioctl(device_t dev, u_long cmd, void *arg) {
	switch(cmd) {
		case NANDIOC_GET_INFO:
                if (umem_copyout(&nand_info, arg, sizeof(nand_info)))
                        return EFAULT;
			return 0;
		
	}
	printf("Whoops, wrong ioctl received!\n");
	return -1;
}

static int nand_open(device_t dev, int mode) {
	printf("NAND opened.\n");
	return 0;
}

static int nand_close(device_t dev) {
	return 0;
}

