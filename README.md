# 点阵时钟 #

----------
本时钟使用新塘N76E003AT单片机作为主控+DS3231MZ时钟芯片（使用法拉电容作为电池）+ICN2012作为行驱动、HC595作为列驱动+温湿度传感器SI7021+10K光敏电阻+蜂鸣器+五向摇杆（ADC方式）  
LED显示时会根据环境光自动调节亮度，不至于夜晚被亮瞎眼;)  
面对屏幕 摇杆右上角为<日期显示>，右下角为<温湿度显示>,左上角为<秒计时>，左下角为<分钟计时>，计时模式可设定定时时长。  
硬件设计可查看[设计原理图与PCB：](https://github.com/LoIIs/LEDsClock/tree/master/Hardware)  
![](https://github.com/LoIIs/LEDsClock/blob/master/docs/images/2345%E6%88%AA%E5%9B%BE20180412212408.jpg)  
**时间显示模式：**  
![](https://github.com/LoIIs/LEDsClock/blob/master/docs/images/IMG_E0688_%E7%9C%8B%E5%9B%BE%E7%8E%8B.jpg)  
**秒计时模式：**  
![](https://github.com/LoIIs/LEDsClock/blob/master/docs/images/IMG_E0690_%E7%9C%8B%E5%9B%BE%E7%8E%8B.jpg)  
