#ifndef __TYPEDEF_H_
#define __TYPEDEF_H_

typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long ulong;

#define getU8(value, index) *((uchar *)(&value) + index)
#define U16_H 0
#define U16_L 1
#define getU16H(value) getU8(value,U16_H)
#define getU16L(value) getU8(value,U16_L)

#endif