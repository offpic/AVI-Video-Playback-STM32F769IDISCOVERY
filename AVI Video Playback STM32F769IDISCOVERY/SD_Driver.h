#ifndef __SD_DRIVER_H
#define __SD_DRIVER_H
#endif


unsigned char SD_Init( void );											//初始化SD卡

unsigned char SD_Detected( void );									//检查SD卡是否插入卡槽



#if _FS_NORTC == 0 
unsigned int get_time(void);																//Gets Time from RTC	
#endif

