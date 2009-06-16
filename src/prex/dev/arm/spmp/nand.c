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
static uint16_t *translate_user = NULL, *mapTable = NULL, *translate_fw = NULL, 
	*translate_rsvA = NULL, *translate_rsvB = NULL;
/* Not yet implemented, because I haven't seen these used on a device
static uint16_t *translate_rsvC = NULL, *translate_fw2 = NULL; */

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
/*	printf("NAND ADDR: %02x %02x %02x %02x %02x\n",
		col_no & 0xFF, (col_no >> 8), page_no & 0xFF, (page_no >> 8) & 0xFF, (page_no >> 16) & 0xFF); */
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

typedef struct {
	char asciiName[8];
	uint16_t signature;
	uint16_t nrUserBlks;
	uint16_t nrRsvBlks;
	uint16_t checksum;
	uint32_t fwStartBlk;
	uint32_t nrFwBlks;
	uint32_t unk1;
	uint32_t fwStartBlk2;
	uint32_t nrFwBlks2;
	uint16_t nrRsvA;
	uint16_t nrRsvB;
	uint16_t nrRsvC;
	uint16_t unk2;
	uint32_t unk3;
	uint32_t unk4;
} __packed nandrsv_header_t;

static nandrsv_header_t nandrsv_header;

void NAND_ReadRsvBlk(void) {
	NAND_ReadPage(page_buf, 0);
	printf("sizeof header = %x page_buf=%x rsv_header=%x\n", sizeof(nandrsv_header),
		(uint32_t)page_buf, (uint32_t)&nandrsv_header);
	memcpy(&nandrsv_header, page_buf, sizeof(nandrsv_header));
	char asciiNamebuf[128];
	strncpy(asciiNamebuf, nandrsv_header.asciiName, sizeof(nandrsv_header.asciiName));
	asciiNamebuf[sizeof(nandrsv_header.asciiName)] = '\0';
	printf("Read NANDRsv block: asciiName = [%s] signature = [0x%x] checksum = [%x]\n", 
		asciiNamebuf, nandrsv_header.signature, nandrsv_header.checksum);
	printf("nrFwBlks = [0x%x] fwStartBlk = [0x%x] nrFwBlks2 = [0x%x] fwStartBlk2 = [0x%x]\n",
		nandrsv_header.nrFwBlks, nandrsv_header.fwStartBlk,
		nandrsv_header.nrFwBlks2, nandrsv_header.fwStartBlk2);
	printf("nrRsvA = [0x%x] nrRsvB = [0x%x] nrRsvC = [0x%x] nrRsvBlks = [0x%x]\n",
		nandrsv_header.nrRsvA, nandrsv_header.nrRsvB, nandrsv_header.nrRsvC, nandrsv_header.nrRsvBlks);
	printf("nrUserBlks = %x unk1 = %x unk2 = %x unk3 = %x unk4 = %x\n",
		nandrsv_header.nrUserBlks, nandrsv_header.unk1, nandrsv_header.unk2, 
		nandrsv_header.unk3, nandrsv_header.unk4);
}

static uint16_t NAND_GetLogicalBlkno(uint16_t physical_block) {
	uint8_t buf[256];

	NAND_ReadPageSpare(buf, physical_block * nand_info.nand_pages_per_block);
/*	printf("block %x buf=[%02x %02x %02x %02x]\n", physical_block, buf[0], buf[1], buf[2], buf[3]); */
	if (buf[0] != 0xFF) return 0xFFFE; /* bad block ? */
	
	/* Spare area has format FF aa bb xx xx xx xx xx xx xx... 
		aa = MSB of logical block
		bb = LSB of logical block
		xx xx ... = 13 bytes of ECC data
	*/
	if (buf[1] != 0xFF) return (buf[1] << 8 | buf[2]) % nand_info.nand_num_blocks;
	
	/* Spare area has format FF FF aa bb aa bb xx xx xx xx...
		aa = MSB of logical block
		bb = LSB of logical block
		xx xx ... = 10 bytes of ECC data?
	*/
	 if (buf[2] == 0xFF && buf[3] == 0xFF)
		return 0xFFFF; /* unprogrammed block */

	if ((buf[2] != buf[4]) || (buf[3] != buf[5])) {
		printf("Warning: Logical block number is inconsistent for physical block %x (%02x %02x /  %02x %02x)\n",
			physical_block, buf[2], buf[3], buf[4], buf[5]);
		/* continue anyway? */
	}
	
	return (buf[2] << 8 | buf[3])  % nand_info.nand_num_blocks;
}

