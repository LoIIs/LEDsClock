
#include "N76E003.h"
#include "SFR_Macro.h"
#include "Function_define.h"
#include "Common.h"
#include "Delay.h"

#include "display.h"
#include "bsp_sensor.h"

uint16_t secCnt = 0;

extern bit time_sFlag;
extern uint8_t timeCnt;
extern uint8_t workMode;

void sys_Init()
{
	Set_All_GPIO_Quasi_Mode;
	bsp_Init();	
	display_Init();	
}

void main()
{
	sys_Init();
//	DS3231_SetTime(40,28,22,12,4,18,4);		//给时钟设置初始值
	SI7021_Measure();
	while(1)
	{	
		if(time_sFlag)
			photoresistor_ADC();
		switch(workMode)
		{
			case DateMode:
				DS3231_GetDate();
				display_Date();
			break;
			case T_RHMode:
				SI7021_Measure();
				display_T_RH();
			break;
			case TimerUpMode1:
				display_TimerUpMode(TimerUpMode1);
			break;
			case TimerUpMode2:
				display_TimerUpMode(TimerUpMode2);
			break;
			case AlarmMode:
				
			break;
			default:
				DS3231_GetTime();
				display_Time();		
				key_ADC();		
			break;
		}
		
	}
}

void Timer1_ISR (void) interrupt 3          //interrupt address is 0x001B
{ 
	static uint16_t i=0,j=0;
	i++;j++;
	if(j>666)
	{
		secCnt++;
		j = 0;
	}
	if(i>3333)
	{
		time_sFlag = ~time_sFlag;	
		i = 0;
		timeCnt++;
	}
}


