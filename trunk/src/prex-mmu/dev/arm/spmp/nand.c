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
// #define EPIC_SPEW 1

static int memcmp(uint32_t *buf1, uint32_t *buf2, uint32_t len) {
	uint32_t i;
	for(i=0; i< len/4; i++)
		if (buf1[i] != buf2[i]) return buf1[i] - buf2[i];
	return 0;
}
static void printhex(uint8_t *buf, size_t n, uint32_t addr, int linew) {
	size_t pos,i, l, j;
        int is_same = 0;
        for (pos=0;pos<n;pos+=linew,addr+=linew){
                l = pos+linew>n?n:pos+linew;
              if (pos >= linew) {
                        if (!memcmp(buf+pos, buf+pos-linew, linew)) {
                                if (!is_same) printf("*\n");
                                is_same = 1;
                                continue;
                        }
                }
                is_same=0;
                printf("%08X: ", addr);
                for (i=pos,j=0;i<pos+linew;i++,j++){
                        if (i<n){
                                printf("%02X", buf[i]);
                        }
                      else
                        {
                                printf("   ");
                        }
                        if(!((j+1)%4))
                                printf(" ");
                }
                if (linew%4)
                        printf(" ");
                for (i=pos;i<l;i++){
                        if(buf[i]>=0x20&&buf[i]<=0x7F)
                                printf("%c", buf[i]);
                        else
                                printf("%c", '.');
                }
                printf("\n");
        }
}

static uint8_t page_buf[MAX_PAGE_SIZE];
static uint16_t translate_user[8192];
static uint16_t mapTable[0x100], translate_fw[0x100],
	translate_rsvA[0x100], translate_rsvB[0x100];
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

void NAND_ReadPage(void* buf, int page_no, int offset, int len) {
	int i = 0;
	uint8_t* charbuf = (uint8_t*)buf;
	NAND_WriteCmd(0x00);
	NAND_WriteAddr(page_no, offset);
	NAND_WriteCmd(0x30);
	NAND_WaitCmdBusy();
	NAND_WaitReadBusy();

	for(; i < len; i++) {
		NAND_StrobeRead();
		/* NAND_WaitReadBusy(); */
		charbuf[i] = NAND_ReadByte();
	}
	NAND_Init();
#ifdef EPIC_SPEW
	printhex(buf, len, page_no, 16);
#endif
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
	
#ifdef EPIC_SPEW
//	printf("spare: ");
//	printhex(buf, 128, page_no, 16);
#endif
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
	NAND_ReadPage(page_buf, 0, 0, 512);
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
	uint8_t buf[128];
	int page=0;
	uint16_t offset, retval;
	
	if (physical_block > nandrsv_header.nrUserBlks) {
		offset = 0; // no offset for Rsv blocks
	} else {
		offset = (0x3e8 * (physical_block / 1024)) & 0xFFFF;
	}
	
	NAND_ReadPageSpare(buf, physical_block * nand_info.nand_pages_per_block + page);
//	printf("block %x page %x buf=[%02x %02x %02x %02x] offset=%x\n", physical_block, page, buf[0], buf[1], buf[2], buf[3], offset);
	if (buf[0] != 0xFF) return 0xFFFE; /* bad block ? */
	
	/* Spare area has format FF aa bb xx xx xx xx xx xx xx... 
		aa = MSB of logical block
		bb = LSB of logical block
		xx xx ... = 13 bytes of ECC data
	*/
	if (buf[1] != 0xFF) {
		retval = (buf[1] & 0xF) << 8;
		retval |= buf[2];
		retval >>= 1;
		retval += offset;
		return retval;
	}
	
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

	retval = (buf[2] * 0xF) << 8;
	retval |= buf[3];
	retval >>= 1;
	retval += offset;
	return retval;
}

