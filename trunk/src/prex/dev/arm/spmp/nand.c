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

#define offset_t uint32_t
#define PAGE_SIZE		2048
#define BLOCK_SIZE		(128 * PAGE_SIZE)
#define ECC_SIZE		64

static uint8_t sector_buf[PAGE_SIZE];
static uint16_t translate[0x1F20];

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

void NAND_Init(){
	NAND_ENABLE = 1;
	NAND_CTRL_HI |= 0x3;
	NAND_CTRL_HI &= ~(NAND_CTRL_CLE | NAND_CTRL_ALE);
	DEV_ENABLE |= 0x1;
	DEV_ENABLE_OUT |= 0x1;
	NAND_CTRL_HI &= ~0x1;
}

void NAND_StrobeRead(){
	volatile uint8_t* nand_base = ((volatile uint8_t*)NAND_BASE);
	uint8_t temp = NAND_CTRL_LO;
	NAND_CTRL_LO = temp | NAND_CTRL_RE;
	nand_base[0x25] = 0x1;
	NAND_CTRL_LO = temp;
}

uint8_t NAND_ReadByte()
{	
	return NAND_DATA;
}

void NAND_WriteCmd(uint8_t cmd){
	NAND_CTRL_HI = (NAND_CTRL_HI & ~NAND_CTRL_ALE) | NAND_CTRL_CLE;
	NAND_DATA = cmd;
	NAND_CTRL_HI = (NAND_CTRL_HI & ~NAND_CTRL_ALE) | NAND_CTRL_CLE;
	NAND_CTRL_HI &= ~NAND_CTRL_CLE;
}

void NAND_WriteAddr(uint32_t page_no, uint32_t col_no){
	NAND_CTRL_HI = (NAND_CTRL_HI & ~NAND_CTRL_CLE) | NAND_CTRL_ALE;
	NAND_DATA = col_no & 0xFF;
	NAND_DATA = (col_no >> 8) & 0x0F;
	NAND_DATA = (page_no & 0xFF);
	NAND_DATA = (page_no >> 8) & 0xFF;
	NAND_DATA = (page_no >> 16) & 0x0F;
	NAND_CTRL_HI = (NAND_CTRL_HI & ~NAND_CTRL_CLE) | NAND_CTRL_ALE;
	NAND_CTRL_HI &= ~NAND_CTRL_ALE;
}

/* Possibly broken, should check in disasm */
int NAND_WaitReadBusy(){
	int i = 0;
	for(; i < 3000; i++){
		if(!(NAND_STATUS & NAND_STATUS_READ_BUSY))
			return 0;
	}
	return -1;
}

int NAND_WaitCmdBusy(){
	int i = 0;
	for(; i < 50000; i++){
		if((NAND_STATUS2 & NAND_STATUS_CMD_READY))
			return 0;
	}
	return -1;	
}

