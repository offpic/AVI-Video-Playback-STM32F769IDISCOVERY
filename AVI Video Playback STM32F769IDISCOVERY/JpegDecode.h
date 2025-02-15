#ifndef __JPEGDECODE_H
#define __JPEGDECODE_H
#endif


#include "ff.h"	




//0：JPEG硬件解码 DMA模式
//1：JPEG硬件解码 IT模式
//2：JPEG硬件解码 POLL模式
//3：LIBJPEG解码模式
//4：TJpgDEC解码模式
#define JPEG_Decode_Mode		0				//使用之前需配置，默认JPEG硬件解码 DMA模式






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
void HW_JPEG_Init( void );																																											//初始化硬件JPEG编码/解码器
void hw_jpeg_decode(unsigned char *file, unsigned short int Xpos, unsigned short int Ypos );										//硬件JPEG解码
#endif


#if (JPEG_Decode_Mode == 3 )
unsigned char libjpeg_jpeg_decode(unsigned char *file, unsigned short int Xpos, unsigned short int Ypos );			//使用LIBJPEG解码JPEG
#endif


#if (JPEG_Decode_Mode == 4 )
unsigned char tjpglb_jpeg_decode( unsigned char *file, unsigned short int Xpos, unsigned short int Ypos );			//使用TJPG解码JPEG
#endif



unsigned char avi_video_decode(  unsigned char *file, unsigned int FrameSize );																	//AVI视频帧解码

unsigned char Check_JPEG_Decode( FIL *file );																																																				//查询解码是否完成

void Display_Image( unsigned short int Xpos, unsigned short int Ypos );																																				//显示解码后的图片


