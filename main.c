#include "main.h"
#include "eeprom.h"

void Timer0Init();
void Timer1Init();
void ADCInit();
void UartInit();
void PWMInit();
void IOInit();

void readEEPROM();
void saveDAC_set();

void hotLoop();
void sleepLoop();
void KeyTest();

/*
ADC_INDEX:
0:DAC
1:NTC
*/
uchar ADC_index = 0;

uint DAC_in = 0;
uint DAC_set;
uchar DAC_setL = 0;
uint DAC_tag = 0; //Real DAC target
#define ADDR_DAC_SETH 0x00
#define ADDR_DAC_MAPH 0x01
uchar DAC_addrL = 0;

uint sleepCount = 0;
uchar sleepFlash = 0;
#define SLEEP_END 9155  //600,000,000/65536=9155.2734
#define SLEEP_HALF 1831 //120,000,000/65536=1831.0546
#define SLEEP_SEC 15	//1,000,000/65536=15.2587
bit skeFlag;			//Flag of shake switch
bit skeFlagL;			//Last Flag

bit hotFlag;
uchar index = 0;
uchar key[3]; //SRP,SRN,SRW

void SRestTest()
{
	if ((PCON & 0x10) == 0)
	{
		PCON = PCON | 0x10;
		IAP_CONTR = 0x60;
	}
	else
	{
		PCON = PCON & 0xef;
	}
}

void main()
{
	//Off power out
	hotFlag = 0;
	Hot_out = 0;
	LED_r = 0;
	//Init
	SRestTest();
	Timer0Init();
	Timer1Init();
	IOInit();
	ADCInit();
	UartInit();
	PWMInit();
	NumberLED();
	readEEPROM();

	EA = 1;
	while (1)
	{
		numToLED(DAC_set);
		if (skeFlag)
			NumberLED_Num[3] |= 0x80;
		if (getU16L(DAC_set) != DAC_setL)
		{
			if (sleepCount > SLEEP_SEC)
			{
				EEPROM_enable();
				saveDAC_set();
				EEPROM_disable();
				DAC_setL = DAC_set;
			}
		}
	}
}

void saveDAC_set()
{
	//Save DAC_set
	EEPROM_writeByte(ADDR_DAC_MAPH, DAC_addrL, 0x5F);
	DAC_addrL += 2;
	if (DAC_addrL == 0)
	{
		EEPROM_clrWrtByte(ADDR_DAC_MAPH, 0, 0xFA);
	}
	EEPROM_writeByte(ADDR_DAC_SETH, DAC_addrL, getU16H(DAC_set));
	EEPROM_writeNextByte(getU16L(DAC_set));
	EEPROM_writeByte(ADDR_DAC_MAPH, DAC_addrL, 0xFA);
}

void readEEPROM()
{
	uchar buf;
	//Read eeprom:
	EEPROM_enable();
	// EEPROM_setAddr(ADDR_DAC_SETH, ADDR_DAC_SETL);
	// EEPROM_operate(EEPROM_CMD_READ);
	// getU16H(DAC_set) = IAP_DATA;
	// EEPROM_addAddr();
	// EEPROM_operate(EEPROM_CMD_READ);
	// getU16L(DAC_set) = IAP_DATA;
	EEPROM_readByte(ADDR_DAC_MAPH, 0, buf);
	TI = 0;
	while (1)
	{
		if (buf == 0x5A)
		{
			//Jump
			IAP_ADDRL += 2; //Add uint size
			//Check if its over
			if (IAP_ADDRL == 0)
			{
				EEPROM_clrWrtByte(ADDR_DAC_MAPH, 0, 0xFA);
				break;
			}
			EEPROM_operate(EEPROM_CMD_READ);
			buf = IAP_DATA;
		}
		else if (buf == 0xFA)
		{
			DAC_addrL = IAP_ADDRL;
			EEPROM_readByte(ADDR_DAC_SETH, DAC_addrL, getU16H(DAC_set));
			EEPROM_readNextByte(getU16L(DAC_set));
			DAC_setL = getU16L(DAC_set);
			break;
		}
		else
		{
			EEPROM_clrWrtByte(ADDR_DAC_MAPH, 0, 0xFA);
			break;
		}
	}

	//Check DAC_set
	if (DAC_set > 1024)
	{
		DAC_set = 0;
		saveDAC_set();
	}
	DAC_setL = DAC_set;
	EEPROM_disable();
}

/*
00:Normal
01:Input
10:Output
11:OpenDrain
*/
void IOInit()
{
	P1M0 |= (BIT3 | BIT4 | BIT5);
	P1M0 &= ~(BIT0);
	P1M1 |= BIT0;
	P1M1 &= ~(BIT3 | BIT4 | BIT5);
	P3M0 |= (BIT6);
	P3M1 &= ~(BIT6);
}

void Timer0Init(void) //200us12T@12.000MHz
{
	AUXR &= 0x7F;
	TMOD &= 0xF0;
	TMOD |= 0x02;
	TL0 = 0x88;
	TH0 = 0x88;
	TF0 = 0;
	TR0 = 1;
	ET0 = 1;
}
void Timer1Init(void) //65536us(//50ms)12T@12.000MHz
{
	AUXR &= 0xBF;
	TMOD &= 0x0F;
	TMOD |= 0x10;
	// TL1 = 0xB0;
	// TH1 = 0x3C;
	TF1 = 0;
	TR1 = 1;
}

