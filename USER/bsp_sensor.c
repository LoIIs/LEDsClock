#include "N76E003.h"
#include "SFR_Macro.h"
#include "Function_define.h"
#include "Common.h"
#include "Delay.h"
#include "bsp_sensor.h"
#include "display.h"



uint8_t workMode = 0;
uint8_t lightLevel = 0x80;
uint16_t keyLevel = 0;

bit T_RH_OK_Flag = 0;
xdata int Temperature;
xdata uint8_t RelativeHumidity;
xdata uint8_t time[8]={0};
extern void display(uint8_t mode);
void bsp_Init()
{
	P03_PushPull_Mode;
	P13_OpenDrain_Mode;
	P14_OpenDrain_Mode;
	
	I2CLK = 9;
	I2CEN = 1;
//	I2C_SetValue_Byte(DS3231_ADDR,DS3231_CONTROL,0x18);
}


void key_ADC()
{
	Enable_ADC_AIN0;
	clr_ADCF;
	set_ADCS;									// ADC start trig signal
    while(ADCF == 0);
	keyLevel = (keyLevel|ADCRH)<<8;
	keyLevel |= ADCRL;
	
	if(keyLevel<0x1000)
		workMode = DateMode;
	else if(keyLevel<0x5000)
		workMode = TimeMode;
	else if(keyLevel<0x9000)
		workMode = T_RHMode;
	else if(keyLevel<0xBD00)
		workMode = TimerUpMode1;
	else if(keyLevel<0xF000)
		workMode = TimerUpMode2;
	else
		workMode = NoPressMode;
}

void photoresistor_ADC()
{
	Enable_ADC_AIN5;
	clr_ADCF;
	set_ADCS;									// ADC start trig signal
    while(ADCF == 0);
	lightLevel = ADCRH;
}


uint8_t I2C_GetValue_Byte(uint8_t address,uint8_t cmd)
{
	uint8_t value=0;
	/* 发送START命令 */
	STA = 1;SI = 0;while(!SI);
	/* 发送从机地址与读写控制位 */
	I2DAT = address|I2C_CMD_WR;
	STA = 0;SI = 0;	while(!SI);
	/* 发送传感器控制命令 */
	I2DAT = cmd;
	SI = 0;while(!SI);
	/* 重新开始I2C传输 */
	STA = 1;SI = 0;while(!SI);
	/* 修改为从从机读取数据模式 */
	I2DAT = address|I2C_CMD_RD;
	STA = 0;SI = 0;while(!SI);		
	/* 获取1个字节数据，并NACK相应 */
	AA = 0;SI = 0;while(!SI);
	value = I2DAT;
	/* 发送停止信号 */
	STO = 1;SI = 0;while(STO);
	return value;
}

void I2C_SetValue_Byte(uint8_t address,uint8_t cmd,uint8_t dat)
{
	uint8_t value=0;
	/* 发送START命令 */
	STA = 1;SI = 0;while(!SI);
	/* 发送从机地址与读写控制位 */
	I2DAT = address|I2C_CMD_WR;
	STA = 0;SI = 0;	while(!SI);
	/* 发送传感器控制命令 */
	I2DAT = cmd;
	SI = 0;while(!SI);
	I2DAT = dat;
	SI = 0;while(!SI);
	/* 发送停止信号 */
	STO = 1;SI = 0;while(STO);
}


uint16_t I2C_GetValue_Word(uint8_t address,uint8_t cmd)
{
	uint16_t value = 0;
	/* 发送START命令 */
	STA = 1;SI = 0;while(!SI);
	/* 发送从机地址与读写控制位 */
	I2DAT = address|I2C_CMD_WR;
	STA = 0;SI = 0;while(!SI);
	/* 发送传感器控制命令 */
	I2DAT = cmd;
	SI = 0;while(!SI);
	/* 重新开始I2C传输 */
	STA = 1;SI = 0;while(!SI);
	/* 修改为从从机读取数据模式 */
	I2DAT = address|I2C_CMD_RD;
	STA = 0;SI = 0;
	while(!SI) ;
	/* 获取1个字节数据，并ACK相应 */
	AA = 1;SI = 0;
	while(!SI) ;
	value = (value|I2DAT)<<8;	
	/* 获取1个字节数据，并NACK相应 */
	AA = 0;SI = 0;while(!SI);
	value |= I2DAT;
	/* 发送停止信号 */
	STO = 1;SI = 0;while(STO);
	return value;
}


void SI7021_Measure()
{
	uint32_t T_Value,RH_Value;
	T_Value = I2C_GetValue_Word(SI7021_ADDR,SI7021_CMD_MT);
	RH_Value = I2C_GetValue_Word(SI7021_ADDR,SI7021_CMD_MRH);
	
	Temperature = (17572*T_Value)/65536-4685;
	Temperature = Temperature/10;
	RelativeHumidity = ((RH_Value*125)/65536)-6;
}

uint8_t HEX2BCD(uint8_t dat)
{
	uint8_t temp;
	temp = (dat/10)<<4;
	temp = temp|(dat%10);
	return temp;
}

void DS3231_SetTime(uint8_t sec,uint8_t min,uint8_t hour,uint8_t day,uint8_t month,uint8_t year,uint8_t week)
{
	I2C_SetValue_Byte(DS3231_ADDR,DS3231_SECOND,HEX2BCD(sec));
	I2C_SetValue_Byte(DS3231_ADDR,DS3231_MINUTE,HEX2BCD(min));
	I2C_SetValue_Byte(DS3231_ADDR,DS3231_HOUR,HEX2BCD(hour));
	I2C_SetValue_Byte(DS3231_ADDR,DS3231_DAY,HEX2BCD(day));
	I2C_SetValue_Byte(DS3231_ADDR,DS3231_MONTH,HEX2BCD(month));
	I2C_SetValue_Byte(DS3231_ADDR,DS3231_YEAR,HEX2BCD(year));
	I2C_SetValue_Byte(DS3231_ADDR,DS3231_WEEK,week);
}

void DS3231_GetTime()
{
	time[0] = I2C_GetValue_Byte(DS3231_ADDR,DS3231_SECOND);
	time[1] = I2C_GetValue_Byte(DS3231_ADDR,DS3231_MINUTE);
	time[2] = I2C_GetValue_Byte(DS3231_ADDR,DS3231_HOUR);
//	time[3] = I2C_GetValue_Byte(DS3231_ADDR,DS3231_DAY);
//	time[4] = I2C_GetValue_Byte(DS3231_ADDR,DS3231_MONTH);
//	time[5] = I2C_GetValue_Byte(DS3231_ADDR,DS3231_YEAR);
//	time[6] = I2C_GetValue_Byte(DS3231_ADDR,DS3231_WEEK);
}

void DS3231_GetDate()
{
	time[3] = I2C_GetValue_Byte(DS3231_ADDR,DS3231_DAY);
	time[4] = I2C_GetValue_Byte(DS3231_ADDR,DS3231_MONTH);
	time[5] = I2C_GetValue_Byte(DS3231_ADDR,DS3231_YEAR);
	time[6] = I2C_GetValue_Byte(DS3231_ADDR,DS3231_WEEK);
}
