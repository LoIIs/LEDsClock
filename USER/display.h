
#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#define TimeMode			0x00
#define DateMode			0x01
#define T_RHMode			0x02
#define TimerUpMode1		0x03
#define TimerUpMode2		0x04
#define AlarmMode			0x05
#define NoPressMode			0x06

void display_Init(void);
void write_ROW(uint32_t rowData);
void display(uint8_t mode);
void display_Time(void);
void display_Date(void);
void display_T_RH(void);
void display_TimerUpMode(uint8_t timerUpMode);
//void display_TimerMode2(void);

#endif
