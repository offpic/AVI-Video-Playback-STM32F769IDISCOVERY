#ifndef __SD_DRIVER_H
#define __SD_DRIVER_H
#endif


unsigned char SD_Init( void );											//��ʼ��SD��

unsigned char SD_Detected( void );									//���SD���Ƿ���뿨��



#if _FS_NORTC == 0 
unsigned int get_time(void);																//Gets Time from RTC	
#endif

