
#include "N76E003.h"
#include "SFR_Macro.h"
#include "Function_define.h"
#include "Common.h"
#include "Delay.h"

#include "display.h"

sbit HC595_SER = P0^0;
sbit HC595_SCLK = P1^0;
sbit HC595_RCLK = P1^1;
sbit HC595_SCLR = P1^2;
sbit ICN2012_A0 = P0^5;
sbit ICN2012_A1 = P0^6;
sbit ICN2012_A2 = P0^7;
sbit ICN2012_E1 = P1^5;
sbit BEEPER = P0^3;

uint8_t timeCnt = 0;
uint8_t workState;
uint16_t secSet;
extern uint16_t secCnt;
extern uint8_t workMode;
extern uint8_t keyLevel;
extern uint8_t lightLevel;

extern xdata int Temperature;
extern xdata uint8_t RelativeHumidity;
extern xdata uint8_t time[8];

bit timerON = 0;
bit time_sFlag = 1;
bit T_RH_delayFlag = 1;

void display_Init()
{
	HC595_SCLR = 1;
	ICN2012_A0 = 0;
	ICN2012_A1 = 0;
	ICN2012_A2 = 0;
	ICN2012_E1 = 0;
	BEEPER = 0;	
	
	TIMER1_MODE2_ENABLE;
	TH1 = 56;
    TL1 = 56;  
	set_ET1;                                    //enable Timer1 interrupt
	set_EA;
	set_TR1;
}


unsigned char code nAsciiDot[] =              // ASCII
{
	0x00,0x70,0x88,0x98,0xA8,0xC8,0x88,0x70, // -0-
	0x00,0x20,0x60,0x20,0x20,0x20,0x20,0x70, // -1-
	0x00,0x70,0x88,0x08,0x30,0x40,0x80,0xF8, // -2-
	0x00,0xF8,0x08,0x10,0x30,0x08,0x88,0x70, // -3-
	0x00,0x10,0x30,0x50,0x90,0xF8,0x10,0x10, // -4-
	0x00,0xF8,0x80,0xF0,0x08,0x08,0x88,0x70, // -5-
	0x00,0x38,0x40,0x80,0xF0,0x88,0x88,0x70, // -6-
	0x00,0xF8,0x08,0x10,0x20,0x40,0x40,0x40, // -7-
	0x00,0x70,0x88,0x88,0x70,0x88,0x88,0x70, // -8-
	0x00,0x70,0x88,0x88,0x78,0x08,0x10,0xE0, // -9-
	0x00,0x00,0x40,0x40,0x00,0x40,0x40,0x00, // -:-
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // - -
	0x00,0x00,0x00,0x00,0xE0,0x00,0x00,0x00, // ---
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80, // -.-
	0x00,0x03,0x01,0x00,0x00,0x00,0x00,0x00, // -`-
	0x00,0xC0,0xC8,0x10,0x20,0x40,0x98,0x18, // -%-
	0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x80, // -.-
	0x00,0x00,0x00,0x00,0x00,0x80,0x80,0x80, // -.-
	0x00,0x00,0x00,0x00,0x80,0x80,0x80,0x80, // -.-
};

static void delay(uint8_t cnt)
{
	uint8_t i;
	for(i=0;i<cnt;i++)
	{
		nop;nop;nop;nop;nop;nop;nop;
		nop;nop;nop;nop;nop;nop;nop;
		nop;nop;nop;nop;nop;nop;nop;
		nop;nop;nop;nop;nop;nop;nop;
	}
}

/* 按秒表方式显示 0000 */
uint32_t dataCombine_Timer1(uint16_t secCnt,uint8_t colLine)
{
	uint16_t i,number;
	uint32_t result=0;
	number = (secCnt/1000)%10;
	i = number*8+colLine;
	result = (result|nAsciiDot[i])<<6;
	number = (secCnt/100)%10;
	i = number*8+colLine;
	result = (result|nAsciiDot[i])<<6;
	number = (secCnt/10)%10;
	i = number*8+colLine;
	result = (result|nAsciiDot[i])<<6;		
	result = (result|nAsciiDot[0x68+colLine])<<1;
	number = secCnt%10;
	i = number*8+colLine;
	result = (result|nAsciiDot[i])>>3;
	return result;
}

/* 按计时方式显示 00:00 */
uint32_t dataCombine_Timer2(uint16_t secCnt,uint8_t colLine)
{
	uint16_t i,number;
	uint32_t result=0;
	number = (secCnt/6000)%10;
	i = number*8+colLine;
	result = (result|nAsciiDot[i])<<5;
	number = (secCnt/600)%10;
	i = number*8+colLine;
	result = (result|nAsciiDot[i])<<5;
	result = (result|nAsciiDot[0x50+colLine])<<3;
	number = (secCnt%600)/100;
	i = number*8+colLine;
	result = (result|nAsciiDot[i])<<5;			
	number = ((secCnt%600)/10)%10;
	i = number*8+colLine;
	result = (result|nAsciiDot[i])>>3;
	return result;
}

