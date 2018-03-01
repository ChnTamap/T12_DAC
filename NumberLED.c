#include "typedefine.h"
#include "NumberLED.h"

unsigned char code NumberLED_List[16] = {
	63, 6, 91, 79, 102, 109, 125, 7, 127, 111, 119, 124, 57, 94, 121, 113};

void NumberLED();
void NumberLED_display();
void NumberLED_setNum(uchar index, uchar num, bit dot);

uchar NumberLED_Num[4] = {2, 0, 0, 0};
uchar NumberLED_p0p = 0;

void NumberLED()
{
	PXM1 = 0;
	PXM0 = 0xFF;
	NumberLED_IO_L = 0xFF;
}

void NumberLED_display()
{
	static uchar loo = 0x01;
	NumberLED_IO_H = 0;
	NumberLED_IO_L = ~loo;
	NumberLED_IO_H = NumberLED_Num[NumberLED_p0p];

	NumberLED_p0p++;
	loo <<= 1;
	if (NumberLED_p0p >= 4)
	{
		NumberLED_p0p = 0;
		loo = 0x01;
	}
}

void NumberLED_setNum(uchar index, uchar num, bit dot)
{
	NumberLED_Num[index] = NumberLED_List[num];
	if (dot)
		NumberLED_Num[index] |= 0x80;
}

void numToLED(uint num)
{
	uint nBit;
	nBit = num % 10;
	NumberLED_setNum(3, nBit, 0);
	num -= nBit;
	nBit = num % 100;
	NumberLED_setNum(2, nBit / 10, 0);
	num -= nBit;
	nBit = num % 1000;
	NumberLED_setNum(1, nBit / 100, 0);
	num -= nBit;
	nBit = num % 10000;
	NumberLED_setNum(0, nBit / 1000, 0);
}