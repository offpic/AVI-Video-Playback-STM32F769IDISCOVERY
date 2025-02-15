#include "wm8994.h"
#include "PC_COM.h"
#include "SDRAM.h"

#include "ff.h"

#include "AudioPlayer.h"
#include "Audio_Driver.h"

//#include "mp3common.h"

#include <string.h>
#include <stdlib.h>


//#include  "os.h"



//extern void Convert_Mono(short *buffer);
//extern void Convert_Stereo(short *buffer);


FILINFO file_information;
FIL file_object;

unsigned char audio_buffer[1152];
unsigned short int audio_output_buffer[4608];

volatile unsigned char AudioOutFlag;
volatile unsigned char TxCpltFlag;
volatile unsigned char TxHalfCpltFlag;

volatile unsigned char RxCpltFlag;
volatile unsigned char RxHalfCpltFlag;


//HMP3Decoder hMP3Decoder;
//MP3FrameInfo mp3FrameInfo;


_SongInfo SongInfo;
_RecordInfo RecordInfo;
_MPEGAudioFrameInfo MPEGAudioFrameInfo;


//extern OS_MUTEX SD_Mutex;													//SD卡互斥信号量
//extern OS_MUTEX SDRAM_Mutex;											//SDRAM互斥信号量



///**
// * 比特率表 (kbits/s)
// */
//const short int MP3_BitrateTable[3][3][15] =
//{
//	{
//		{0,32,64,96,128,160,192,224,256,288,320,352,384,416,448},
//		{0,32,48,56,64 ,80 ,96 ,112,128,160,192,224,256,320,384},
//		{0,32,40,48,56 ,64 ,80 ,96 ,112,128,160,192,224,256,320}
//	},
//	{
//		{0,32,48,56,64 ,80 ,96 ,112,128,144,160,176,192,224,256},
//		{0,8 ,16,24,32 ,40 ,48 ,56 ,64 ,80 ,96 ,112,128,144,160},
//		{0,8 ,16,24,32 ,40 ,48 ,56 ,64 ,80 ,96 ,112,128,144,160}
//	},
//	{
//		{0,32,48,56,64 ,80 ,96 ,112,128,144,160,176,192,224,256},
//		{0,8 ,16,24,32 ,40 ,48 ,56 ,64 ,80 ,96 ,112,128,144,160},
//		{0,8 ,16,24,32 ,40 ,48 ,56 ,64 ,80 ,96 ,112,128,144,160}
//	}
//};


///**
// *采样率表 
// */
//const int MP3_SamplerateTable[3][3] =
//{
//	{44100,48000,32000}, // MPEG-1
//	{22050,24000,16000 }, // MPEG-2
//	{11025,12000,8000  }  // MPEG-2.5
//};


///**
// *边沿信息大小表
// */
//const char MP3_SideInfoTable[3][4] =
//{
//	{ 32, 32, 32,17 }, 	// MPEG-1
//	{17, 17, 17, 9 }, 	// MPEG-2
//	{17, 17, 17, 9 }  	// MPEG-2.5
//};


///**
// *每帧数据的采样数
// */
//const short int MP3_FrameSample[3][3] =
//{
//	{ 384, 1152, 1152 }, 	// MPEG-1
//	{ 384, 1152, 576 }, 	// MPEG-2
//	{ 384, 1152, 576 }  	// MPEG-2.5
//};






/**
* @brief  延时函数，延时Xms
* @note   
* @retval None
*/
void Audio_Delay_1ms( unsigned int ms )
{  
//	OS_ERR   p_err;
	
	unsigned int i, j;
	
	for( i=0; i<ms; i++ )
	for( j=0; j<100000; j++ );
	
//	OSTimeDly ( ms, OS_OPT_TIME_DLY, &p_err );
}








/**
* @brief  搜索一个字符串在另一个字符串的第一次出现
* @note  
* @param str1: 被搜索的字符串
* @param str2: 要搜索的字符串
* @param num: 被搜索的字符串的大小
* @retval 返回str2在str1首次出现的地址
*/
unsigned char *strstr2( unsigned char *str1, unsigned char *str2, unsigned short int num )
{
	unsigned short int temp1, temp2;
	unsigned short int i;
	unsigned char *p;
	
	temp1 = strlen( (char *)str2 );
	
	if( temp1 > num ) return NULL;
	
	temp2 = num;
	p = str1;

	for( i=0; i<num; i++ )
	{
		if( temp2 >= temp1 )
		{
			if( memcmp( p, str2, temp1 ) == 0 ) return p;
		}
		else
		{
			return NULL;
		}
		
		p++;
		temp2--;
	}
	
	return NULL;
}





/**
  * @brief  音频播放器初始化函数
  * @retval 1：成功 0：失败
  */
unsigned char AudioPlayer_Basic_Init( void )
{
	WM8994_GPIO_Config(  );							//配置MCU与wm8994的接口.
	WM8994_I2C_Config( );								//配置MCU与WM8994的I2C工作模式

	return 1;
}






///**
//  * @brief  计算MP3音频帧的大小
//	* @param  FrameAddr: 音频帧的地址
//  * @retval 返回MP3音频帧的大小
//  */
//unsigned int MP3_FrameSize( unsigned int FrameAddr )
//{
//	unsigned int size, read_num;
//	OS_ERR os_err;
//	CPU_TS ts;
//	
//	size = MPEGAudioFrameInfo.mFrameSize;
//	
//	OSMutexPend ( &SD_Mutex,
//								0,
//								OS_OPT_PEND_BLOCKING,
//								&ts,
//								&os_err);
//	
//	f_lseek( &file_object, FrameAddr );
//	
//	if( f_read( &file_object, audio_buffer, 4, &read_num ) != FR_OK ) 
//	{
//		f_close( &file_object );
//		
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
//		return 0;
//	}
//	
//	if( ( audio_buffer[2] & 0x02 ) >> 1 ) size = size + 1;
//	
//	if( f_read( &file_object, &audio_buffer[4], size-4, &read_num ) != FR_OK ) 
//	{
//		f_close( &file_object );
//		
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
//		
//		return 0;
//	}
//	
//	OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
//	
//	return size;
//}



//	

///**
//  * @brief  查找MP3文件的音频帧的起始地址
//	* @param  FrameNum: MP3文件音频帧的编号（不超过MP3音频帧数量的最大值）
//  * @retval 返回查找的音频帧在MP3文件的地址
//  */
//unsigned int SearchMp3Frame( unsigned int FrameNum )
//{
//	unsigned int addr, read_num;
//	unsigned short int temp, i;
//	unsigned char buf[4];
//	
//	if( FrameNum == 0 )	return MPEGAudioFrameInfo.mFirstFrameAddr;
//	
//	addr = MPEGAudioFrameInfo.mFirstFrameAddr;
//	
//	for( i=0; i<FrameNum; i++ )
//	{
//		temp = MPEGAudioFrameInfo.mFrameSize;
//		
//		f_lseek( &file_object, addr );
//		
//		f_read( &file_object, buf, 4, &read_num );
//		
//		if( ( buf[2] & 0x02 ) >> 1 ) temp = temp + 1;
//		
//		addr = addr + temp;
//	}
//	
//	return addr;
//}




///**
//  * @brief  根据时间点定位MP3音频帧
//	* @param  time: MP3文件播放时间（不超过MP3播放时间的最大值）单位：S
//  * @retval 返回查找的音频帧的编号
//  */
//unsigned int SearchFrameFromeMp3Time( unsigned short int time )
//{
//	unsigned int temp;
//	
//	if( time == 0 ) return 0;
//	
//	temp = time * MPEGAudioFrameInfo.mTotalFrames;
//	temp = (temp / SongInfo.time)  - 1;
//	
//	return temp;
//}





