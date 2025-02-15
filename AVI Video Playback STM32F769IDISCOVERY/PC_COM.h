#ifndef __PC_COM_H
#define __PC_COM_H
#endif


#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_conf.h"




struct DataFrame
{
	volatile unsigned char ProcessFlag;	
	unsigned char FrameHead[2];		//帧头
	volatile unsigned char FrameTail;		 	//帧尾
	volatile unsigned char FrameSum;				//帧的校验码
	volatile unsigned char CMD_Type;			//命令的类型
	volatile unsigned int Data_Len;			//数据区的长度
	unsigned char DataBuffer[1024];	//数据区
};



void PC_COM_Init( void );		//PC串口的初始化函数

void PC_COM_RX_INT_Config( FunctionalState NewState );														//配置串口1的接收中断-USART1

void PC_COM_Send( unsigned char *buf,  unsigned int num );												//PC串口发送数据

//unsigned char PC_COM_TXDBufferWrite( unsigned char *buffer, unsigned int num );		// 往PC串口发送缓存器里写入数据

int PC_COM_Receive_Packet( void  );																								//从PC串口接收缓存器读取数据帧

int fputc(int ch, FILE *f);																												//Printf函数重映射