void display_TimerUpMode(uint8_t timerUpMode)
{
	uint8_t i,j,level,mode;
	uint32_t rowData;
	
	if(timerON==0)
	{	
		timerON = 1;
		secCnt = 0;
	}
	Enable_ADC_AIN0;
	clr_ADCF;
	set_ADCS;									// ADC start trig signal
    while(ADCF == 0);
	level = ADCRH;
	if(level<0x10)
		mode = 0x01;
	else if(level<0x50)
	{
		timerON = 0;
		workMode = TimeMode;
		return;
	}
	else if(level<0x90)
		mode = 0x02;

	if(secSet!=0)
	{
		if(secCnt>=secSet)
			BEEPER = 1;
		if(secCnt>secSet+50)
			BEEPER = 0;
	}
	
	switch(mode)
	{
		case 0x01:
			secSet += 10;
			for(j=0;j<10;j++)
			{
				for(i=0;i<8;i++)
				{
					if(timerUpMode==TimerUpMode1)
						rowData = dataCombine_Timer1(secSet,i);
					else if(timerUpMode==TimerUpMode2)
						rowData = dataCombine_Timer2(secSet,i);
					write_ROW(rowData);
					ICN2012_E1 = 1;	
					delay(lightLevel);
					ICN2012_E1 = 0;	
					delay(0xFF-lightLevel);		
					P0 += 0x20;	
				}
			}
			mode = 0x03;
		break;
		case 0x02:
			if(secSet>00)
				secSet -= 10;
			for(j=0;j<10;j++)
			{
				for(i=0;i<8;i++)
				{
					if(timerUpMode==TimerUpMode1)
						rowData = dataCombine_Timer1(secSet,i);
					else if(timerUpMode==TimerUpMode2)
						rowData = dataCombine_Timer2(secSet,i);
					write_ROW(rowData);
					ICN2012_E1 = 1;	
					delay(lightLevel);
					ICN2012_E1 = 0;	
					delay(0xFF-lightLevel);		
					P0 += 0x20;	
				}
			}
			mode = 0x03;
		break;
		case 0x03:
			for(j=0;j<150;j++)
			{
				for(i=0;i<8;i++)
				{
					if(timerUpMode==TimerUpMode1)
						rowData = dataCombine_Timer1(secSet,i);
					else if(timerUpMode==TimerUpMode2)
						rowData = dataCombine_Timer2(secSet,i);
					write_ROW(rowData);
					ICN2012_E1 = 1;	
					delay(lightLevel);
					ICN2012_E1 = 0;	
					delay(0xFF-lightLevel);		
					P0 += 0x20;	
				}
			}
			timerON = 0;
			mode = 0x05;
		break;
		default:
			for(i=0;i<8;i++)
			{
				if(timerUpMode==TimerUpMode1)
					rowData = dataCombine_Timer1(secCnt,i);
				else if(timerUpMode==TimerUpMode2)
					rowData = dataCombine_Timer2(secCnt,i);
				write_ROW(rowData);
				ICN2012_E1 = 1;	
				delay(lightLevel);
				ICN2012_E1 = 0;	
				delay(0xFF-lightLevel);		
				P0 += 0x20;	
			}
		break;
	}
}


uint32_t dataCombine_T(uint8_t colLine)
{
	uint16_t i,number;
	uint32_t result=0;
	if(Temperature<0)
	{
		result = (result|nAsciiDot[0x60+colLine])<<4;	
		Temperature = -Temperature;
	}
	number = Temperature/100;
	i = number*8+colLine;
	result = (result|nAsciiDot[i])<<5;
	number = (Temperature/10)%10;
	i = number*8+colLine;
	result = (result|nAsciiDot[i])<<5;
	result = (result|nAsciiDot[0x68+colLine])<<1;
	number = Temperature%10;
	i = number*8+colLine;
	result = (result|nAsciiDot[i]);
	result = (result|nAsciiDot[0x70+colLine]);
	return result;
}

uint32_t dataCombine_RH(uint8_t colLine)
{
	uint16_t i,number;
	uint32_t result=0;
	number = RelativeHumidity/10;
	i = number*8+colLine;
	result = (result|nAsciiDot[i])<<5;
	number = RelativeHumidity%10;
	i = number*8+colLine;
	result = (result|nAsciiDot[i])<<6;
	result = (result|nAsciiDot[0x78+colLine]);
	return result;
}