///**
//  * @brief  解码MP3音频帧
//	* @param  FrameNum: MP3文件音频帧的编号（不超过MP3音频帧数量的最大值）
//	* @param  buffer: 存放MP3解码后的数据
//  * @retval 其他值：音频帧的地址 0：失败
//  */
//unsigned int Mp3_Frame_Decoder( unsigned int FrameNum, short int *buffer )
//{
//	unsigned int  offset_addr, read_num;
//	unsigned char *p1;
//	int bytesLeft, temp;
//	
//	offset_addr = SearchMp3Frame( FrameNum );
//	
//	bytesLeft = MP3_FrameSize( offset_addr );
//	
//	f_lseek( &file_object, offset_addr );
//	
////	bytesLeft = MPEGAudioFrameInfo.mFrameSize;
//	
//	if( f_read( &file_object, audio_buffer, bytesLeft, &read_num ) != FR_OK ) 
//	{
//		return 0;
//	}
//		
//	p1 = audio_buffer;
//	
//	temp = MP3Decode( hMP3Decoder, &p1, &bytesLeft, buffer, 0 );
//	
//	if( temp != 0 ) 
//	{
//		return 0;
//	}
//	
//	MP3GetLastFrameInfo( hMP3Decoder, &mp3FrameInfo );
//	
//	if( mp3FrameInfo.nChans == 1 ) Convert_Mono((short *)buffer);
//	else	Convert_Stereo( (short *)buffer );
//	
//	return offset_addr;
//}




///**
//  * @brief  播放MP3的一桢数据
//	* @param  buffer: 存放MP3解码数据
//  * @retval 1：成功 0：失败
//  */
//unsigned char MP3FramePlay(	unsigned short int *buffer )
//{
//	if( BSP_AUDIO_OUT_Play( buffer, MPEGAudioFrameInfo.mSamplesPerFrame *2 ) != 0 ) return 0;
//	
//	return 1;
//}





///**
//  * @brief  MP3播放器的初始化函数
//	* @param  mp3_file: MP3文件的名字
//  * @retval 1：成功 0：失败
//  */
//unsigned char mp3_player_Init( char *mp3_file )
//{
//	unsigned int addr, i, read_num;
//	unsigned int ID3_Length, temp1;
//	double temp2;
//	unsigned char buf[4];
//	unsigned char *p, *mem;
//	OS_ERR os_err;
//	CPU_TS ts;
//	
//	memset( SongInfo.TIT2, 0 , 30 );
//	memset( SongInfo.TALB, 0 , 30 );
//	memset( SongInfo.TPE1, 0 , 20 );
//	memset( SongInfo.FileName, 0, 30 );
//	
//	if(BSP_AUDIO_OUT_Init( OUTPUT_DEVICE_AUTO, 80,  44100 ) != AUDIO_OK ) return 0;
//	
//	OSMutexPend ( &SD_Mutex,
//								0,
//								OS_OPT_PEND_BLOCKING,
//								&ts,
//								&os_err);
//	
//	if( f_open( &file_object, mp3_file, FA_OPEN_EXISTING | FA_READ ) != FR_OK ) 
//	{
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
//		return 0;
//	}
//	
//	SongInfo.FileSize = f_size( &file_object );
//	
//	memcpy( SongInfo.FileName, mp3_file, strlen( mp3_file) );
//	
//	if( f_read( &file_object, buf, 3, &read_num ) != FR_OK ) 
//	{
//		f_close( &file_object );
//		
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
//		return 0;
//	}
//	
//	if( memcmp( buf, (unsigned char *)"ID3", 3 ) != 0 )
//	{
////		printf("\r\n MP3文件损坏！" );
//		
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
//		return 0;
//	}
//	
//	f_lseek( &file_object, 6 );
//	
//	if( f_read( &file_object, buf, 4, &read_num ) != FR_OK ) 
//	{
//		f_close( &file_object );
//		
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
//		
//		return 0;
//	}
//	
//	ID3_Length = (buf[0] & 0x7f) * 0x200000;																					//计算ID3标签头的大小
//	ID3_Length = ID3_Length + ( (buf[1] & 0x7f) * 0x400 );
//	ID3_Length = ID3_Length + ( (buf[2] & 0x7f) * 0x80 );
//	ID3_Length = ID3_Length + (buf[3] & 0x7f) + 10;
//	
//	f_lseek( &file_object, 0 );
//	
////	addr = SDRAM_BUFFER + SDRAM_BANK1_ADDR;
//	
//	while( 1 )
//	{
//		mem =( unsigned char *)malloc( 2048 );
//		if( mem != NULL ) break;
//		
//		OSTimeDly ( 10, OS_OPT_TIME_DLY, &os_err );
//	}
//	
////	OSMutexPend ( &SDRAM_Mutex,
////								0,
////								OS_OPT_PEND_BLOCKING,
////								&ts,
////								&os_err);
//	
////	if( f_read( &file_object, (unsigned char *)addr, ID3_Length, &read_num ) != FR_OK ) 
//	if( f_read( &file_object, mem, ID3_Length, &read_num ) != FR_OK ) 
//	{
//		f_close( &file_object );
//		
//		free( mem );
//		
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
//		
////		OSMutexPost ( &SDRAM_Mutex,
////									OS_OPT_POST_NONE,
////									&os_err);
//		return 0;
//	}
//	
////	addr = SDRAM_BUFFER + SDRAM_BANK1_ADDR;
////	p = strstr2( (unsigned char *)addr, (unsigned char *)"TIT2", ID3_Length );
//	p = strstr2( mem, (unsigned char *)"TIT2", ID3_Length );
//	
//	if( p!= NULL )
//	{
//		temp1 = *( p + 4 );
//		temp1 =  (temp1 << 8) | (*( p + 5 ));
//		temp1 =  (temp1 << 8) | (*( p + 6 ));
//		temp1 =  (temp1 << 8) | (*( p + 7 ));
//		
//		for( i=0; i<temp1; i++ )
//		{
//			SongInfo.TIT2[i] = *( p + 10 + i );
//		}
//	}
//	
////	addr = SDRAM_BUFFER + SDRAM_BANK1_ADDR;
////	p = strstr2( (unsigned char *)addr, (unsigned char *)"TALB", ID3_Length );
//	p = strstr2( mem, (unsigned char *)"TALB", ID3_Length );
//	
//	if( p!= NULL )
//	{
//		temp1 = *( p + 4 );
//		temp1 =  (temp1 << 8) | (*( p + 5 ));
//		temp1 =  (temp1 << 8) | (*( p + 6 ));
//		temp1 =  (temp1 << 8) | (*( p + 7 ));
//		
//		for( i=0; i<temp1; i++ )
//		{
//			SongInfo.TALB[i] = *( p + 10 + i );
//		}
//	}
//	
////	addr = SDRAM_BUFFER + SDRAM_BANK1_ADDR;
////	p = strstr2( (unsigned char *)addr, (unsigned char *)"TPE1", ID3_Length );
//	p = strstr2( mem, (unsigned char *)"TPE1", ID3_Length );
//	
//	if( p!= NULL )
//	{
//		temp1 = *( p + 4 );
//		temp1 =  (temp1 << 8) | (*( p + 5 ));
//		temp1 =  (temp1 << 8) | (*( p + 6 ));
//		temp1 =  (temp1 << 8) | (*( p + 7 ));
//		
//		for( i=0; i<temp1; i++ )
//		{
//			SongInfo.TPE1[i] = *( p + 10 + i );
//		}
//	}
//	
//	free( mem );
//	
////	OSMutexPost ( &SDRAM_Mutex,
////									OS_OPT_POST_NONE,
////									&os_err);
//	
//	f_lseek( &file_object, ID3_Length );																										//解析音频帧头
//	
//	if( f_read( &file_object, buf, 4, &read_num ) != FR_OK ) 
//	{
//		f_close( &file_object );
//		
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
//		
//		return 0;
//	}
//	
//	if( buf[0] != 0xff ) return 0;
//	
//	temp1 = ( buf[1] & 0x18 ) >> 3;
//	
//	if( temp1 == 0 ) MPEGAudioFrameInfo.mMPEGVersion = 2;
//	else if( temp1 == 2 ) MPEGAudioFrameInfo.mMPEGVersion = 1;
//	else if( temp1 == 3 ) MPEGAudioFrameInfo.mMPEGVersion = 0;

