#ifndef __NUMBERLED_H_
#define __NUMBERLED_H_

#include "main.h"
#include "typedefine.h"

#define NumberLED_IO_H P2
#define NumberLED_IO_L P0
#define PXM1 P2M1
#define PXM0 P2M0

extern uchar NumberLED_Num[4];
void NumberLED();
void NumberLED_display();
void NumberLED_setNum(uchar index, uchar num, bit dot);
void numToLED(uint num);

#endif