void NAND_ReadTables(void) {
	int i;
	/* read mapTable */
	NAND_ReadPage(page_buf, 1);

	for (i=0; i < nandrsv_header.nrRsvBlks; i++) {
		memcpy(&mapTable[i], page_buf + i * 2, 2);
	}

/*	for (i=0; i < nandrsv_header.nrRsvBlks; i++) {
		printf("mapTable[%x]=%x spare=%04x\n", i, mapTable[i], NAND_GetLogicalBlkno(mapTable[i]));
	} */
	
	/* Having a RsvC section will probably displace the B and A sections, but I've never seen one.
	   If we have one but don't account for it, it will shift the maps of RsvB and RsvA over, so
	   just panic so someone will fix me. */
	
	if (nandrsv_header.nrRsvC != 0) {
		printf("ERROR: Don't know how to handle %x-block RsvC starting at %x\n", 
			nandrsv_header.nrRsvC, mapTable[0]);
		panic("NAND Init failure: unsupported configuration\n");
	}

	/* Consistency checks to avoid going past the end of mapTable[] */
	if (nandrsv_header.nrRsvB > nandrsv_header.nrRsvBlks)
		panic("NAND: nrRsvB > nrRsvBlks\n");
	
	if ((nandrsv_header.nrRsvB + nandrsv_header.nrRsvA) > nandrsv_header.nrRsvBlks)
		panic("NAND: nrRsvB + nrRsvA > nrRsvBlks\n");
	
	if ((nandrsv_header.fwStartBlk + nandrsv_header.nrFwBlks) > nandrsv_header.nrRsvBlks)
		panic("NAND: fwStartBlk + nrFwBlks > nrRsvBlks\n");

/* fill out tables from mapTable */

	for (i=0; i < nandrsv_header.nrRsvB; i++) {
		translate_rsvB[NAND_GetLogicalBlkno(mapTable[i])] = mapTable[i];
	}

	for (i=0; i < nandrsv_header.nrRsvA; i++) {
		translate_rsvA[NAND_GetLogicalBlkno(mapTable[i+nandrsv_header.nrRsvB])] = 
			mapTable[i+nandrsv_header.nrRsvB];
	}
	
	for (i=0; i < nandrsv_header.nrFwBlks; i++) {
		translate_fw[i] = mapTable[i+nandrsv_header.fwStartBlk];
	}
	
	/* We don't have to panic here, because nothing else depends on the location of this buffer */
	if (nandrsv_header.nrFwBlks2 != 0) {
		printf("WARNING: Don't know how to handle %x-block firmware2 starting at %x\n",
			nandrsv_header.nrFwBlks2, mapTable[nandrsv_header.fwStartBlk2]);
	}

/* display results */
	printf("Reserved area translation tables:\n");
	printf("RsvB: [");
	for (i=1; i < nandrsv_header.nrRsvB; i++) {
		if (translate_rsvB[i] == 0xFFFF) printf("x ");
		else
			printf("%04x%s", translate_rsvB[i], (nandrsv_header.nrRsvB - i)>1?" ":"");
	}
	printf("]\n");
	printf("RsvA: [");
	for (i=1; i < nandrsv_header.nrRsvA; i++) {
		if (translate_rsvA[i] == 0xFFFF) printf("x ");
		else
			printf("%04x%s", translate_rsvA[i], (nandrsv_header.nrRsvA - i)>1?" ":"");
	}
	printf("]\n");
	printf("Firmware: [");
	for (i=0; i < nandrsv_header.nrFwBlks; i++) {
		printf("%04x%s", translate_fw[i], (nandrsv_header.nrFwBlks - i)>1?" ":"");
	}
	printf("]\n");
}

/* Returns the page containing the blockmap */
int NAND_FillBlockmap(void) {
	int i, j;
	uint16_t min_block = 0xFFFF;
	int* buf = (int*)page_buf;
	/* skip first block */
	for(i = 1; i < nand_info.nand_num_blocks - nandrsv_header.nrRsvBlks; i++) {
		uint16_t logical_blkno = NAND_GetLogicalBlkno(i);
		if (logical_blkno < 0xFF00) {
			printf("logical blkno = %x\n", logical_blkno);
			translate_user[logical_blkno] = i;
			printf("translate_user[%x]=%x\n", logical_blkno, i);
			if (logical_blkno < min_block)
				min_block = logical_blkno;
		}
	}
	printf("First block is: %d, mapped to: %d\n", min_block, translate_user[min_block]);
	return 0;
}

#define NAND_RAWDEV  0
#define NAND_USERDEV 1
#define NAND_FWDEV   2
#define NAND_RSVADEV 3
#define NAND_RSVBDEV 4