//	
//	temp1 = ( buf[1] & 0x06 ) >> 1;
//	
//	if( temp1 == 1 )  MPEGAudioFrameInfo.mLayer = 2;
//	else if( temp1 == 2 ) MPEGAudioFrameInfo.mLayer = 1;
//	else if( temp1 == 3 ) MPEGAudioFrameInfo.mLayer = 0;
//	
//	temp1 = buf[1] & 0x01;
//	
//	if( temp1 == 1 ) MPEGAudioFrameInfo.mProtection = 1;
//	else MPEGAudioFrameInfo.mProtection = 0;
//	
//	temp1 = ( buf[2] & 0xf0 ) >> 4;
//	
//	MPEGAudioFrameInfo.mBitrate = MP3_BitrateTable[MPEGAudioFrameInfo.mMPEGVersion][MPEGAudioFrameInfo.mLayer][temp1] * 1000;
//	SongInfo.Bitrate = MPEGAudioFrameInfo.mBitrate;
//	
//	if( SongInfo.Bitrate >= 320000 )
//	{
////		printf("\r\n 不支持比特率超过320K的歌曲");
//		
//		f_close( &file_object );
//			
//		OSMutexPost ( &SD_Mutex,
//								OS_OPT_POST_NONE,
//								&os_err);
//		return 0;
//	}
//		
//	temp1 = ( buf[2] & 0x0c ) >> 2;
//	
//	MPEGAudioFrameInfo.mSamplerate = MP3_SamplerateTable[MPEGAudioFrameInfo.mMPEGVersion][temp1];
//	
//	temp1 = ( buf[2] & 0x02 ) >> 1;
//	
//	MPEGAudioFrameInfo.mPaddingSize = temp1;
//	
//	temp1 = ( buf[3] & 0xc0  ) >> 6;
//	
//	MPEGAudioFrameInfo.mChannelMode = temp1;
//	
//	temp1 = ( buf[3] & 0x30  ) >> 4;
//	
//	MPEGAudioFrameInfo.mExtensionMode = temp1;
//	
//	temp1 = ( buf[3] & 0x08  ) >> 3;
//	
//	MPEGAudioFrameInfo.mCopyrightBit = temp1;
//	
//	temp1 = ( buf[3] & 0x04  ) >> 2;
//	
//	MPEGAudioFrameInfo.mOriginalBit = temp1;
//	
//	temp1 = ( buf[3] & 0x03  );
//	
//	MPEGAudioFrameInfo.mEmphasis = temp1;
//	
//	MPEGAudioFrameInfo.mSideInfoSize = MP3_SideInfoTable[MPEGAudioFrameInfo.mMPEGVersion][MPEGAudioFrameInfo.mChannelMode];
//	
//	MPEGAudioFrameInfo.mSamplesPerFrame = MP3_FrameSample[MPEGAudioFrameInfo.mMPEGVersion][MPEGAudioFrameInfo.mLayer];
//	
//	if( MPEGAudioFrameInfo.mProtection == 1 ) addr = ID3_Length + 4 + MPEGAudioFrameInfo.mSideInfoSize;
//	else 
//	{
//		f_lseek( &file_object, ID3_Length+4 );
//	
//		if( f_read( &file_object, buf, 2, &read_num ) != FR_OK ) 
//		{
//			f_close( &file_object );
//			
//			OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
//			return 0;
//		}
//		
//		MPEGAudioFrameInfo.mCRCValue = buf[0];
//		MPEGAudioFrameInfo.mCRCValue = ( MPEGAudioFrameInfo.mCRCValue << 8 ) | buf[1];
//		
//		addr =  ID3_Length + 6 + MPEGAudioFrameInfo.mSideInfoSize;
//	}
//	
//	f_lseek( &file_object, addr );
//	
//	if( f_read( &file_object, buf, 4, &read_num ) != FR_OK ) 
//	{
//		f_close( &file_object );
//		
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
//		
//		return 0;
//	}

//	if( memcmp( buf, (unsigned char *)"Info", 4 ) != 0 )
//	{
//		MPEGAudioFrameInfo.mBitrateType = 2;
////		printf("\r\n 不支持可变比特率编码的文件");
//		return 0;
//	}
//		
//	MPEGAudioFrameInfo.mBitrateType = 1;
//	
//	f_lseek( &file_object, addr+8 );
//	
//	if( f_read( &file_object, buf, 4, &read_num ) != FR_OK ) 
//	{
//		f_close( &file_object );
//		
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
//		
//		return 0;
//	}

//	MPEGAudioFrameInfo.mTotalFrames = buf[0];
//	MPEGAudioFrameInfo.mTotalFrames = ( MPEGAudioFrameInfo.mTotalFrames << 8 ) | buf[1];
//	MPEGAudioFrameInfo.mTotalFrames = ( MPEGAudioFrameInfo.mTotalFrames << 8 ) | buf[2];
//	MPEGAudioFrameInfo.mTotalFrames = ( MPEGAudioFrameInfo.mTotalFrames << 8 ) | buf[3];
//	MPEGAudioFrameInfo.mTotalFrames = MPEGAudioFrameInfo.mTotalFrames;
//	
//	f_lseek( &file_object, addr+12 );
//	
//	if( f_read( &file_object, buf, 4, &read_num ) != FR_OK ) 
//	{
//		f_close( &file_object );
//		
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
//		
//		return 0;
//	}
//	
//	MPEGAudioFrameInfo.mTotalBytes = buf[0];
//	MPEGAudioFrameInfo.mTotalBytes = (MPEGAudioFrameInfo.mTotalBytes << 8 ) | buf[1];
//	MPEGAudioFrameInfo.mTotalBytes = (MPEGAudioFrameInfo.mTotalBytes << 8 ) | buf[2];
//	MPEGAudioFrameInfo.mTotalBytes = (MPEGAudioFrameInfo.mTotalBytes << 8 ) | buf[3];
//	
//	temp1 = MPEGAudioFrameInfo.mSamplesPerFrame * MPEGAudioFrameInfo.mBitrate;
//	temp2 = temp1;
//	temp2 = temp2 / 8.0 / MPEGAudioFrameInfo.mSamplerate;
//	
//	if( MPEGAudioFrameInfo.mPaddingSize == 1 )
//	{
//		if(MPEGAudioFrameInfo.mLayer == 0 ) MPEGAudioFrameInfo.mFrameSize = temp2 + 4;
//		else MPEGAudioFrameInfo.mFrameSize = temp2 + 1;
//	}
//	else MPEGAudioFrameInfo.mFrameSize = temp2;
//	
//	MPEGAudioFrameInfo.DecodeFrameTime = MPEGAudioFrameInfo.mSamplesPerFrame * 1000.0;									//播放一帧的时长
//	MPEGAudioFrameInfo.DecodeFrameTime = MPEGAudioFrameInfo.DecodeFrameTime / MPEGAudioFrameInfo.mSamplerate;
//	
//	MPEGAudioFrameInfo.mFirstFrameAddr = ID3_Length + MPEGAudioFrameInfo.mFrameSize;			//第一帧的地址
//	
//	SongInfo.time = MPEGAudioFrameInfo.mTotalFrames * MPEGAudioFrameInfo.DecodeFrameTime / 1000.0; 			//计算播放时长
//	
//	
//	BSP_AUDIO_OUT_SetFrequency( MPEGAudioFrameInfo.mSamplerate );
//	
//	if( BSP_AUDIO_OUT_SetOutputMode( OUTPUT_DEVICE_HEADPHONE ) != AUDIO_OK ) return 0;
//	
//	BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);
//	
//	if( BSP_AUDIO_OUT_SetVolume( 80 ) != AUDIO_OK ) return 0;
//	
//	
//	hMP3Decoder = MP3InitDecoder( );																																		 //初始化MP3解码器
//	
//	if( hMP3Decoder == 0 ) 
//	{
//		f_close( &file_object );
//		
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
//		
////		printf("\r\n MP3解码器初始化失败！");
//		return 0;
//	}
//	
//	OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
//	
//	return 1;
//}






