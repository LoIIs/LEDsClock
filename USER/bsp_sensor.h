
#ifndef __BSP_SENSOR_H__
#define __BSP_SENSOR_H__

#define I2C_CMD_WR		0x00
#define I2C_CMD_RD		0x01

#define SI7021_ADDR			0x80
#define SI7021_CMD_MRH		0xE5
#define SI7021_CMD_MT		0xE3
#define SI7021_CMD_Reset	0xFE

#define DS3231_ADDR			0xD0
#define DS3231_SECOND       0x00    //秒
#define DS3231_MINUTE       0x01    //分
#define DS3231_HOUR         0x02    //时
#define DS3231_WEEK         0x03    //星期
#define DS3231_DAY          0x04    //日
#define DS3231_MONTH        0x05    //月
#define DS3231_YEAR         0x06    //年
//闹铃1            
#define DS3231_SALARM1ECOND 0x07    //秒
#define DS3231_ALARM1MINUTE 0x08    //分
#define DS3231_ALARM1HOUR   0x09    //时
#define DS3231_ALARM1WEEK   0x0A    //星期/日
//闹铃2
#define DS3231_ALARM2MINUTE 0x0b    //分
#define DS3231_ALARM2HOUR   0x0c    //时
#define DS3231_ALARM2WEEK   0x0d    //星期/日
#define DS3231_CONTROL      0x0e    //控制寄存器
#define DS3231_STATUS       0x0f    //状态寄存器
#define BSY                 2       //忙
#define OSF                 7       //振荡器停止标志
#define DS3231_XTAL         0x10    //晶体老化寄存器
#define DS3231_TEMPERATUREH 0x11    //温度寄存器高字节(8位)
#define DS3231_TEMPERATUREL 0x12    //温度寄存器低字节(高2位) 

//extern xdata float Temperature;
//extern xdata uint8_t RelativeHumidity;
//extern xdata uint8_t time[8];

void bsp_Init();
void photoresistor_ADC();
void SI7021_Measure();
void DS3231_GetTime();
void DS3231_GetDate();
void I2C_SetValue_Byte(uint8_t address,uint8_t cmd,uint8_t dat);
void DS3231_SetTime(uint8_t sec,uint8_t min,uint8_t hour,uint8_t day,uint8_t month,uint8_t year,uint8_t week);

#endif