void NAND_ReadTables(void) {
	int i, blkno;

	/* read mapTable */
	NAND_ReadPage(page_buf, 1, 0, 512);

	for (i=0; i < nandrsv_header.nrRsvBlks; i++) {
		memcpy(&mapTable[i], page_buf + i * 2, 2);
	}

	for (i=0; i < nandrsv_header.nrRsvBlks; i++) {
		printf("mapTable[%x]=%x spare=%04x\n", i, mapTable[i], NAND_GetLogicalBlkno(mapTable[i]));
	}
	
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
		blkno = NAND_GetLogicalBlkno(mapTable[i]);
		if (blkno > 0xFF00) continue;
		translate_rsvB[blkno] = mapTable[i];
	}

	for (i=0; i < nandrsv_header.nrRsvA; i++) {
		blkno = NAND_GetLogicalBlkno(mapTable[i+nandrsv_header.nrRsvB]);
		if (blkno > 0x60) continue;
		translate_rsvA[blkno] = mapTable[i+nandrsv_header.nrRsvB];
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
	for (i=0; i <= nandrsv_header.nrRsvB; i++) {
		if (translate_rsvB[i] == 0xFFFF) printf("x ");
		else
			printf("%04x%s", translate_rsvB[i], (nandrsv_header.nrRsvB - i)>0?" ":"");
	}
	printf("]\n");
	
	printf("RsvA: [");
	for (i=0; i <= nandrsv_header.nrRsvA; i++) {
		if (translate_rsvA[i] == 0xFFFF) printf("x ");
		else
			printf("%04x%s", translate_rsvA[i], (nandrsv_header.nrRsvA - i)>0?" ":"");
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
//		NAND_ReadHead(page_buf, i * nand_info.nand_pages_per_block);
		uint16_t logical_blkno = NAND_GetLogicalBlkno(i);
/*		if (buf[0] == 0x4941484f) {
				printf("USBMS block %x @ physical block %x, logical block %x\n",
					buf[1] >> 8, i, logical_blkno);
				NAND_ReadPage(page_buf, i * nand_info.nand_pages_per_block, 0, 4096+128);
				printhex(page_buf, 16, 0, 16);
//				NAND_ReadPage(page_buf, i * nand_info.nand_pages_per_block+1, 0, 4096+128);
//				printhex(page_buf, 4096+128, 4096, 16);
			} */
		if (logical_blkno < 0xFF00) {
/*			printf("logical blkno = %x\n", logical_blkno); */
			translate_user[logical_blkno] = i;
/*			printf("translate_user[%x]=%x\n", logical_blkno, i); */
			if (logical_blkno < min_block)
				min_block = logical_blkno;
		}
	}
	printf("First block is: %d, mapped to: %d\n", min_block, translate_user[min_block]);
/* fixme hardcoded 200 */
/*	printf("User: [");
	for (i=0; i < 200; i++) {
		printf("%x%s", translate_user[i], (200 - i)>1?" ":"");
	}
	printf("]\n"); */
	
	printf("User: [");
	for (i=0; i < 400; i++) {
		printf("%x%s", translate_user[i], (400 - i)>1?" ":"");
	}
	printf("]\n");
	
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

	NAND_ReadTables();
	printf("Creating blockmap...\n");

	blockmap = NAND_FillBlockmap();
	/* NAND_ReadSector(translate, 2); */
	return 0;
}