///**
//  * @brief  播放MP3
//	* @param  time: MP3文件开始播放的时间点（不超过MP3播放时间的最大值），单位：s
//  * @retval 1：成功 0：失败
//  */
//unsigned char MP3Play( unsigned short int time )
//{
//	unsigned int i, read_num, temp;
//	unsigned char *p;
//	int bytesLeft;
//	unsigned int curren_frame_addr;
//	unsigned int curren_frame_size;
//	OS_ERR os_err;
//	CPU_TS ts;
//	
//	SongInfo.state = OK;
//	SongInfo.time_count = time;
//	
//	if( time == SongInfo.time ) 
//	{
//		OSMutexPend ( &SD_Mutex,
//								0,
//								OS_OPT_PEND_BLOCKING,
//								&ts,
//								&os_err);
//		
//		f_close( &file_object );
//		
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
//		
//		MP3FreeDecoder( hMP3Decoder );
//		return 1;
//	}
//	
//	i = SearchFrameFromeMp3Time( time );
//	
//	curren_frame_addr = MPEGAudioFrameInfo.mFirstFrameAddr;
//	curren_frame_size = MP3_FrameSize( curren_frame_addr );
//	
//	for( temp = 0; temp<=i+1; temp++ )
//	{
//		p = audio_buffer;
//		bytesLeft = curren_frame_size;
//		
//		if( temp == (i+1) )
//		{
//			if( MPEGAudioFrameInfo.mChannelMode == 3 )
//			{
//				if( MP3Decode( hMP3Decoder, &p, &bytesLeft, (short *)&audio_output_buffer[MPEGAudioFrameInfo.mSamplesPerFrame], 0 ) != 0 )
//				{
//					MP3FreeDecoder( hMP3Decoder );
//					
//					OSMutexPend ( &SD_Mutex,
//												0,
//												OS_OPT_PEND_BLOCKING,
//												&ts,
//												&os_err);
//					
//					f_close( &file_object );
//					
//					OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
//					
//					return 0;
//				}
//			}
//			else
//			{
//				if( MP3Decode( hMP3Decoder, &p, &bytesLeft, (short *)&audio_output_buffer[MPEGAudioFrameInfo.mSamplesPerFrame*2], 0 ) != 0 )
//				{
//					MP3FreeDecoder( hMP3Decoder );
//					
//					OSMutexPend ( &SD_Mutex,
//												0,
//												OS_OPT_PEND_BLOCKING,
//												&ts,
//												&os_err);
//					
//					f_close( &file_object );
//					
//					OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
//					
//					return 0;
//				}
//			}
//		}
//		else
//		{
//			if( MP3Decode( hMP3Decoder, &p, &bytesLeft, (short *)audio_output_buffer, 0 ) != 0 )
//			{
//				MP3FreeDecoder( hMP3Decoder );
//				
//				OSMutexPend ( &SD_Mutex,
//											0,
//											OS_OPT_PEND_BLOCKING,
//											&ts,
//											&os_err);
//				
//				f_close( &file_object );
//				
//				OSMutexPost ( &SD_Mutex,
//								OS_OPT_POST_NONE,
//								&os_err);
//				
//				return 0;
//			}
//		}	
//		
//		curren_frame_addr += curren_frame_size;
//		curren_frame_size = MPEGAudioFrameInfo.mFrameSize;
//	
//		OSMutexPend ( &SD_Mutex,
//									0,
//									OS_OPT_PEND_BLOCKING,
//									&ts,
//									&os_err);
//	
//		if( f_read( &file_object, audio_buffer, 4, &read_num ) != FR_OK ) 
//		{
//			f_close( &file_object );
//			
//			OSMutexPost ( &SD_Mutex,
//										OS_OPT_POST_NONE,
//										&os_err);
//			
//			MP3FreeDecoder( hMP3Decoder );
//			
//			return 0;
//		}
//	
//		if( ( audio_buffer[2] & 0x02 ) >> 1 ) curren_frame_size = curren_frame_size + 1;
//		
//		if( f_read( &file_object, &audio_buffer[4], curren_frame_size-4, &read_num ) != FR_OK ) 
//		{
//			f_close( &file_object );
//			
//			OSMutexPost ( &SD_Mutex,
//										OS_OPT_POST_NONE,
//										&os_err);
//			
//			return 0;
//		}
//	
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
//		
//		Audio_Delay_1ms( 1 );
//	}
//	
//	i++;
//	i++;
//	
//	curren_frame_size = MP3_FrameSize( curren_frame_addr );

//	TxHalfCpltFlag = 0;
//	TxCpltFlag = 0;
//	AudioOutFlag = 0;
//	
//	if( MPEGAudioFrameInfo.mChannelMode == 3 )	BSP_AUDIO_OUT_Play( audio_output_buffer, MPEGAudioFrameInfo.mSamplesPerFrame*2*2);
//	else BSP_AUDIO_OUT_Play( audio_output_buffer, MPEGAudioFrameInfo.mSamplesPerFrame*2*2*2);
//	
////	if( MPEGAudioFrameInfo.mChannelMode == 3 )	BSP_AUDIO_OUT_Play( (unsigned short int *)(SDRAM_BUFFER + SDRAM_BANK1_ADDR), MPEGAudioFrameInfo.mSamplesPerFrame*2*2 );
////	else BSP_AUDIO_OUT_Play( (unsigned short int *)(SDRAM_BUFFER + SDRAM_BANK1_ADDR), MPEGAudioFrameInfo.mSamplesPerFrame*2*2*2);
//	
//	AudioOutFlag = 1;
//	
//	for( ; i<MPEGAudioFrameInfo.mTotalFrames; i++ )
//	{
//		while( AudioOutFlag == 1 )
//		{
//			if( SongInfo.state == Pause )
//			{
//				BSP_AUDIO_OUT_Pause( );
//				
//				while( SongInfo.state == Pause )
//				{
//					Audio_Delay_1ms( 2 );
//				}
//			}
//			else if( SongInfo.state == Resume )
//			{
//				BSP_AUDIO_OUT_Resume( );
//				SongInfo.state = OK;
//			}
//			else if( SongInfo.state == Stop )
//			{
//				
//				OSMutexPend ( &SD_Mutex,
//											0,
//											OS_OPT_PEND_BLOCKING,
//											&ts,
//											&os_err);
//				
//				f_close( &file_object );
//				
//				OSMutexPost ( &SD_Mutex,
//								OS_OPT_POST_NONE,
//								&os_err);
//				
//				MP3FreeDecoder( hMP3Decoder );
//				BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
//				
//				SongInfo.state = OK;
//				
//				return 1;
//			}
//			
//			Audio_Delay_1ms( 2 );
//		}
//	
//		if( TxHalfCpltFlag == 1 )
//		{
//			SongInfo.time_count = (i+1) * SongInfo.time / MPEGAudioFrameInfo.mTotalFrames;
//			bytesLeft = curren_frame_size;

