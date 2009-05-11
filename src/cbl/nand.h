#ifndef _NAND_H
#define _NAND_H

#include "../spmp3050/spmp3050.h"

void NAND_Init();

void NAND_StrobeRead();
uint8_t NAND_ReadByte();

void NAND_WriteCmd(uint8_t cmd);
void NAND_WriteAddr(uint8_t addr);

int NAND_WaitReadBusy();
int NAND_WaitCmdBusy();

uint64_t NAND_ReadID();
#endif
