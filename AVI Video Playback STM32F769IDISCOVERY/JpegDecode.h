#ifndef __JPEGDECODE_H
#define __JPEGDECODE_H
#endif


#include "ff.h"	




//0��JPEGӲ������ DMAģʽ
//1��JPEGӲ������ ITģʽ
//2��JPEGӲ������ POLLģʽ
//3��LIBJPEG����ģʽ
//4��TJpgDEC����ģʽ
#define JPEG_Decode_Mode		0				//ʹ��֮ǰ�����ã�Ĭ��JPEGӲ������ DMAģʽ






typedef struct
{
  volatile unsigned char State;  
  unsigned char *DataBuffer;
  volatile unsigned int DataBufferSize;

}JPEG_Data_BufferTypeDef;



typedef struct
{
	volatile unsigned char Error_Flag;
	volatile unsigned char End_Flag;
	volatile unsigned int MCU_TotalNb;
	volatile unsigned char Input_Is_Paused;
	volatile unsigned char Output_Is_Paused;
	volatile unsigned int Data_Length;
	volatile unsigned int Data_Addr;
	volatile unsigned int JPEG_OUT_Read_BufferIndex;
	volatile unsigned int JPEG_OUT_Write_BufferIndex;
	volatile unsigned int JPEG_IN_Read_BufferIndex;
	volatile unsigned int JPEG_IN_Write_BufferIndex;
	volatile unsigned int MCU_BlockIndex;
}_Jpeg_Decode_Info;





#if (JPEG_Decode_Mode == 0 || JPEG_Decode_Mode == 1 || JPEG_Decode_Mode == 2 )
void HW_JPEG_Init( void );																																											//��ʼ��Ӳ��JPEG����/������
void hw_jpeg_decode(unsigned char *file, unsigned short int Xpos, unsigned short int Ypos );										//Ӳ��JPEG����
#endif


#if (JPEG_Decode_Mode == 3 )
unsigned char libjpeg_jpeg_decode(unsigned char *file, unsigned short int Xpos, unsigned short int Ypos );			//ʹ��LIBJPEG����JPEG
#endif


#if (JPEG_Decode_Mode == 4 )
unsigned char tjpglb_jpeg_decode( unsigned char *file, unsigned short int Xpos, unsigned short int Ypos );			//ʹ��TJPG����JPEG
#endif



unsigned char avi_video_decode(  unsigned char *file, unsigned int FrameSize );																	//AVI��Ƶ֡����

unsigned char Check_JPEG_Decode( FIL *file );																																																				//��ѯ�����Ƿ����

void Display_Image( unsigned short int Xpos, unsigned short int Ypos );																																				//��ʾ������ͼƬ