//			p = audio_buffer;
//			
////			if( MP3Decode( hMP3Decoder, &p, &bytesLeft, (short *)(SDRAM_BUFFER + SDRAM_BANK1_ADDR), 0 ) != 0 ) 
//			if( MP3Decode( hMP3Decoder, &p, &bytesLeft, (short *)audio_output_buffer, 0 ) != 0 )
//			{
//				curren_frame_addr = curren_frame_addr + curren_frame_size;
//				curren_frame_size = MPEGAudioFrameInfo.mFrameSize;
//	
//				OSMutexPend ( &SD_Mutex,
//											0,
//											OS_OPT_PEND_BLOCKING,
//											&ts,
//											&os_err);
//			
//				if( f_read( &file_object, audio_buffer, 4, &read_num ) != FR_OK ) 
//				{
//					f_close( &file_object );
//					
//					OSMutexPost ( &SD_Mutex,
//												OS_OPT_POST_NONE,
//												&os_err);
//					
//					MP3FreeDecoder( hMP3Decoder );
//					BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
//					
//					return 0;
//				}
//			
//				if( ( audio_buffer[2] & 0x02 ) >> 1 ) curren_frame_size = curren_frame_size + 1;
//				
//				if( f_read( &file_object, &audio_buffer[4], curren_frame_size-4, &read_num ) != FR_OK ) 
//				{
//					f_close( &file_object );
//					
//					OSMutexPost ( &SD_Mutex,
//												OS_OPT_POST_NONE,
//												&os_err);
//					
//					MP3FreeDecoder( hMP3Decoder );
//					BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
//					
//					return 0;
//				}
//			
//				OSMutexPost ( &SD_Mutex,
//											OS_OPT_POST_NONE,
//											&os_err);
//				
//				continue;
//			}
//			
//			MP3GetLastFrameInfo( hMP3Decoder, &mp3FrameInfo );
//			
//			if( mp3FrameInfo.nChans == 1 ) Convert_Mono( (short *)audio_output_buffer );
//			else	Convert_Stereo( (short *)audio_output_buffer );
//			
//			curren_frame_addr = curren_frame_addr + curren_frame_size;
//			curren_frame_size = MPEGAudioFrameInfo.mFrameSize;
//	
//			OSMutexPend ( &SD_Mutex,
//										0,
//										OS_OPT_PEND_BLOCKING,
//										&ts,
//										&os_err);
//		
//			if( f_read( &file_object, audio_buffer, 4, &read_num ) != FR_OK ) 
//			{
//				f_close( &file_object );
//				
//				OSMutexPost ( &SD_Mutex,
//											OS_OPT_POST_NONE,
//											&os_err);
//				
//				MP3FreeDecoder( hMP3Decoder );
//				BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
//				
//				return 0;
//			}
//		
//			if( ( audio_buffer[2] & 0x02 ) >> 1 ) curren_frame_size = curren_frame_size + 1;
//			
//			if( f_read( &file_object, &audio_buffer[4], curren_frame_size-4, &read_num ) != FR_OK ) 
//			{
//				f_close( &file_object );
//				
//				OSMutexPost ( &SD_Mutex,
//											OS_OPT_POST_NONE,
//											&os_err);
//				
//				MP3FreeDecoder( hMP3Decoder );
//				BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
//				
//				return 0;
//			}
//		
//			OSMutexPost ( &SD_Mutex,
//										OS_OPT_POST_NONE,
//											&os_err);
//			
//			TxHalfCpltFlag = 0;
//			AudioOutFlag = 1;
//		}
//		
//		if( TxCpltFlag == 1 )
//		{
//			SongInfo.time_count = (i+1) * SongInfo.time / MPEGAudioFrameInfo.mTotalFrames;
//			bytesLeft = curren_frame_size;
////			
////			f_lseek( &file_object, curren_frame_addr );
////			
////			if( f_read( &file_object, audio_buffer, bytesLeft, &read_num ) != FR_OK ) 
////			{
////				f_close( &file_object );
////				MP3FreeDecoder( hMP3Decoder );
////				BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
////				return 0;
////			}


//			p = audio_buffer;
//			
//			if( MPEGAudioFrameInfo.mChannelMode == 3 )
//			{
//				if( MP3Decode( hMP3Decoder, &p, &bytesLeft, (short *)&audio_output_buffer[MPEGAudioFrameInfo.mSamplesPerFrame], 0 ) != 0 ) 
////				if( MP3Decode( hMP3Decoder, &p, &bytesLeft, (short *)(SDRAM_BUFFER + SDRAM_BANK1_ADDR + MPEGAudioFrameInfo.mSamplesPerFrame * 2 ), 0 ) != 0 )
//				{
//					curren_frame_addr = curren_frame_addr + curren_frame_size;
//					curren_frame_size = MPEGAudioFrameInfo.mFrameSize;
//	
//					OSMutexPend ( &SD_Mutex,
//												0,
//												OS_OPT_PEND_BLOCKING,
//												&ts,
//												&os_err);
//				
//					if( f_read( &file_object, audio_buffer, 4, &read_num ) != FR_OK ) 
//					{
//						f_close( &file_object );
//						
//						OSMutexPost ( &SD_Mutex,
//													OS_OPT_POST_NONE,
//													&os_err);
//						
//						MP3FreeDecoder( hMP3Decoder );
//						BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
//						
//						return 0;
//					}
//				
//					if( ( audio_buffer[2] & 0x02 ) >> 1 ) curren_frame_size = curren_frame_size + 1;
//					
//					if( f_read( &file_object, &audio_buffer[4], curren_frame_size-4, &read_num ) != FR_OK ) 
//					{
//						f_close( &file_object );
//						
//						OSMutexPost ( &SD_Mutex,
//													OS_OPT_POST_NONE,
//													&os_err);
//						
//						MP3FreeDecoder( hMP3Decoder );
//						BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
//						
//						return 0;
//					}
//				
//					OSMutexPost ( &SD_Mutex,
//												OS_OPT_POST_NONE,
//													&os_err);
//					continue;
//				}
//			}
//			else
//			{
//				if( MP3Decode( hMP3Decoder, &p, &bytesLeft, (short *)&audio_output_buffer[MPEGAudioFrameInfo.mSamplesPerFrame*2], 0 ) != 0 ) 
////				if( MP3Decode( hMP3Decoder, &p, &bytesLeft, (short *)(SDRAM_BUFFER + SDRAM_BANK1_ADDR + MPEGAudioFrameInfo.mSamplesPerFrame * 4 ), 0 ) != 0 )
//				{
//					curren_frame_addr = curren_frame_addr + curren_frame_size;
//					curren_frame_size = MPEGAudioFrameInfo.mFrameSize;
//	
//					OSMutexPend ( &SD_Mutex,
//												0,
//												OS_OPT_PEND_BLOCKING,
//												&ts,
//												&os_err);
//				
//					if( f_read( &file_object, audio_buffer, 4, &read_num ) != FR_OK ) 
//					{
//						f_close( &file_object );
//						
//						OSMutexPost ( &SD_Mutex,
//													OS_OPT_POST_NONE,
//													&os_err);
//						
//						MP3FreeDecoder( hMP3Decoder );
//						BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
//						
//						return 0;
//					}
//				
//					if( ( audio_buffer[2] & 0x02 ) >> 1 ) curren_frame_size = curren_frame_size + 1;
//					
//					if( f_read( &file_object, &audio_buffer[4], curren_frame_size-4, &read_num ) != FR_OK ) 
//					{
//						f_close( &file_object );
//						
//						OSMutexPost ( &SD_Mutex,
//													OS_OPT_POST_NONE,
//													&os_err);
//						
//						MP3FreeDecoder( hMP3Decoder );
//						BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
//						
//						return 0;
//					}
//				
//					OSMutexPost ( &SD_Mutex,
//												OS_OPT_POST_NONE,
//												&os_err);
//					
//					continue;
//				}
//			}
//		
//			MP3GetLastFrameInfo( hMP3Decoder, &mp3FrameInfo );
//			
//			if( mp3FrameInfo.nChans == 1 ) Convert_Mono((short *)&audio_output_buffer[MPEGAudioFrameInfo.mSamplesPerFrame]);
//			else	Convert_Stereo( (short *)&audio_output_buffer[MPEGAudioFrameInfo.mSamplesPerFrame*2] );
//			
////			if( mp3FrameInfo.nChans == 1 ) Convert_Mono((short *)(SDRAM_BUFFER + SDRAM_BANK1_ADDR + MPEGAudioFrameInfo.mSamplesPerFrame * 2 ));
////			else	Convert_Stereo( (short *)(SDRAM_BUFFER + SDRAM_BANK1_ADDR + MPEGAudioFrameInfo.mSamplesPerFrame * 4 ) );
//			
//			curren_frame_addr = curren_frame_addr + curren_frame_size;
//			curren_frame_size = MPEGAudioFrameInfo.mFrameSize;
//	
//			OSMutexPend ( &SD_Mutex,
//										0,
//										OS_OPT_PEND_BLOCKING,
//										&ts,
//										&os_err);
//		
//			if( f_read( &file_object, audio_buffer, 4, &read_num ) != FR_OK ) 
//			{
//				f_close( &file_object );
//				
//				OSMutexPost ( &SD_Mutex,
//											OS_OPT_POST_NONE,
//											&os_err);
//				
//				MP3FreeDecoder( hMP3Decoder );
//				BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
//				
//				return 0;
//			}
//		
//			if( ( audio_buffer[2] & 0x02 ) >> 1 ) curren_frame_size = curren_frame_size + 1;
//			
//			if( f_read( &file_object, &audio_buffer[4], curren_frame_size-4, &read_num ) != FR_OK ) 
//			{
//				f_close( &file_object );
//				
//				OSMutexPost ( &SD_Mutex,
//											OS_OPT_POST_NONE,
//											&os_err);
//				
//				MP3FreeDecoder( hMP3Decoder );
//				BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
//				
//				return 0;
//			}
//		
//			OSMutexPost ( &SD_Mutex,
//										OS_OPT_POST_NONE,
//										&os_err);