// this function can only read 512 bytes at a time. 
static int nand_read_sector_internal(device_t dev, char *kbuf, int sector)
{
	int i = 0, subpage;
	int logical_block, physical_block;
	int logical_page, physical_page;
	int sectors_per_page = nand_info.nand_bytes_per_page / 512;
	
	printf("nand_read_sector_internal(%x, %x)\n", (uint32_t)kbuf, sector);

	if(!kbuf)
		return EFAULT;

	logical_page = sector / sectors_per_page;
	logical_block = logical_page / nand_info.nand_pages_per_block;
	subpage = sector % sectors_per_page;

	printf("Reading %d bytes from sector %x (logical block/page/subpage %x/%x/%x).\n", 
		512, sector, logical_block, logical_page, subpage);
	
	if(dev == nand_dev[NAND_RAWDEV]) { /* Main device, no adjusting */
		physical_block = logical_block;
		physical_page = logical_page % nand_info.nand_pages_per_block
			+ physical_block * nand_info.nand_pages_per_block;
		printf("Reading from physical NAND %x/%x/%x\n", physical_block, physical_page, subpage);
		NAND_ReadPage(page_buf, physical_page, subpage * 512, 512);
		memcpy(kbuf, page_buf, 512);
		return 0;
	}
	
	static uint16_t *translate_table = NULL;
	
	if (dev == nand_dev[NAND_USERDEV]) translate_table = translate_user;
	if (dev == nand_dev[NAND_FWDEV])   translate_table = translate_fw;
	if (dev == nand_dev[NAND_RSVADEV]) translate_table = translate_rsvA;
	if (dev == nand_dev[NAND_RSVBDEV]) translate_table = translate_rsvB;
	
	if (translate_table == NULL) {
		printf("unknown nand device\n");
		return -1;
	}


	physical_block = translate_table[logical_block];
    printf("logical %x translates to physical %x\n", logical_block, physical_block );

	physical_page = logical_page % nand_info.nand_pages_per_block
		+ physical_block * nand_info.nand_pages_per_block;

    /* with physical_page +0x100 i'm at
       nand_user at offset 00000000 maps to /dev/sde at offset 5dd8 0000 - 5dd80200 : 0dfa70913ca785c64b4e092e8c80dfe683029685
       without i'm at:
       nand_user at offset 00000000 maps to /dev/sde at offset 5dc8 0000 - 5dc80200 : 127162cc57b62d319023752fa21a0d21717dfb3d

       we want to be at 0, so rationnaly we need to do:
       - 0x5dc80

       resulting in:
       nand_user at offset 00000000 maps to /dev/sde at offset 16d00000 - 16d00200 : 8f11e39813065b01d3c7100854b6eb5a332fa2fe


       modifying physical_block results in:
       - 0x1
       nand_user at offset 00000000 maps to /dev/sde at offset 5dc00000 - 5dc00200 : c435fb619bd3752f93af78d591eea55caa73fb42
    */

    /*     physical_page -= 0x5dc80; */

    //    physical_page += 0x1;

	printf("Reading from physical NAND %x/%x/%x\n", physical_block, physical_page, subpage);

	if (physical_block == 0xFFFF){
      printf("No mapping found!\n");
      return -1;
	}
    
/*	int max_bytes_to_read = nand_info.nand_bytes_per_page - subpage * 512;

	if (max_bytes_to_read < bytes_remaining) {
		printf("Can't read %d bytes (max = %d)\n", bytes_remaining, max_bytes_to_read);
		return -1;
	} */
	
	NAND_ReadPage(kbuf, physical_page, subpage * 512, 512);
/*	memcpy(kbuf, page_buf + subpage * 512, 512); */
	return 0;

}


/* todo -- support Rsv devices */
static int nand_read(device_t dev, char *buf, size_t *nbyte, int sector) 
{
  uint8_t* kbuf = kmem_map(buf, *nbyte);
  size_t bytes_remaining = *nbyte;

  printf("nand_read(%x, %x, %x)\n", (uint32_t)kbuf, (uint32_t)*nbyte, sector);

  while( bytes_remaining > 0 )
    {
      int r = nand_read_sector_internal( dev, kbuf, sector );
      if (r==-1)
        {
          *nbyte = 0;
          return -1;
        }

      kbuf += 512;
      bytes_remaining -= 512;
      sector++;
    }
  return 0;
}

static int nand_write(device_t dev, char *buf, size_t *nbyte, int blkno) {
	return *nbyte;
}

static int nand_ioctl(device_t dev, u_long cmd, void *arg) {
	switch(cmd) {
		case NANDIOC_GET_INFO:
          printf("nand_get_info called\n");
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

