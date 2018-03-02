#ifndef __EEPROM_H_
#define __EEPROM_H_

#include "STC12.h"
#include "typedefine.h"
#include "hex.h"

#define EEPROM_CMD_NULL 0x00
#define EEPROM_CMD_READ 0x01
#define EEPROM_CMD_WRITE 0x02
#define EEPROM_CMD_CLEAR 0x03
#define EEPROM_CONTR_TIME 3 //12Mhz
#define EEPROM_CONTR_ENABLE (0x80 + EEPROM_CONTR_TIME)
#define EEPROM_CONTR_DISABLE 0x00

#define EEPROM_enable() IAP_CONTR = EEPROM_CONTR_ENABLE;
#define EEPROM_disable() IAP_CONTR = EEPROM_CONTR_DISABLE;
#define EEPROM_operate(cmd) \
	IAP_CMD = cmd;          \
	IAP_TRIG = 0x5A;        \
	IAP_TRIG = 0xA5;

#define EEPROM_setAddr(addrH, addrL) \
	IAP_ADDRH = addrH;               \
	IAP_ADDRL = addrL;
#define EEPROM_addAddr() IAP_ADDRL++;

#define EEPROM_readByte(addrH, addrL, dat) \
	EEPROM_setAddr(addrH, addrL);          \
	EEPROM_operate(EEPROM_CMD_READ);       \
	dat = IAP_DATA;
#define EEPROM_readNextByte(dat)     \
	EEPROM_addAddr();                \
	EEPROM_operate(EEPROM_CMD_READ); \
	dat = IAP_DATA;
//Clear and write
#define EEPROM_clrWrtByte(addrH, addrL, dat) \
	EEPROM_setAddr(addrH, addrL);            \
	EEPROM_operate(EEPROM_CMD_CLEAR);        \
	IAP_DATA = dat;                          \
	EEPROM_operate(EEPROM_CMD_WRITE);
#define EEPROM_writeByte(addrH, addrL, dat) \
	IAP_DATA = dat;                         \
	EEPROM_setAddr(addrH, addrL);           \
	EEPROM_operate(EEPROM_CMD_WRITE);
#define EEPROM_writeNextByte(dat) \
	IAP_DATA = dat;               \
	EEPROM_addAddr();             \
	EEPROM_operate(EEPROM_CMD_WRITE);

#endif