//			TxCpltFlag = 0;
//			AudioOutFlag = 1;
//		}
//	}
//	
//	
//	OSMutexPend ( &SD_Mutex,
//								0,
//								OS_OPT_PEND_BLOCKING,
//								&ts,
//								&os_err);
//	
//	f_close( &file_object );
//	
//	OSMutexPost ( &SD_Mutex,
//					OS_OPT_POST_NONE,
//					&os_err);

//	MP3FreeDecoder( hMP3Decoder );
//	BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
//	
//	return 1;
//}





/**
  * @brief  wav播放器的初始化函数
	* @param  wav_file: wav文件的名字
  * @retval 1：成功 0：失败
  */
unsigned char wav_player_Init( char *wav_file )
{
	unsigned int  temp1;
	unsigned int read_num;
	unsigned char buf[4];
//	OS_ERR os_err;
//	CPU_TS ts;
	
	memset( SongInfo.TIT2, 0 , 30 );
	memset( SongInfo.TALB, 0 , 30 );
	memset( SongInfo.TPE1, 0 , 20 );
	memset( SongInfo.FileName, 0, 30 );
	
//	OSMutexPend ( &SD_Mutex,
//								0,
//								OS_OPT_PEND_BLOCKING,
//								&ts,
//								&os_err);
	
	if( f_open( &file_object, wav_file, FA_OPEN_EXISTING | FA_READ ) != FR_OK ) 
	{
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
		
		return 0;
	}
	
	if( f_read( &file_object, buf, 4, &read_num ) != FR_OK ) 
	{
		f_close( &file_object );
		
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
		return 0;
	}
	
	if( memcmp( buf, (unsigned char *)"RIFF", 4 ) != 0 )
	{
		f_close( &file_object );
		
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
		return 0;
	}
	
	f_lseek( &file_object, 8 );
	
	if( f_read( &file_object, buf, 4, &read_num ) != FR_OK ) 
	{
		f_close( &file_object );
		
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
		
		return 0;
	}
	
	if( memcmp( buf, (unsigned char *)"WAVE", 4 ) != 0 )
	{
		f_close( &file_object );
		
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
		return 0;
	}
	
	memcpy( SongInfo.FileName, wav_file, strlen( wav_file) );
	
	SongInfo.FileSize = f_size( &file_object );																				//计算文件大小
	
	f_lseek( &file_object, 22 );
	
	if( f_read( &file_object, buf, 2, &read_num ) != FR_OK ) 
	{
		f_close( &file_object );
		
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
		return 0;
	}
	
	MPEGAudioFrameInfo.mChannelMode = buf[0];																					//计算声道数量
	
	if( f_read( &file_object, buf, 4, &read_num ) != FR_OK ) 
	{
		f_close( &file_object );
		
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
		return 0;
	}
	
	temp1 = buf[3];																																		//计算采样频率
	temp1 = ( temp1 << 8 ) | buf[2];
	temp1 = ( temp1 << 8 ) | buf[1];
	temp1 = ( temp1 << 8 ) | buf[0];
	MPEGAudioFrameInfo.mSamplerate = temp1;
	
	if( f_read( &file_object, buf, 4, &read_num ) != FR_OK ) 
	{
		f_close( &file_object );
		
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
		return 0;
	}
	
	temp1 = buf[3];																																		//计算比特率
	temp1 = ( temp1 << 8 ) | buf[2];
	temp1 = ( temp1 << 8 ) | buf[1];
	temp1 = ( temp1 << 8 ) | buf[0];
	MPEGAudioFrameInfo.mBitrate = temp1;
	SongInfo.Bitrate = temp1;
	
	f_lseek( &file_object, 38 );
	
	if( f_read( &file_object, buf, 4, &read_num ) != FR_OK ) 
	{
		f_close( &file_object );
		
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
		
		return 0;
	}
	
	if( memcmp( buf, (char  *)"fact", 4 ) == 0 )
	{
		f_lseek( &file_object, 54 );
		
		if( f_read( &file_object, buf, 4, &read_num ) != FR_OK ) 
		{
			f_close( &file_object );

//			OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
			return 0;
		}
		
		temp1 = buf[3];																																		//计算音频数据的大小
		temp1 = ( temp1 << 8 ) | buf[2];
		temp1 = ( temp1 << 8 ) | buf[1];
		temp1 = ( temp1 << 8 ) | buf[0];
		MPEGAudioFrameInfo.mTotalBytes = temp1;
		
		MPEGAudioFrameInfo.mFirstFrameAddr = 58;
	}
	else
	{
		f_lseek( &file_object, 38 );
		
		if( f_read( &file_object, buf, 4, &read_num ) != FR_OK ) 
		{
			f_close( &file_object );
			
//			OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
			
			return 0;
		}
		
		if( memcmp( buf, (char  *)"data", 4 ) != 0 ) 
		{
			f_close( &file_object );
			
//			OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
			
			return 0;
		}
		
		if( f_read( &file_object, buf, 4, &read_num ) != FR_OK ) 
		{
			f_close( &file_object );
			
//			OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
			
			return 0;
		}
		
		temp1 = buf[3];																																		//计算音频数据的大小
		temp1 = ( temp1 << 8 ) | buf[2];
		temp1 = ( temp1 << 8 ) | buf[1];
		temp1 = ( temp1 << 8 ) | buf[0];
		MPEGAudioFrameInfo.mTotalBytes = temp1;
		
		MPEGAudioFrameInfo.mFirstFrameAddr = 44;
	}
	
//	OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
	SongInfo.time = MPEGAudioFrameInfo.mTotalBytes / MPEGAudioFrameInfo.mBitrate;		 //计算歌曲播放时长
	
	
	if( MPEGAudioFrameInfo.mChannelMode == 1 ) MPEGAudioFrameInfo.mFrameSize = 2;
	else MPEGAudioFrameInfo.mFrameSize = 4;
	
	if( MPEGAudioFrameInfo.mChannelMode == 1 ) MPEGAudioFrameInfo.mTotalFrames = MPEGAudioFrameInfo.mTotalBytes / 2;
	else	MPEGAudioFrameInfo.mTotalFrames = MPEGAudioFrameInfo.mTotalBytes / 4;
	
	
	if(BSP_AUDIO_OUT_Init( OUTPUT_DEVICE_HEADPHONE, 80, MPEGAudioFrameInfo.mSamplerate ) != AUDIO_OK ) return 0;
	
	BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);
	
	return 1;
}








/**
  * @brief  查找Wav文件的音频帧的起始地址
	* @param  FrameNum: Wav文件音频帧的编号（不超过Wav音频帧数量的最大值）
  * @retval 返回查找的音频帧在Wav文件的地址
  */
unsigned int SearchWAVFrame( unsigned int FrameNum )
{
	unsigned int i, addr;
	
	if( FrameNum == 0 )	return MPEGAudioFrameInfo.mFirstFrameAddr;
	
	addr = MPEGAudioFrameInfo.mFirstFrameAddr;
	
	for( i=0; i<FrameNum; i++ )
	{
		addr = addr + MPEGAudioFrameInfo.mFrameSize;
	}
	
	return addr;
}







/**
  * @brief  根据时间点定位Wav音频帧
	* @param  time: Wav文件播放时间（不超过Wav播放时间的最大值）单位：S
  * @retval 返回查找的音频帧的编号
  */