void UartInit(void) //9600bps@12.000MHz
{
	PCON &= 0x7F;
	SCON = 0x50;
	AUXR |= 0x04;
	BRT = 0xD9;
	AUXR |= 0x01;
	AUXR |= 0x10;
	ES = 1;
}

void timerInterrupt() interrupt 1 using 1
{
	TF0 = 0;
	switch (ADC_index)
	{
	case 0:
		ADC_CONTR = 0xE8;
		break;
	case 1:
		ADC_CONTR = 0xE9;
		break;
	}

	hotLoop();
	NumberLED_display();
	KeyTest();
}

uchar Hot_count = 0;
void hotLoop()
{
	if (hotFlag)
	{
		if (Hot_count > 64)
		{
			Hot_count = 0;
			Hot_out = 0;
		}
		else if (Hot_count == 3)
			LED_r = Hot_out = !DAC_re;
		Hot_count++;
		sleepLoop();
	}
	else
	{
		LED_r = Hot_out = 0;
		if (TF1)
		{
			TF1 = 0;
			sleepCount++;
		}
	}
}

void sleepLoop() //Sleep
{
	if (TF1)
	{
		TF1 = 0;
		//50ms
		// TL1 = 0xB0;
		// TH1 = 0x3C;

		sleepCount++;
		if (sleepCount > SLEEP_END)
		{
			sleepCount = SLEEP_END;
			LED_g = 1;
			hotFlag = 0;
		}
		else if (sleepCount > SLEEP_HALF)
		{
			if (DAC_tag == DAC_set)
				DAC_tag = DAC_set / 2;
			sleepFlash++;
			if (sleepFlash > SLEEP_SEC)
			{
				sleepFlash = 0;
				LED_g = !LED_g;
			}
		}
		else
		{
			LED_g = 0;
			DAC_tag = DAC_set;
		}
	}
}

bit _sP = 0, _sN = 0, fP = 0, fN = 0, _sSW = 0, fSW = 0;
void KeyTest()
{
	key[0] <<= 1;
	key[0] |= IO_SRP;

	key[1] <<= 1;
	key[1] |= IO_SRN;

	key[2] <<= 1;
	key[2] |= IO_SW;

	_sP = !key[0];
	_sN = !key[1];
	_sSW = !key[2];

	if (_sP)
	{
		if (!fN)
			if (_sN)
			{
				DAC_set += 4;
				sleepCount = 0;
			}
		if (DAC_set > 1024)
			DAC_set = 1024;
	}

	if (_sN)
	{
		if (!fP)
			if (_sP)
			{
				sleepCount = 0;
				DAC_set -= 4;
			}
		if (DAC_set > 1024)
			DAC_set = 0;
	}

	//sw
	if (!_sSW)
	{
		if (fSW)
		{
			LED_g = hotFlag;
			hotFlag = !hotFlag;
			sleepCount = 0;
		}
	}

	fSW = _sSW;
	fP = _sP;
	fN = _sN;
}

void PWMInit()
{
	CMOD = BIT3;		  //PCA speed : SYSclk(System Clock)
	CCAPM0 = BIT6 | BIT1; //8bit PWM without interrupt
	CR = 1;				  //start
}

void ADCInit()
{
	//ADC
	ADC_CONTR = 0xE0;
	ADC_RES = 0;
	P1ASF = BIT0;
	AUXR1 &= 0xFB;
	EADC = 1;
}

uchar pwm = 0;
uint dac_avg = 0;
uchar dac_count = 0;
void ADCinterrupt() interrupt 5
{
	if (ADC_index == 0)
	{
		dac_avg += (ADC_RES << 2) + ADC_RESL;
		if (dac_count > 7)
		{
			DAC_in = dac_avg >> 3; //8
			dac_count = 0;
			dac_avg = 0;
			if (DAC_in > DAC_tag)
				//if(DAC_in - DAC_set > 4)
				if (pwm < 0xFF)
					pwm++;
			if (DAC_in < DAC_tag)
				//if(DAC_set - DAC_in > 4)
				if (pwm)
					pwm--;
			CCAP0H = pwm;
		}
		dac_count++;
		ADC_index++;
	}
	else
	{
		skeFlag = ADC_RES > 250;
		if (skeFlag != skeFlagL)
			sleepCount = 0;

		skeFlagL = skeFlag;
		ADC_index = 0;
	}
	ADC_CONTR = 0xE0;
}

void UartInterrupt() interrupt 4
{
	static uchar reCount = 10;
	uchar buf = 0;
	if (RI)
	{
		RI = 0;
		buf = SBUF;
		//Auto Download: Read 10 0xF8
		//STC-ISP version V6.85R
		if (buf == 0xF8)
		{
			if (!reCount)
			{
				SRestTest();
			}
			reCount--;
		}
		else
			reCount = 10;

		DAC_set = buf * 4;
		sleepCount = 0;
		SBUF = DAC_in >> 2;
	}
	// TI = 0;
}