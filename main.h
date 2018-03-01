#ifndef __MAIN_H_
#define __MAIN_H_

#include "STC12.h"
#include "typedefine.h"
#include "NumberLED.h"
#include "hex.h"
/* 
Elc Borad Pin:
+	+	+	+	+	O	+	+	+
VCC	OP	PWM	ADC	GND		OUT	NC	NC
VCC	P35	P13	P10	GND		P36
 */

sbit DAC_re = P3 ^ 5;  //OP input
sbit PWM_out = P1 ^ 3; //PWM output
sbit IO_SRP = P3 ^ 2;  //SR+pos input
sbit IO_SRN = P3 ^ 3;  //SR-neg input
sbit IO_SW = P3 ^ 4;   //Swich input

sbit Hot_out = P3 ^ 6; //MOSFET output
sbit LED_r = P1 ^ 4;   //LED RED
sbit LED_g = P1 ^ 5;   //LED GREEN

#endif