unsigned int SearchFrameFromeWAVTime( unsigned short int time )
{
	unsigned int temp;
	
	if( time == 0 ) return 0;
	
	temp = time * MPEGAudioFrameInfo.mTotalFrames;
	temp = temp /  SongInfo.time - 1;
	
	return temp;
}





/**
  * @brief  解码Wav音频帧
	* @param  FrameNum: Wav文件音频帧的编号（不超过Wav音频帧数量的最大值）
	* @param	num：解码的帧数
	* @param  buffer: 存放Wav解码后的数据
  * @retval 1：成功 0：失败
  */
unsigned char Wav_Frame_Decoder( unsigned int FrameNum, unsigned short int num, unsigned char *buffer )
{
	unsigned int  addr, read_num;
//	OS_ERR os_err;
//	CPU_TS ts;
	
	addr = SearchWAVFrame( FrameNum );
	
//	OSMutexPend ( &SD_Mutex,
//								0,
//								OS_OPT_PEND_BLOCKING,
//								&ts,
//								&os_err);
	
	f_lseek( &file_object, addr );
		
	if( f_read( &file_object, buffer, num*MPEGAudioFrameInfo.mFrameSize, &read_num ) != FR_OK ) 
	{
		f_close( &file_object );
		
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
		
		return 0;
	}
	
//	OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
	
	return 1;
}




/**
  * @brief  播放WAV
	* @param  time: WAV文件开始播放的时间点（不超过WAV播放时间的最大值），单位：s
  * @retval 1：成功 0：失败
  */
unsigned char WAVPlay( unsigned short int time )
{
	unsigned int FrameNum, i, num, read_num;
	unsigned char flag = 0;
	unsigned short int *p;
//	OS_ERR os_err;
//	CPU_TS ts;
	
	SongInfo.state = OK;
	SongInfo.time_count = time;
	FrameNum = SearchFrameFromeWAVTime( time );
	f_lseek( &file_object, SearchWAVFrame( FrameNum ) );
	
	num = MPEGAudioFrameInfo.mTotalFrames - FrameNum;
	
	AudioOutFlag = 0;
	TxHalfCpltFlag = 0;
	TxCpltFlag = 0;
	
	while( num != 0 )
	{
		if(MPEGAudioFrameInfo.mChannelMode == 1 )
		{
			if( flag == 0 )  
			{
				i = 4096;
			}
			else
			{
				if( num > 2048 ) i = 2048;
				else i = num;
			}
		}
		else
		{
			if( flag == 0 )  
			{
				i = 2048;
			}
			else
			{
				if( num > 1024 ) i = 1024;
				else i = num;
			}
		}
		
		while( AudioOutFlag == 1 )
		{
			if( SongInfo.state == Pause )
			{
				BSP_AUDIO_OUT_Pause( );
				while( SongInfo.state == Pause )
				{
					Audio_Delay_1ms( 1 );
				}
			}
			else if( SongInfo.state == Resume )
			{
				BSP_AUDIO_OUT_Resume( );
				SongInfo.state = OK;
			}
			else if( SongInfo.state == Stop )
			{
//				OSMutexPend ( &SD_Mutex,
//											0,
//											OS_OPT_PEND_BLOCKING,
//											&ts,
//											&os_err);
				
				f_close( &file_object );
				
//				OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
				
//				MP3FreeDecoder( hMP3Decoder );
				BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
				
				SongInfo.state = OK;
				
				return 1;
			}
			
			Audio_Delay_1ms( 2 );
		}
		
		if( flag == 0 )
		{
//			OSMutexPend ( &SD_Mutex,
//											0,
//											OS_OPT_PEND_BLOCKING,
//											&ts,
//											&os_err);
			
			if( f_read( &file_object,(unsigned char *)audio_output_buffer, i*MPEGAudioFrameInfo.mFrameSize, &read_num ) != FR_OK ) 
			{
				f_close( &file_object );
				
//				OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
				
				BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
				return 0;
			}
			
//			OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
			
			p = audio_output_buffer;
			FrameNum += i;
			num = num - i;
			
			BSP_AUDIO_OUT_Play( p, i*MPEGAudioFrameInfo.mFrameSize );
			
			flag = 1;
			AudioOutFlag = 1;
		}
		else
		{
			if( TxHalfCpltFlag == 1 )
			{ 
//				OSMutexPend ( &SD_Mutex,
//											0,
//											OS_OPT_PEND_BLOCKING,
//											&ts,
//											&os_err);
				
				if( f_read( &file_object,(unsigned char *)audio_output_buffer, i*MPEGAudioFrameInfo.mFrameSize, &read_num ) != FR_OK ) 
				{
					f_close( &file_object );
					
//					OSMutexPost ( &SD_Mutex,
//												OS_OPT_POST_NONE,
//												&os_err);
			
					BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
					return 0;
				}
				
//				OSMutexPost ( &SD_Mutex,
//												OS_OPT_POST_NONE,
//												&os_err);
				
				SongInfo.time_count = (FrameNum+1) * SongInfo.time / MPEGAudioFrameInfo.mTotalFrames;
				
				FrameNum += i;
				num = num - i;
				AudioOutFlag = 1;
				TxHalfCpltFlag = 0;
			}
			
			if( TxCpltFlag  == 1 )
			{		
//				OSMutexPend ( &SD_Mutex,
//											0,
//											OS_OPT_PEND_BLOCKING,
//											&ts,
//											&os_err);
				
				if( f_read( &file_object,(unsigned char *)&audio_output_buffer[2048], i*MPEGAudioFrameInfo.mFrameSize, &read_num ) != FR_OK ) 
				{
					f_close( &file_object );
					
//					OSMutexPost ( &SD_Mutex,
//												OS_OPT_POST_NONE,
//												&os_err);
					
					BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
					return 0;
				}
				
//				OSMutexPost ( &SD_Mutex,
//												OS_OPT_POST_NONE,
//												&os_err);
				
				SongInfo.time_count = (FrameNum+1) * SongInfo.time / MPEGAudioFrameInfo.mTotalFrames;
				
				FrameNum += i;
				num = num - i;
				AudioOutFlag = 1;
				TxCpltFlag  = 0;
			}
		}
	}
	
	BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
	
//	OSMutexPend ( &SD_Mutex,
//								0,
//								OS_OPT_PEND_BLOCKING,
//								&ts,
//								&os_err);
	
	f_close( &file_object );
	
//	OSMutexPost ( &SD_Mutex,
//								OS_OPT_POST_NONE,
//								&os_err);
	
	return 1;
}






/**
  * @brief  开始录音
	*	@param  wav_file: 录音文件存放的名字
	* @param  AudioFreq: Audio frequency used to play the audio stream
  * @retval 1：成功 0：失败
  */