/* Fills the buf with 5 chars worth of nand_id */
int NAND_ReadID(char* buf){
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

void NAND_ReadSector(void* buf, int sec_no){
	int i = 0;
	uint8_t* charbuf = (uint8_t*)buf;
	NAND_WriteCmd(0x00);
	NAND_WriteAddr(sec_no, 0);
	NAND_WriteCmd(0x30);
	NAND_WaitCmdBusy();
	NAND_WaitReadBusy();

	for(; i < PAGE_SIZE; i++){
		NAND_StrobeRead();
		/* NAND_WaitReadBusy(); */
		charbuf[i] = NAND_ReadByte();
	}
	NAND_Init();
}

void NAND_ReadSectorSpare(void * buf, int sec_no){
	int i = 0;
	uint8_t* charbuf = (uint8_t*)buf;
	NAND_WriteCmd(0x00);
	NAND_WriteAddr(sec_no, 0x800);
	NAND_WriteCmd(0x30);
	NAND_WaitCmdBusy();
	NAND_WaitReadBusy();

	for(; i < ECC_SIZE; i++){
		NAND_StrobeRead();
		charbuf[i] = NAND_ReadByte();
	}
	NAND_Init();
}

/* Returns the sector containing the blockmap */
int NAND_FillBlockmap(){
	int i = 0, j;
	uint16_t sec;
	int* buf = (int*)sector_buf;
	for(; i < 0x1F20; i++){
		for(j = 0; j < (BLOCK_SIZE / PAGE_SIZE); j++){
			NAND_ReadSectorSpare(sector_buf, i * (BLOCK_SIZE / PAGE_SIZE) + j);
			if(sector_buf[0x5] != 0xFF){
				sec = (((int)sector_buf[0x6]) << 8) | sector_buf[0x7];
				sec &= 0x3FF;
				translate[sec] = i;
				printf("buf: %08X %08X %08X %08X\n",buf[0],buf[1],buf[2],buf[3]);
				printf("Added translation from %04X to %04X\n", sec, i);
				break;
			}
		}
	}
	return 0;
}

static int nand_init(void){
	char nand_id[5];
	int ret = 0;
	int blockmap;
	/* Create NAND device as an alias of the registered device. */
	nand_dev[0] = device_create(&nand_io, "nand", DF_BLK);
	if (nand_dev[0] == DEVICE_NULL)
		return -1;

	nand_dev[1] = device_create(&nand_io, "nand1", DF_BLK);
	if (nand_dev[1] == DEVICE_NULL)
		return -1;
	
	NAND_Init();

	if(!NAND_ReadID(nand_id)){
		printf("NAND Chip found, id: %02X %02X %02X %02X %02X\n",
			nand_id[0],
			nand_id[1],
			nand_id[2],
			nand_id[3],
			nand_id[4]);
	}
	else
	{
		printf("No NAND chip found.\n");
		return -1;
	}
	printf("Creating blockmap...\n");
	blockmap = NAND_FillBlockmap();
	/* NAND_ReadSector(translate, 2); */
	return 0;
}

/* Horribly non-optimized, but it's late right now... */
static int nand_read(device_t dev, char *buf, size_t *nbyte, int blkno)
{
	uint8_t* kbuf = kmem_map(buf, *nbyte);
	static int cur_blk = 0;
	size_t todo = *nbyte;
	int i = 0;
	int sector, sub_sector, block;
	/* Temporary addition */
	/* blkno += 0x3E5000; This is AIMG */
	/* blkno += 0x30000;  Main FAT partition  */
	printf("Reading %d bytes from block %d.\n",todo, blkno);
	if(dev == nand_dev[0]){ /* Main device, no adjusting */

	}
	
	if(dev == nand_dev[1]){	/* Firmware device, use blockmap */
		/* blkno += 7168; */
		block = translate[(blkno * 512) / BLOCK_SIZE];
		blkno = (block * BLOCK_SIZE + (blkno * 512) % BLOCK_SIZE) / 512;
		printf("Reading from 0x%08X\n", blkno * 512);
	}
	

	if(!kbuf)
		return EFAULT;

	while(todo > 0){
		sector = blkno / 4;
		sub_sector = blkno % 4;
		NAND_ReadSector(sector_buf, sector);
		if(todo > PAGE_SIZE){
			memcpy(kbuf, sector_buf, PAGE_SIZE);
			todo -= PAGE_SIZE;
			blkno += 4;
			kbuf += PAGE_SIZE;
		}
		else
		{
			memcpy(kbuf, sector_buf + (512 * sub_sector), todo);
			todo = 0;
		}
		blkno++;
	}
	return 0;
}

static int nand_write(device_t dev, char *buf, size_t *nbyte, int blkno)
{
	return *nbyte;
}

static int nand_ioctl(device_t dev, u_long cmd, void *arg)
{
	switch(cmd){
	}
	printf("Whoops, wrong ioctl received!\n");
	return -1;
}

static int nand_open(device_t dev, int mode)
{
	printf("NAND opened.\n");
	return 0;
}

static int nand_close(device_t dev)
{
	return 0;
}