void display_T_RH(void)
{
	uint8_t i,j;
	uint32_t rowData;
	for(j=0;j<0xCF;j++)
	{	
		lightLevel = j+0x20;
		for(i=0;i<8;i++)
		{
			rowData = dataCombine_T(i);
			write_ROW(rowData);
			ICN2012_E1 = 1;	
			delay(lightLevel);
			ICN2012_E1 = 0;	
			delay(0xFF-lightLevel);		
			P0 += 0x20;	
		}			
	}
	for(j=0;j<0xCF;j++)
	{		
		lightLevel = j+0x20;
		for(i=0;i<8;i++)
		{
			rowData = dataCombine_RH(i);
			write_ROW(rowData);
			ICN2012_E1 = 1;	
			delay(lightLevel);
			ICN2012_E1 = 0;	
			delay(0xFF-lightLevel);		
			P0 += 0x20;	
		}		
	}
	workMode = TimeMode;
}

uint32_t dataCombine_Date(uint8_t colLine)
{
	uint16_t i,number;
	uint32_t result=0;
	number = time[4]>>4;
	i = number*8+colLine;
	result = (result|nAsciiDot[i])<<5;
	number = time[4]&0x0F;
	i = number*8+colLine;
	result = (result|nAsciiDot[i])<<5;
	result = (result|nAsciiDot[0x60+colLine])<<3;
	number = time[3]>>4;
	i = number*8+colLine;
	result = (result|nAsciiDot[i])<<5;
	number = time[3]&0x0F;
	i = number*8+colLine;
	result = (result|nAsciiDot[i])>>3;
	return result;
}

void display_Date(void)
{
	uint8_t i;
	uint32_t rowData;
	for(timeCnt=0;timeCnt<3;)
	{
		for(i=0;i<8;i++)
		{
			rowData = dataCombine_Date(i);
			write_ROW(rowData);
			ICN2012_E1 = 1;	
			delay(lightLevel);
			ICN2012_E1 = 0;	
			delay(0xFF-lightLevel);		
			P0 += 0x20;	
		}
	}
	workMode = TimeMode;	
}

uint32_t dataCombine_Time(uint8_t colLine)
{
	uint16_t i,number;
	uint32_t result=0;
	number = time[2]>>4;
	i = number*8+colLine;
	result = (result|nAsciiDot[i])<<5;
	number = time[2]&0x0F;
	i = number*8+colLine;
	result = (result|nAsciiDot[i])<<8;
	number = time[1]>>4;
	i = number*8+colLine;
	result = (result|nAsciiDot[i])<<5;
	number = time[1]&0x0F;
	i = number*8+colLine;
	result = (result|nAsciiDot[i])>>3;	
	if(time_sFlag)
		result = result|((uint16_t)nAsciiDot[0x50+colLine]<<5);
	else
		result = result|((uint16_t)nAsciiDot[0x58+colLine]<<5);
	return result;
}

void display_Time(void)
{
	uint8_t i;
	uint32_t rowData;
	for(i=0;i<8;i++)
	{
		rowData = dataCombine_Time(i);
		write_ROW(rowData);
		ICN2012_E1 = 1;	
		delay(lightLevel);
		ICN2012_E1 = 0;	
		delay(0xFF-lightLevel);		
		P0 += 0x20;	
	}	
}

static uint32_t display_DataHandle(uint32_t rowData)
{
    uint8_t i,temp;
    uint32_t value=0;
	temp = (rowData>>16)&0xFF;
    for (i = 0; i < 8; ++i)
    {
        value <<= 1;
        value |= (temp&0x01);
        temp >>= 1; 
    }	
	temp = (rowData>>8)&0xFF;
    for (i = 0; i < 8; ++i)
    {
        value <<= 1;
        value |= (temp&0x01);
        temp >>= 1; 
    }
	temp = rowData&0xFF;
    for (i = 0; i < 8; ++i)
    {
        value <<= 1;
        value |= (temp&0x01);
        temp >>= 1; 
    }
    return ~value;
}

void write_ROW(uint32_t rowData)
{
	uint8_t i;

	HC595_RCLK = 0;
	rowData = display_DataHandle(rowData);
	
	for(i=0;i<24;i++)
	{
		HC595_SCLK = 0;
		if(rowData&0x800000)
		{
			HC595_SER=1;
		}
		else
		{
			HC595_SER=0;
		}
		HC595_SCLK = 1;
		rowData <<= 1;		
	}
	HC595_RCLK = 1;
}

void display(uint8_t mode)
{
	uint8_t i;
	uint32_t rowData;
	for(i=0;i<8;i++)
	{
		rowData = dataCombine_Time(i);				
		write_ROW(rowData);
		ICN2012_E1 = 1;	
		delay(lightLevel);
		ICN2012_E1 = 0;	
		delay(0xFF-lightLevel);		
		P0 += 0x20;					
	}
}