unsigned char AudioRecord( char *wav_file, int AudioFreq )
{
	unsigned char buffer[35] = "Record/";
	unsigned int bw, temp;
	unsigned char AudioInFlag;
	FRESULT res;
//	OS_ERR os_err;
//	CPU_TS ts;
//	
	if(	BSP_AUDIO_IN_Init( INPUT_DEVICE_DIGITAL_MICROPHONE_2, 100,  AudioFreq ) != AUDIO_OK )	return 0;
	
	RecordInfo.Samplerate = AudioFreq;
	RecordInfo.FileSize = 0;
	memcpy( RecordInfo.FileName, wav_file, strlen(wav_file) );

	memcpy( &buffer[7], wav_file, strlen(wav_file) );
	
//	OSMutexPend ( &SD_Mutex,
//								0,
//								OS_OPT_PEND_BLOCKING,
//								&ts,
//								&os_err);
	
	res = f_stat( (const TCHAR*)"Record", &file_information );
	
	if( res == FR_NO_FILE )
	{
		if( f_mkdir( (const TCHAR*)"Record" ) != FR_OK )	
		{
//			OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
			
			return 0;
		}
	}
	else if( res != FR_OK )	
	{
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
		return 0;
	}
	
	if( f_open( &file_object, (const TCHAR*)buffer, FA_CREATE_ALWAYS | FA_WRITE ) != FR_OK ) 
	{
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
		
//		printf("\r\n 无法创建文件" );
		return 0;
	}
	
	if( f_write( &file_object, ( unsigned char *)"RIFF", 4, &bw ) != FR_OK ) 
	{
		f_close( &file_object );
		
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
		
		return 0;
	}
	
	buffer[0] = 0;
	buffer[1] = 0;
	buffer[2] = 0;
	buffer[3] = 0;
	if( f_write( &file_object, buffer, 4, &bw ) != FR_OK ) 
	{
		f_close( &file_object );
		
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
		
		return 0;
	}
	
	if( f_write( &file_object, ( unsigned char *)"WAVE", 4, &bw ) != FR_OK ) 
	{
		f_close( &file_object );
		
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
		
		return 0;
	}
	
	if( f_write( &file_object, ( unsigned char *)"fmt ", 4, &bw ) != FR_OK ) 
	{
		f_close( &file_object );
		
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
		
		return 0;
	}
	
	buffer[0] = 0x10;
	buffer[1] = 0;
	buffer[2] = 0;
	buffer[3] = 0;
	if( f_write( &file_object, buffer, 4, &bw ) != FR_OK ) 
	{
		f_close( &file_object );
		
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
		
		return 0;
	}
	
	buffer[0] = 0x01;
	buffer[1] = 0;
	if( f_write( &file_object, buffer, 2, &bw ) != FR_OK ) 
	{
		f_close( &file_object );
		
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
		
		return 0;
	}
	
	buffer[0] = 0x02;
	buffer[1] = 0;
	if( f_write( &file_object, buffer, 2, &bw ) != FR_OK ) 
	{
		f_close( &file_object );
		
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
		
		return 0;
	}
	
	buffer[3] = AudioFreq >> 24;
	buffer[2] = ( AudioFreq >> 16 ) & 0x00ff;
	buffer[1] = ( AudioFreq >> 8 ) & 0x0000ff;
	buffer[0] = AudioFreq & 0x000000ff;
	if( f_write( &file_object, buffer, 4, &bw ) != FR_OK ) 
	{
		f_close( &file_object );
		
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
		
		return 0;
	}
	
	temp = AudioFreq;
	temp = temp * 2 * 2;
	RecordInfo.Bitrate = temp;
	
	buffer[3] = temp >> 24;
	buffer[2] = ( temp >> 16 ) & 0x00ff;
	buffer[1] = ( temp >> 8 ) & 0x0000ff;
	buffer[0] = temp & 0x000000ff;
	if( f_write( &file_object, buffer, 4, &bw ) != FR_OK ) 
	{
		f_close( &file_object );
		
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
		
		return 0;
	}
	
	buffer[0] = 0x04;
	buffer[1] = 0;
	if( f_write( &file_object, buffer, 2, &bw ) != FR_OK ) 
	{
		f_close( &file_object );
		
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
		
		return 0;
	}
	
	buffer[0] = 0x10;
	buffer[1] = 0;
	if( f_write( &file_object, buffer, 2, &bw ) != FR_OK ) 
	{
		f_close( &file_object );
		
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
		
		return 0;
	}
	
	if( f_write( &file_object, ( unsigned char *)"data", 4, &bw ) != FR_OK ) 
	{
		f_close( &file_object );
		
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
		
		return 0;
	}
	
	buffer[0] = 0;
	buffer[1] = 0;
	buffer[2] = 0;
	buffer[3] = 0;
	if( f_write( &file_object, buffer, 4, &bw ) != FR_OK ) 
	{
		f_close( &file_object );
		
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);
		
		return 0;
	}
	
//	OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&os_err);

	RxCpltFlag = 0;
	RxHalfCpltFlag = 0;
	temp = 0;
	
	BSP_AUDIO_IN_Record( audio_output_buffer, 4608 );
	
	AudioInFlag = 1;
	RecordInfo.state = OK;
	RecordInfo.time_count = 0;
	RecordInfo.time = 0;
	
//	PC_COM_Send( (unsigned char *)"\r\n 开始录音", 11 );
	
	while( AudioInFlag == 1 )
	{
		Audio_Delay_1ms( 1 );
		
		if( RxCpltFlag == 1 )
		{
//			OSMutexPend ( &SD_Mutex,
//										0,
//										OS_OPT_PEND_BLOCKING,
//										&ts,
//										&os_err);
			
			if( f_write( &file_object, (unsigned char *)(audio_output_buffer+2304), 4608, &bw ) != FR_OK ) 
			{
				f_close( &file_object );
				
//				OSMutexPost ( &SD_Mutex,
//											OS_OPT_POST_NONE,
//											&os_err);
				
				BSP_AUDIO_IN_Stop(CODEC_PDWN_SW);
				return 0;
			}
			
//			OSMutexPost ( &SD_Mutex,
//										OS_OPT_POST_NONE,
//										&os_err);
			
			temp += bw;
			RxCpltFlag = 0;
		}
		
		if( RxHalfCpltFlag == 1 )
		{
//			OSMutexPend ( &SD_Mutex,
//										0,
//										OS_OPT_PEND_BLOCKING,
//										&ts,
//										&os_err);
			
			if( f_write( &file_object, (unsigned char *)audio_output_buffer, 4608, &bw ) != FR_OK ) 
			{
				f_close( &file_object );
				
//				OSMutexPost ( &SD_Mutex,
//											OS_OPT_POST_NONE,
//											&os_err);
				
				BSP_AUDIO_IN_Stop(CODEC_PDWN_SW);
				return 0;
			}
			
//			OSMutexPost ( &SD_Mutex,
//										OS_OPT_POST_NONE,
//										&os_err);
			temp += bw;
			RxHalfCpltFlag = 0;
		}
		
		if( RecordInfo.state == OK )
		{
			RecordInfo.time_count++;
			
//			if( RecordInfo.time_count == 10000 ) RecordInfo.state = Stop;
		}
		else if( RecordInfo.state == Pause )
		{
			BSP_AUDIO_IN_Pause( );
			while( RecordInfo.state == Pause )
			{
				Audio_Delay_1ms( 1 );
			}
		}
		else if( RecordInfo.state == Resume )
		{
			BSP_AUDIO_IN_Resume( );
			RecordInfo.state = OK;
		}
		else 
		{
			BSP_AUDIO_IN_Stop(CODEC_PDWN_SW);
			
			RecordInfo.state = OK;
			
			RecordInfo.time =  RecordInfo.time_count / 1000;
			
			buffer[3] = temp >> 24;
			buffer[2] = ( temp >> 16 ) & 0x00ff;
			buffer[1] = ( temp >> 8 ) & 0x0000ff;
			buffer[0] = temp & 0x000000ff; 
			
//			OSMutexPend ( &SD_Mutex,
//										0,
//										OS_OPT_PEND_BLOCKING,
//										&ts,
//										&os_err);
			
			RecordInfo.FileSize = f_tell( &file_object );
			
			f_lseek( &file_object, 40 );
			
			if( f_write( &file_object, buffer, 4, &bw ) != FR_OK ) 
			{
				f_close( &file_object );
				
//				OSMutexPost ( &SD_Mutex,
//											OS_OPT_POST_NONE,
//											&os_err);
				
				return 0;
			}
			
			buffer[3] = (RecordInfo.FileSize-8) >> 24;
			buffer[2] = ( (RecordInfo.FileSize-8) >> 16 ) & 0x00ff;
			buffer[1] = ( (RecordInfo.FileSize-8) >> 8 ) & 0x0000ff;
			buffer[0] = (RecordInfo.FileSize-8) & 0x000000ff;
			
			f_lseek( &file_object, 4 );
			
			if( f_write( &file_object, buffer, 4, &bw ) != FR_OK ) 
			{
				f_close( &file_object );
//				
//				OSMutexPost ( &SD_Mutex,
//											OS_OPT_POST_NONE,
//											&os_err);
				
				return 0;
			}
			
			f_close( &file_object );
			
//			OSMutexPost ( &SD_Mutex,
//										OS_OPT_POST_NONE,
//										&os_err);
			
//			printf("\r\n 录音结束");
			
			return 1;
		}
	}
	return 0;
}


	
	
		