static int nand_init(void) {
	char nand_id[5];
	int blockmap;
	int i = 0;
	
	/* Create NAND device as an alias of the registered device. */
	nand_dev[NAND_RAWDEV] = device_create(&nand_io, "nand", DF_BLK);
	if (nand_dev[NAND_RAWDEV] == DEVICE_NULL)
		return -1;

	nand_dev[NAND_USERDEV] = device_create(&nand_io, "nand_user", DF_BLK);
	if (nand_dev[NAND_USERDEV] == DEVICE_NULL)
		return -1;

	nand_dev[NAND_FWDEV] = device_create(&nand_io, "nand_fw", DF_BLK);
	if (nand_dev[NAND_FWDEV] == DEVICE_NULL)
		return -1;

	nand_dev[NAND_RSVADEV] = device_create(&nand_io, "nand_rsvA", DF_BLK);
	if (nand_dev[NAND_RSVADEV] == DEVICE_NULL)
		return -1;

	nand_dev[NAND_RSVBDEV] = device_create(&nand_io, "nand_rsvB", DF_BLK);
	if (nand_dev[NAND_RSVBDEV] == DEVICE_NULL)
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

/*	static uint16_t *translate_user = NULL, *mapTable = NULL, *translate_fw = NULL, 
		*translate_rsvA = NULL, *translate_rsvB = NULL; */

	printf("Allocating tables\n");
#define ALLOC_XLATE_BUF(name, size) \
			name = ((size)>PAGE_SIZE)?page_alloc(size):kmem_alloc(size); \
			if (name == NULL) { \
				printf("Couldn't malloc %d bytes for " #name "\n", (size)); \
				return -1; \
				} else printf("Allocated %d bytes @ %x for " #name "\n", (size), (uint32_t)name); \
			memset(name, 0xFF, (size));

	ALLOC_XLATE_BUF(mapTable, 2 * nandrsv_header.nrRsvBlks);
	ALLOC_XLATE_BUF(translate_user, 2 * nandrsv_header.nrUserBlks);
	ALLOC_XLATE_BUF(translate_fw, 2 * nandrsv_header.nrFwBlks);
	ALLOC_XLATE_BUF(translate_rsvA, 2 * nandrsv_header.nrRsvA);
	ALLOC_XLATE_BUF(translate_rsvB, 2 * nandrsv_header.nrRsvB);
	
	NAND_ReadTables();
	printf("Creating blockmap...\n");

	blockmap = NAND_FillBlockmap();
	/* NAND_ReadSector(translate, 2); */
	return 0;
}

/* todo -- support Rsv devices */
static int nand_read(device_t dev, char *buf, size_t *nbyte, int sector) {
	uint8_t* kbuf = kmem_map(buf, *nbyte);
	size_t bytes_remaining = *nbyte;
	int i = 0;
	int block, blkno, page, subpage;
	int nand_block;
	/* Temporary addition */
	/* blkno += 0x3E5000; This is AIMG */
	/* blkno += 0x30000;  Main FAT partition  */

	if(!kbuf)
		return EFAULT;

/*	printf("Reading %d bytes from page %d.\n", bytes_remaining, sector * 512 / nand_info.nand_bytes_per_page);
	nand_block = (sector * 512) / (nand_info.nand_pages_per_block * nand_info.nand_bytes_per_page); */

	printf("Reading %d bytes from page %x.\n", bytes_remaining, sector);
	nand_block = sector / nand_info.nand_pages_per_block;
	
	if(dev == nand_dev[0]) { /* Main device, no adjusting */
		block = nand_block;
		printf("Reading from NAND block %x page %x\n", nand_block, sector % nand_info.nand_pages_per_block);
		NAND_ReadPage(page_buf, sector);
		memcpy(kbuf, page_buf, bytes_remaining);
		return 0;
	}
	
	if(dev == nand_dev[1]) {	/* Firmware device, use blockmap */
		/* blkno += 7168; */
		block = translate_user[nand_block+1];
		printf("Reading from NAND block %d\n", nand_block);
		if(block == 0xFFFF){
			printf("No mapping found!\n");
			*nbyte = 0;
			return -1;
		}

		int max_bytes_to_read = nand_info.nand_pages_per_block * nand_info.nand_bytes_per_page;
		page = sector % nand_info.nand_pages_per_block;
		subpage = 0;

/*		page = ((sector * 512) % (nand_info.nand_pages_per_block * nand_info.nand_bytes_per_page))
			/ nand_info.nand_bytes_per_page; */
		subpage = (sector) % (nand_info.nand_bytes_per_page / 512);
		max_bytes_to_read -= page * nand_info.nand_bytes_per_page;
		max_bytes_to_read -= subpage * 512;
		
		
		printf("Reading %d bytes from block %d page %d subpage %d\n",
			bytes_remaining, block, page, subpage);

		if (max_bytes_to_read < bytes_remaining) {
			printf("Can't read %d bytes (max = %d)\n", bytes_remaining, max_bytes_to_read);
			return -1;
		}
		
		while (bytes_remaining > 0) {
			NAND_ReadPage(page_buf, block * nand_info.nand_pages_per_block + page);
			int bytes_left_in_page = nand_info.nand_bytes_per_page - subpage * 512;
			memcpy(kbuf, page_buf + subpage * 512, 
				bytes_remaining > bytes_left_in_page ? bytes_left_in_page : bytes_remaining);
			if (bytes_remaining <= bytes_left_in_page) break;
			bytes_remaining -= bytes_left_in_page;
			subpage = 0;
			page++;
		}
	}		

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

