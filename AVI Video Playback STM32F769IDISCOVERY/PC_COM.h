#ifndef __PC_COM_H
#define __PC_COM_H
#endif


#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_conf.h"




struct DataFrame
{
	volatile unsigned char ProcessFlag;	
	unsigned char FrameHead[2];		//֡ͷ
	volatile unsigned char FrameTail;		 	//֡β
	volatile unsigned char FrameSum;				//֡��У����
	volatile unsigned char CMD_Type;			//���������
	volatile unsigned int Data_Len;			//�������ĳ���
	unsigned char DataBuffer[1024];	//������
};



void PC_COM_Init( void );		//PC���ڵĳ�ʼ������

void PC_COM_RX_INT_Config( FunctionalState NewState );														//���ô���1�Ľ����ж�-USART1

void PC_COM_Send( unsigned char *buf,  unsigned int num );												//PC���ڷ�������

//unsigned char PC_COM_TXDBufferWrite( unsigned char *buffer, unsigned int num );		// ��PC���ڷ��ͻ�������д������

int PC_COM_Receive_Packet( void  );																								//��PC���ڽ��ջ�������ȡ����֡

int fputc(int ch, FILE *f);																												//Printf������ӳ��



