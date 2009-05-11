#include "../spmp3050/spmp3050.h"

void NAND_Init(){
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

uint8_t NAND_ReadByte(){
	
	return NAND_DATA;
}

void NAND_WriteCmd(uint8_t cmd){
	NAND_CTRL_HI = (NAND_CTRL_HI & ~NAND_CTRL_ALE) | NAND_CTRL_CLE;
	NAND_DATA = cmd;
	NAND_CTRL_HI = (NAND_CTRL_HI & ~NAND_CTRL_ALE) | NAND_CTRL_CLE;
	NAND_CTRL_HI &= ~NAND_CTRL_CLE;
}

void NAND_WriteAddr(uint8_t addr){
	NAND_CTRL_HI = (NAND_CTRL_HI & ~NAND_CTRL_CLE) | NAND_CTRL_ALE;
	NAND_DATA = addr;
	NAND_CTRL_HI = (NAND_CTRL_HI & ~NAND_CTRL_CLE) | NAND_CTRL_ALE;
	NAND_CTRL_HI &= ~NAND_CTRL_ALE;
}

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
		if(!(NAND_STATUS2 & NAND_STATUS_CMD_BUSY))
			return 0;
	}
	return -1;	
}

uint64_t NAND_ReadID(){
	uint64_t nand_id = 0;
	NAND_WriteCmd(0x90);
	NAND_WriteAddr(0);
	NAND_WaitCmdBusy();
	NAND_WaitReadBusy();
	
	NAND_StrobeRead();
	NAND_WaitReadBusy();
	nand_id = NAND_ReadByte();
	nand_id = nand_id << 8;
	
	NAND_StrobeRead();
	NAND_WaitReadBusy();
	nand_id |= NAND_ReadByte();
	nand_id = nand_id << 8;
	
	NAND_StrobeRead();
	NAND_WaitReadBusy();
	nand_id |= NAND_ReadByte();
	nand_id = nand_id << 8;
	
	NAND_StrobeRead();
	NAND_WaitReadBusy();
	nand_id |= NAND_ReadByte();
	nand_id = nand_id << 8;
	
	NAND_StrobeRead();
	NAND_WaitReadBusy();
	nand_id |= NAND_ReadByte();
	
	return nand_id;
}