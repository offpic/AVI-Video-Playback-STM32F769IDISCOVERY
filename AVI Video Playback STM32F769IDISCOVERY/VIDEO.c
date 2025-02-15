#include "stm32f7xx.h"


#include "PC_COM.h"
#include "SDRAM.h"
#include "LCD_Driver.h"
#include "SD_Driver.h"
#include "Audio_Driver.h"
#include "JpegDecode.h"
#include "VIDEO.h"

#include "ff.h"	
#include "ff_gen_drv.h"

#include "string.h"





extern JPEG_Data_BufferTypeDef Jpeg_IN_BufferTab[2];

extern JPEG_HandleTypeDef hjpeg;
extern JPEG_ConfTypeDef jpegInfo;


extern LTDC_HandleTypeDef hltdc;
extern DMA2D_HandleTypeDef hdma2d;
extern unsigned short int LCD_With;											
extern unsigned short int LCD_Heigh;


extern volatile unsigned char TxCpltFlag;
extern volatile unsigned char TxHalfCpltFlag;
extern volatile unsigned char AudioOutFlag;
extern unsigned short int audio_output_buffer[4608];


extern volatile unsigned int ms_count;															//ms������




FIL AVI;
DWORD clmt[2400]; 																									 // Cluster link map table buffer 


_avimainheader AVI_MainHeader;
_avistreamheader AVI_StreamHeader[2];
_avi_index AVI_Index;
_avi_audio_info AVI_Audio_Info;
_list LIST[2];
_avi_player_info AVI_Player_Info;																		//AVI��������Ϣ




volatile unsigned char AVI_IN_BUF1_State;														//֡���뻺����1��״̬
volatile unsigned int AVI_IN_BUF1_Len;															//֡���뻺����1�����ݴ�С
volatile unsigned char AVI_IN_BUF2_State;														//֡���뻺����2��״̬
volatile unsigned int AVI_IN_BUF2_Len;															//֡���뻺����2�����ݴ�С
volatile unsigned char AVI_BUF_FLAG;																//֡���뻺�������л���־

volatile unsigned int AudioFrameTotal;															//AVI�ļ���Ƶ֡����
volatile unsigned int AudioFrameAddr;																//AVI�ļ���Ƶ֡��ַ
volatile unsigned int AudioCurrenFrameNum;													//AVI�ļ���ǰ��Ƶ֡���

volatile unsigned int VideoFrameAddr;																//AVI�ļ���Ƶ֡��ַ
volatile unsigned int VideoCurrenFrameNum;													//AVI�ļ���ǰ��Ƶ֡���




unsigned char AVI_BUF[16*1024];																			//AVI���ݻ���







/**
* @brief  ��ʱ��������ʱXms
* @note   
* @retval None
*/
void Video_Delay_1ms( unsigned int ms )
{  
//	OS_ERR   p_err;
	
	unsigned int i, j;
	
	for( i=0; i<ms; i++ )
	for( j=0; j<100000; j++ );
	
//	OSTimeDly ( ms, OS_OPT_TIME_DLY, &p_err );
}





/**
* @brief  ��ʱ��������ʱXms
* @note   
* @retval None
*/
void Video_Delay_1us( unsigned int ms )
{  
	unsigned int i, j;
	
	for( i=0; i<ms; i++ )
	for( j=0; j<100; j++ );
}





/**
* @brief  ����AVI��Ƶ֡���ݵ����뻺����
* @param  file:  �ļ�Ŀ��
* @param  buf:  �������
* @param  len:  ��ȡ���ݳ���
* @retval 1���ɹ�		0��ʧ��
*/
unsigned char Load_AVIFrame_Data( FIL *file, unsigned char *buf, unsigned int len )
{
	unsigned int num, addr = 0;
	unsigned int temp;
	
	
	while( len > 0 )
	{
		if( len >= 16*1024 ) num = 16*1024;
		else num = len;
		
		if( f_read( file, AVI_BUF, num, &temp ) != FR_OK ) return 0;
		memcpy( buf+addr, AVI_BUF, temp );
		
		addr += temp;
		len -= temp;
	}
	
	return 1;
}




/**
* @brief  AVI��������ʼ��
* @param  file:  ָ��Ҫ���ŵ�AVI�ļ�
* @retval 1���ɹ�		0��ʧ��
*/
unsigned char AVI_Init( void )
{
	unsigned char buf[16];
	unsigned int temp1, addr1, addr2;
	unsigned int temp2, count;
	unsigned int offset;
	signed int i;
	
	
	memset( &AVI_Player_Info, 0, sizeof( _avi_player_info ) );
	memset( &AVI_MainHeader, 0, sizeof( _avimainheader ) );
	memset( &AVI_StreamHeader[0], 0, sizeof( _avistreamheader ) );
	memset( &AVI_Index, 0, sizeof( _avi_index ) );
	memset( &LIST[0], 0, sizeof( _list ) );
	memset( &LIST[1], 0, sizeof( _list ) );
	memset( &AVI_Audio_Info, 0, sizeof( _avi_audio_info ) );
	AudioFrameTotal = 0;
	AudioCurrenFrameNum =0;
	AudioFrameAddr = 0;
	VideoFrameAddr = 0;
	VideoCurrenFrameNum = 0;
	
	
	temp1 = f_size( &AVI );
	
	if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;
	if( memcmp( buf, (unsigned char *)"RIFF", 4 ) != 0 ) return 0;				//��ѯ�Ƿ����RIFF�ֶ�
	
	if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;
	temp2 = buf[3];
	temp2 = ( temp2 << 8 ) | buf[2];
	temp2 = ( temp2 << 8 ) | buf[1];
	temp2 = ( ( temp2 << 8 ) | buf[0] ) + 8;
	if( temp2 != temp1 ) return 0;																				//�Ƚ��ļ���С
	AVI_Player_Info.FileSize = temp1;
	
	if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;
	if( memcmp( buf, (unsigned char *)"AVI ", 4 ) != 0 ) return 0;				//��ѯ�ǲ���AVI�ļ�
	
	if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;
	if( memcmp( buf, (unsigned char *)"LIST", 4 ) != 0 ) return 0;				//��ѯ�ǲ��ǳ���LIST��hdrl���ֶ�				
	
	if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;								//����LIST��hdrl���ֶ����ݽṹ��С
	temp2 = buf[3];
	temp2 = ( temp2 << 8 ) | buf[2];
	temp2 = ( temp2 << 8 ) | buf[1];
	temp2 = ( temp2 << 8 ) | buf[0];
	LIST[0].offaddr = 0X0C;
	LIST[0].size = temp2;
	
	if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;
	if( memcmp( buf, (unsigned char *)"hdrl", 4 ) != 0 ) return 0;				//��ѯ�ǲ��ǳ���LIST�ֶε������ǲ���hdrl

	if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;
	if( memcmp( buf, (unsigned char *)"avih", 4 ) != 0 ) return 0;				//��ѯ�ǲ��ǳ���avih�ֶ�
	offset = 0x18;
	
	if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;								//����avih��Ĵ�С
	temp2 = buf[3];
	temp2 = ( temp2 << 8 ) | buf[2];
	temp2 = ( temp2 << 8 ) | buf[1];
	temp2 = ( temp2 << 8 ) | buf[0];
	AVI_MainHeader.cb = temp2;
	offset = offset + temp2 + 8;
	
	if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;								//��Ƶ֡���ʱ�䣬��usΪ��λ
	temp2 = buf[3];
	temp2 = ( temp2 << 8 ) | buf[2];
	temp2 = ( temp2 << 8 ) | buf[1];
	temp2 = ( temp2 << 8 ) | buf[0];
	AVI_MainHeader.dwMicroSecPerFrame = temp2;
	
	if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;								//AVI�ļ������������
	temp2 = buf[3];
	temp2 = ( temp2 << 8 ) | buf[2];
	temp2 = ( temp2 << 8 ) | buf[1];
	temp2 = ( temp2 << 8 ) | buf[0];
	AVI_MainHeader.dwMaxBytesPerSec = temp2;
	
	if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;								//������������
	temp2 = buf[3];
	temp2 = ( temp2 << 8 ) | buf[2];
	temp2 = ( temp2 << 8 ) | buf[1];
	temp2 = ( temp2 << 8 ) | buf[0];
	AVI_MainHeader.dwPaddingGranularity = temp2;
	
	if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;								//AVI�ļ���ȫ�ֱ�־���Ƿ����������
	temp2 = buf[3];
	temp2 = ( temp2 << 8 ) | buf[2];
	temp2 = ( temp2 << 8 ) | buf[1];
	temp2 = ( temp2 << 8 ) | buf[0];
	if( temp2 != 0x00000910 ) return 0;
	AVI_MainHeader.dwFlags = temp2;
	
	if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;								//��Ƶ֡����
	temp2 = buf[3];
	temp2 = ( temp2 << 8 ) | buf[2];
	temp2 = ( temp2 << 8 ) | buf[1];
	temp2 = ( temp2 << 8 ) | buf[0];
	if( temp2 == 0 ) return 0;
	AVI_MainHeader.dwTotalFrames = temp2;
	
	if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;								//������ʽָ����ʼ֡��
	temp2 = buf[3];
	temp2 = ( temp2 << 8 ) | buf[2];
	temp2 = ( temp2 << 8 ) | buf[1];
	temp2 = ( temp2 << 8 ) | buf[0];
	AVI_MainHeader.dwInitialFrames = temp2;
	
	if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;								//���ļ�������������
	temp2 = buf[3];
	temp2 = ( temp2 << 8 ) | buf[2];
	temp2 = ( temp2 << 8 ) | buf[1];
	temp2 = ( temp2 << 8 ) | buf[0];
	if( temp2 > 2 || temp2 == 0  ) return 0;
	AVI_MainHeader.dwStreams = temp2;
	
	if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;								//�����ȡ���ļ��Ļ����С
	temp2 = buf[3];
	temp2 = ( temp2 << 8 ) | buf[2];
	temp2 = ( temp2 << 8 ) | buf[1];
	temp2 = ( temp2 << 8 ) | buf[0];
	AVI_MainHeader.dwSuggestedBufferSize = temp2;
	
	if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;								//��Ƶͼ��Ŀ�ȣ�������Ϊ��λ��
	temp2 = buf[3];
	temp2 = ( temp2 << 8 ) | buf[2];
	temp2 = ( temp2 << 8 ) | buf[1];
	temp2 = ( temp2 << 8 ) | buf[0];
	if( temp2 > 800 || temp2 == 0 ) return 0;
	AVI_MainHeader.dwWidth = temp2;
	AVI_Player_Info.ImageWith = temp2;
	
	if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;								//��Ƶͼ��ĸ߶ȣ�������Ϊ��λ��
	temp2 = buf[3];
	temp2 = ( temp2 << 8 ) | buf[2];
	temp2 = ( temp2 << 8 ) | buf[1];
	temp2 = ( temp2 << 8 ) | buf[0];
	if( temp2 > 480 || temp2 == 0 ) return 0;
	AVI_MainHeader.dwHeight = temp2;
	AVI_Player_Info.ImageHeigh = temp2;
	
	if( f_lseek(&AVI, offset) != FR_OK ) return 0;
	
	
	for( i=0; i<AVI_MainHeader.dwStreams; i++ )
	{
		if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;								//��ѯ�ǲ��ǳ���LIST��strl���ֶ�
		if( memcmp( buf, (unsigned char *)"LIST", 4 ) != 0 ) return 0;	
		
		if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;
		temp2 = buf[3];
		temp2 = ( temp2 << 8 ) | buf[2];
		temp2 = ( temp2 << 8 ) | buf[1];
		temp2 = ( temp2 << 8 ) | buf[0];
		offset = offset + 8 + temp2;
		
		if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;
		if( memcmp( buf, (unsigned char *)"strl", 4 ) != 0 ) return 0;	
		
		if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;								//��ѯ�ǲ��ǳ���strh�ֶ�
		if( memcmp( buf, (unsigned char *)"strh", 4 ) != 0 ) return 0;
		
		if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;								//����strh��Ĵ�С
		temp1 = buf[3];
		temp1 = ( temp2 << 8 ) | buf[2];
		temp1 = ( temp2 << 8 ) | buf[1];
		temp1 = ( temp2 << 8 ) | buf[0];

		if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;								//��������
		if( memcmp( buf, (unsigned char *)"vids", 4 ) == 0 )
		{
			AVI_Player_Info.VideoStreamFlag = 1;
			AVI_StreamHeader[0].cb = temp1;
			memcpy( AVI_StreamHeader[0].fccType, (unsigned char *)"vids", 4 );
			
			if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;							//ָ����Ƶ������������
			if( memcmp( buf, (unsigned char *)"MJPG", 4 ) != 0 ) return 0;
			memcpy( AVI_StreamHeader[0].fccHandler, (unsigned char *)"MJPG", 4 );
		
			if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;							//�Ƿ�����������������ɫ���Ƿ�仯
			temp2 = buf[3];
			temp2 = ( temp2 << 8 ) | buf[2];
			temp2 = ( temp2 << 8 ) | buf[1];
			temp2 = ( temp2 << 8 ) | buf[0];
			AVI_StreamHeader[0].dwFlags = temp2;
			
			if( f_read( &AVI, buf, 2, &temp2 ) != FR_OK ) return 0;							//�������ȼ������ж����ͬ���͵���ʱ�����ȼ���ߵ�ΪĬ������
			temp2 = buf[1];
			temp2 = ( temp2 << 8 ) | buf[0];
			AVI_StreamHeader[0].wPriority = temp2;
			
			if( f_read( &AVI, buf, 2, &temp2 ) != FR_OK ) return 0;							
			temp2 = buf[1];
			temp2 = ( temp2 << 8 ) | buf[0];
			AVI_StreamHeader[0].wLanguage = temp2;
			
			if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;							//����ʽָ����ʼ֡��
			temp2 = buf[3];
			temp2 = ( temp2 << 8 ) | buf[2];
			temp2 = ( temp2 << 8 ) | buf[1];
			temp2 = ( temp2 << 8 ) | buf[0];
			AVI_StreamHeader[0].dwInitialFrames = temp2;
			
			if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;							//�����ʹ�õ�ʱ��߶�
			temp2 = buf[3];
			temp2 = ( temp2 << 8 ) | buf[2];
			temp2 = ( temp2 << 8 ) | buf[1];
			temp2 = ( temp2 << 8 ) | buf[0];
			AVI_StreamHeader[0].dwScale = temp2;
			
			if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;							//
			temp2 = buf[3];
			temp2 = ( temp2 << 8 ) | buf[2];
			temp2 = ( temp2 << 8 ) | buf[1];
			temp2 = ( temp2 << 8 ) | buf[0];
			if( temp2 == 0 ) return 0;
			AVI_StreamHeader[0].dwRate = temp2;
			
			if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;							//���Ŀ�ʼʱ��
			temp2 = buf[3];
			temp2 = ( temp2 << 8 ) | buf[2];
			temp2 = ( temp2 << 8 ) | buf[1];
			temp2 = ( temp2 << 8 ) | buf[0];
			AVI_StreamHeader[0].dwStart = temp2;
			
			if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;							//���ĳ���
			temp2 = buf[3];
			temp2 = ( temp2 << 8 ) | buf[2];
			temp2 = ( temp2 << 8 ) | buf[1];
			temp2 = ( temp2 << 8 ) | buf[0];
			AVI_StreamHeader[0].dwLength = temp2;
			
			if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;							//��ȡ��������ݽ���ʹ�õĻ����С
			temp2 = buf[3];
			temp2 = ( temp2 << 8 ) | buf[2];
			temp2 = ( temp2 << 8 ) | buf[1];
			temp2 = ( temp2 << 8 ) | buf[0];
			AVI_StreamHeader[0].dwSuggestedBufferSize= temp2;
			
			if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;							//�����ݵ�����ָ��
			temp2 = buf[3];
			temp2 = ( temp2 << 8 ) | buf[2];
			temp2 = ( temp2 << 8 ) | buf[1];
			temp2 = ( temp2 << 8 ) | buf[0];
			AVI_StreamHeader[0].dwQuality= temp2;
			
			if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;	

			if( f_read( &AVI, buf, 2, &temp2 ) != FR_OK ) return 0;							//ָ�����������Ƶ������������������Ƶ��������ʾ��λ��
			temp2 = buf[1];
			temp2 = ( temp2 << 8 ) | buf[0];
			AVI_StreamHeader[0]._rcFrame.left = temp2;
			if( f_read( &AVI, buf, 2, &temp2 ) != FR_OK ) return 0;
			temp2 = buf[1];
			temp2 = ( temp2 << 8 ) | buf[0];
			AVI_StreamHeader[0]._rcFrame.top = temp2;
			if( f_read( &AVI, buf, 2, &temp2 ) != FR_OK ) return 0;
			temp2 = buf[1];
			temp2 = ( temp2 << 8 ) | buf[0];
			AVI_StreamHeader[0]._rcFrame.right = temp2;
			if( f_read( &AVI, buf, 2, &temp2 ) != FR_OK ) return 0;
			temp2 = buf[1];
			temp2 = ( temp2 << 8 ) | buf[0];
			AVI_StreamHeader[0]._rcFrame.bottom = temp2;
		}
		else if( memcmp( buf, (unsigned char *)"auds", 4 ) == 0 )
		{
			AVI_Player_Info.AudioStreamFlag = 1;
			AVI_StreamHeader[1].cb = temp1;
			memcpy( AVI_StreamHeader[1].fccType, (unsigned char *)"auds", 4 );
			
			if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;							//ָ����Ƶ������������
			memcpy( AVI_StreamHeader[1].fccHandler, buf, 4 );
		
			if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;							//�Ƿ�������������
			temp2 = buf[3];
			temp2 = ( temp2 << 8 ) | buf[2];
			temp2 = ( temp2 << 8 ) | buf[1];
			temp2 = ( temp2 << 8 ) | buf[0];
			AVI_StreamHeader[1].dwFlags = temp2;
			
			if( f_read( &AVI, buf, 2, &temp2 ) != FR_OK ) return 0;							//�������ȼ������ж����ͬ���͵���ʱ�����ȼ���ߵ�ΪĬ������
			temp2 = buf[1];
			temp2 = ( temp2 << 8 ) | buf[0];
			AVI_StreamHeader[1].wPriority = temp2;
			
			if( f_read( &AVI, buf, 2, &temp2 ) != FR_OK ) return 0;							
			temp2 = buf[1];
			temp2 = ( temp2 << 8 ) | buf[0];
			AVI_StreamHeader[1].wLanguage = temp2;
			
			if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;							
			
			if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;							//�����ʹ�õ�ʱ��߶�
			temp2 = buf[3];
			temp2 = ( temp2 << 8 ) | buf[2];
			temp2 = ( temp2 << 8 ) | buf[1];
			temp2 = ( temp2 << 8 ) | buf[0];
			AVI_StreamHeader[1].dwScale = temp2;
			
			if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;							//��Ƶ������
			temp2 = buf[3];
			temp2 = ( temp2 << 8 ) | buf[2];
			temp2 = ( temp2 << 8 ) | buf[1];
			temp2 = ( temp2 << 8 ) | buf[0];
			if( temp2 == 0 ) return 0;
			AVI_StreamHeader[1].dwRate = temp2;
			
			if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;							//���Ŀ�ʼʱ��
			temp2 = buf[3];
			temp2 = ( temp2 << 8 ) | buf[2];
			temp2 = ( temp2 << 8 ) | buf[1];
			temp2 = ( temp2 << 8 ) | buf[0];
			AVI_StreamHeader[1].dwStart = temp2;
			
			if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;							//���ĳ���
			temp2 = buf[3];
			temp2 = ( temp2 << 8 ) | buf[2];
			temp2 = ( temp2 << 8 ) | buf[1];
			temp2 = ( temp2 << 8 ) | buf[0];
			AVI_StreamHeader[1].dwLength = temp2;
			
			if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;							//��ȡ��������ݽ���ʹ�õĻ����С
			temp2 = buf[3];
			temp2 = ( temp2 << 8 ) | buf[2];
			temp2 = ( temp2 << 8 ) | buf[1];
			temp2 = ( temp2 << 8 ) | buf[0];
			AVI_StreamHeader[1].dwSuggestedBufferSize= temp2;
			
			if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;							//�����ݵ�����ָ��
			temp2 = buf[3];
			temp2 = ( temp2 << 8 ) | buf[2];
			temp2 = ( temp2 << 8 ) | buf[1];
			temp2 = ( temp2 << 8 ) | buf[0];
			AVI_StreamHeader[1].dwQuality= temp2;
			
			if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;	
			temp2 = buf[3];
			temp2 = ( temp2 << 8 ) | buf[2];
			temp2 = ( temp2 << 8 ) | buf[1];
			temp2 = ( temp2 << 8 ) | buf[0];
			AVI_StreamHeader[1].dwSampleSize= temp2;
			
			if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;	

			if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;
			

			if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;							//��ѯ�Ƿ����strf�ֶ�
			if( memcmp( buf, (unsigned char *)"strf", 4 ) != 0 ) return 0;

			if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;
			
			if( f_read( &AVI, buf, 2, &temp2 ) != FR_OK ) return 0;
			temp2 = buf[1];
			temp2 = ( temp2 << 8 ) | buf[0];
			if( temp2 != 0x0001 ) return 0;
			AVI_Audio_Info.wFormatTag = temp2;
			
			if( f_read( &AVI, buf, 2, &temp2 ) != FR_OK ) return 0;
			temp2 = buf[1];
			temp2 = ( temp2 << 8 ) | buf[0];
			AVI_Audio_Info.nChannels = temp2;
			
			if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;
			temp2 = buf[3];
			temp2 = ( temp2 << 8 ) | buf[2];
			temp2 = ( temp2 << 8 ) | buf[1];
			temp2 = ( temp2 << 8 ) | buf[0];
			AVI_Audio_Info.nSamplesPerSec = temp2;
			
			if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;
			temp2 = buf[3];
			temp2 = ( temp2 << 8 ) | buf[2];
			temp2 = ( temp2 << 8 ) | buf[1];
			temp2 = ( temp2 << 8 ) | buf[0];
			AVI_Audio_Info.nAvgBytesPerSec = temp2;
			
			if( f_read( &AVI, buf, 2, &temp2 ) != FR_OK ) return 0;
			temp2 = buf[1];
			temp2 = ( temp2 << 8 ) | buf[0];
			AVI_Audio_Info.nBlockAlign = temp2;
			
			if( f_read( &AVI, buf, 2, &temp2 ) != FR_OK ) return 0;
			temp2 = buf[1];
			temp2 = ( temp2 << 8 ) | buf[0];
			AVI_Audio_Info.biSize = temp2;
		}
		else return 0;
		
		if( i == 0 ) 
		{
			if( f_lseek(&AVI, offset) != FR_OK ) return 0;
		}
	}
	
	if( AVI_MainHeader.dwStreams == 1 && AVI_StreamHeader[0].cb == 0 ) return 0;
	
	
	if( f_lseek( &AVI, LIST[0].offaddr + 8 + LIST[0].size ) != FR_OK ) return 0;
	
	
	if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;
	if( memcmp( buf, (unsigned char *)"LIST", 4 ) != 0 ) return 0;				//��ѯ�ǲ��ǳ���LIST��INF0���ֶ�		
	
	if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;								//����LIST��INFO���ֶ����ݽṹ��С
	temp2= buf[3];
	temp2 = ( temp2 << 8 ) | buf[2];
	temp2 = ( temp2 << 8 ) | buf[1];
	temp2 = ( temp2 << 8 ) | buf[0];
	offset = temp2 + 8 + LIST[0].offaddr + 8 + LIST[0].size;
	
	if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;
	if( memcmp( buf, (unsigned char *)"INFO", 4 ) != 0 ) return 0;
	
	
	if( f_lseek( &AVI, offset ) != FR_OK ) return 0;
	
	
	if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;
	if( memcmp( buf, (unsigned char *)"JUNK", 4 ) != 0 ) return 0;				//��ѯ�ǲ��ǳ���JUNK�ֶ�

	if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;								//����JUNK�ֶ����ݽṹ��С
	temp2= buf[3];
	temp2 = ( temp2 << 8 ) | buf[2];
	temp2 = ( temp2 << 8 ) | buf[1];
	temp2 = ( temp2 << 8 ) | buf[0];
	offset = temp2 + 8 + offset;
	
	
	if( f_lseek( &AVI, offset ) != FR_OK ) return 0;
	
	
	if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;
	if( memcmp( buf, (unsigned char *)"LIST", 4 ) != 0 ) return 0;				//��ѯ�ǲ��ǳ���LIST��movi���ֶ�
	
	
	if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;								//����LIST��movi���ֶ����ݽṹ��С
	temp1= buf[3];
	temp1 = ( temp1 << 8 ) | buf[2];
	temp1 = ( temp1 << 8 ) | buf[1];
	temp1 = ( temp1 << 8 ) | buf[0];
	
	if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;
	if( memcmp( buf, (unsigned char *)"movi", 4 ) != 0 ) return 0;				//��ѯ�ǲ��ǳ���LIST��movi���ֶ�
	LIST[1].offaddr = offset;
	LIST[1].size = temp1;
	
	
	offset = offset + 8 + LIST[1].size;
	if( f_lseek( &AVI, offset ) != FR_OK ) return 0;

	
	if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;
	if( memcmp( buf, (unsigned char *)"idx1", 4 ) != 0 ) return 0;				//��ѯ�ǲ��ǳ���idx1�ֶ�

	if( f_read( &AVI, buf, 4, &temp2 ) != FR_OK ) return 0;								//����idx1�ֶ����ݽṹ��С
	temp2= buf[3];
	temp2 = ( temp2 << 8 ) | buf[2];
	temp2 = ( temp2 << 8 ) | buf[1];
	temp2 = ( temp2 << 8 ) | buf[0];
	AVI_Index.offaddr = offset;
	AVI_Index.size = temp2;
	
	
	addr1 = AVI_Frame_Index;
	addr2 = AVI_Audio_Index;
//	flag = 0;
//	count = 0;
	count = AVI_Index.size;
	
	while( count > 0 )
	{
		if( f_read( &AVI, buf, 16, &temp2 ) != FR_OK ) return 0;
		
		if( memcmp( buf, (unsigned char *)"00dc", 4 ) == 0 )
		{
			temp1= buf[7];
			temp1 = ( temp1 << 8 ) | buf[6];
			temp1 = ( temp1 << 8 ) | buf[5];
			temp1 = ( temp1 << 8 ) | buf[4];
			if( temp1 != 0 ) 
			{
				temp1= buf[11];
				temp1 = ( temp1 << 8 ) | buf[10];
				temp1 = ( temp1 << 8 ) | buf[9];
				temp1 = ( temp1 << 8 ) | buf[8];
				temp2 = temp1 + LIST[1].offaddr + 16;
				SDRAM_Write_32B( addr1, &temp2, 1 );
				addr1 += 4;
				
				temp1= buf[15];
				temp1 = ( temp1 << 8 ) | buf[14];
				temp1 = ( temp1 << 8 ) | buf[13];
				temp1 = ( temp1 << 8 ) | buf[12];
				SDRAM_Write_32B( addr1, &temp1, 1 );
				addr1 += 4;
			}
		}
		else if( memcmp( buf, (unsigned char *)"01wb", 4 ) == 0 )
		{
			temp1= buf[7];
			temp1 = ( temp1 << 8 ) | buf[6];
			temp1 = ( temp1 << 8 ) | buf[5];
			temp1 = ( temp1 << 8 ) | buf[4];
			if( temp1 != 0 ) 
			{
				temp1= buf[11];
				temp1 = ( temp1 << 8 ) | buf[10];
				temp1 = ( temp1 << 8 ) | buf[9];
				temp1 = ( temp1 << 8 ) | buf[8];
				temp2 = temp1 + LIST[1].offaddr + 16;
				SDRAM_Write_32B( addr2, &temp2, 1 );
				addr2 += 4;
				
				AudioFrameTotal++;
			}
		}
		else return 0;
		
		count -= 16;
	}

	return 1;
}






/**
* @brief  AVI��Ƶ��������ʼ��
* @param  Volume: Initial volume level (from 0 (Mute) to 100 (Max))
* @param  AudioFreq: Audio frequency used to play the audio stream.
* @retval 1���ɹ�		0��ʧ��
*/
unsigned char AVI_Audio_Init( unsigned char Volume, unsigned int AudioFreq )
{
	if(BSP_AUDIO_OUT_Init( OUTPUT_DEVICE_HEADPHONE, Volume,  AudioFreq ) != AUDIO_OK ) return 0;
	
	BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);
	
	return 1;
}





/**
* @brief  ��Ƶ֡����
* @param  Xpos:  ��LCD����ʾ����ʼλ��--x����
* @param  Ypos:  ��LCD����ʾ����ʼλ��--y����
* @retval 1���ɹ�		0��ʧ��
*/
unsigned char Video_Frame_Play( unsigned short int Xpos, unsigned short int Ypos )
{
	static unsigned char Frame_State = 0;
	static unsigned int time1 = 0;
	static float time2 = 0;
	float time3;
	unsigned char flag = 0;
	unsigned int offset, temp;
	
	
	switch( Frame_State )
	{
		case 0:
		{
			time1 = ms_count;
			
			if( AVI_BUF_FLAG == 0 )
			{
				if( avi_video_decode( (unsigned char *)(AVI_IN_BUF1 + SDRAM_BANK1_ADDR), AVI_IN_BUF1_Len ) == 0 ) flag = 1;
				else Frame_State = 1;
			}
			else
			{
				if( avi_video_decode( (unsigned char *)(AVI_IN_BUF2 + SDRAM_BANK1_ADDR), AVI_IN_BUF2_Len ) == 0 ) flag = 1;
				else Frame_State = 1;
			}	
		}
		break;
		
		case 1:
		{
			if( Check_JPEG_Decode( &AVI ) == 0 )
			{
				if( AVI_IN_BUF1_State == 0 && VideoCurrenFrameNum<AVI_MainHeader.dwTotalFrames-1 )
				{
					SDRAM_Read_32B( VideoFrameAddr, &offset, 1 );
					VideoFrameAddr += 4;
					
					SDRAM_Read_32B( VideoFrameAddr, &temp, 1 );
					VideoFrameAddr += 4;
					
					if( f_lseek( &AVI, offset ) != FR_OK )
					{
						flag = 1;
						Frame_State = 0;
						break;
					}
					
					if( Load_AVIFrame_Data( &AVI, (unsigned char *)(AVI_IN_BUF1 + SDRAM_BANK1_ADDR), temp ) == 0 )
					{
						flag = 1;
						Frame_State = 0;
						break;
					}
					
					AVI_IN_BUF1_State = 1;
					AVI_IN_BUF1_Len = temp;
				}
				else if( AVI_IN_BUF2_State == 0 && VideoCurrenFrameNum<AVI_MainHeader.dwTotalFrames-1 )
				{
					SDRAM_Read_32B( VideoFrameAddr, &offset, 1 );
					VideoFrameAddr += 4;
					
					SDRAM_Read_32B( VideoFrameAddr, &temp, 1 );
					VideoFrameAddr += 4;
					
					if( f_lseek( &AVI, offset ) != FR_OK )
					{
						flag = 1;
						Frame_State = 0;
						break;
					}
					
					if( Load_AVIFrame_Data( &AVI, (unsigned char *)(AVI_IN_BUF2 + SDRAM_BANK1_ADDR), temp ) == 0 )
					{
						flag = 1;
						Frame_State = 0;
						break;
					}
					
					AVI_IN_BUF2_State = 1;
					AVI_IN_BUF2_Len = temp;
				}
			}
			else Frame_State = 2;
		}
		break;
		
		case 2:
		{
			Display_Image( Xpos, Ypos );
		
			if( AVI_BUF_FLAG == 0 )
			{
				AVI_IN_BUF1_State = 0;
				AVI_IN_BUF1_Len = 0;
				
				AVI_BUF_FLAG = 1;
			}
			else
			{
				AVI_IN_BUF2_State = 0;
				AVI_IN_BUF2_Len = 0;
				
				AVI_BUF_FLAG = 0;
			}
			
			Frame_State = 3;
		}
		break;
		
		case 3:
		{
			time3 = 1000.0 / AVI_Player_Info.VideoFrameRate;
			if( (ms_count - time1) >= (time3-time2) )
			{
				if( ms_count - time1 > ( time3-time2) ) time2 = ms_count - time1 - time3 + time2;
				else time2 = 0;
				
				Frame_State = 0;
				
				VideoCurrenFrameNum++;
				
				if( VideoCurrenFrameNum == AVI_MainHeader.dwTotalFrames )
				{
					time1 = 0;
					time2 = 0;
				}
			}
		}
		break;
	}
		

	if( flag == 1 ) return 0;
	
	return 1;
}



/**
* @brief  ��Ƶ֡����
* @retval 1���ɹ�		0��ʧ��
*/
unsigned char Audio_Frame_Play( void )
{
	static unsigned char Audio_Frame_State = 0;
	unsigned int offset, temp;
	
	
	switch( Audio_Frame_State )
	{
		case 0:
		{
			TxCpltFlag = 0;
			TxHalfCpltFlag = 0;
		
			BSP_AUDIO_OUT_Play( audio_output_buffer, 8192 );
			AudioOutFlag = 1;
			
			Audio_Frame_State = 1;
		}
		break;
		
		case 1:
		{
			if( AudioOutFlag == 0 )
			{
				Audio_Frame_State = 2;
			}
		}
		break;
		
		case 2:
		{
			AudioCurrenFrameNum++;
			
			AVI_Player_Info.CurrenPlayTime = AudioCurrenFrameNum * AVI_Player_Info.TotalTime / AudioFrameTotal;
					
			if( TxHalfCpltFlag == 1 && AudioCurrenFrameNum < AudioFrameTotal - 1 )
			{
				SDRAM_Read_32B( AudioFrameAddr, &offset, 1 );
				AudioFrameAddr += 4;
				
				if( f_lseek( &AVI, offset ) != FR_OK )	
				{
					Audio_Frame_State = 0;
					BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
					return 0;
				}
				
				if( f_read( &AVI, (unsigned char *)audio_output_buffer, 4096, &temp ) != FR_OK ) 
				{
					Audio_Frame_State = 0;
					BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
					return 0;
				}
				
				AudioOutFlag = 1;
				TxHalfCpltFlag = 0;
			}
			
			if( TxCpltFlag == 1 && AudioCurrenFrameNum < AudioFrameTotal )
			{
				SDRAM_Read_32B( AudioFrameAddr, &offset, 1 );
				AudioFrameAddr += 4;
				
				if( f_lseek( &AVI, offset ) != FR_OK )
				{
					Audio_Frame_State = 0;
					BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
					return 0;
				}
				
				if( f_read( &AVI, (unsigned char *)(audio_output_buffer+2048), 4096, &temp ) != FR_OK ) 
				{
					Audio_Frame_State = 0;
					BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
					return 0;
				}
				
				AudioOutFlag = 1;
				TxCpltFlag = 0;
			}
			
			if( AudioCurrenFrameNum == AudioFrameTotal ) 
			{
				Audio_Frame_State = 0;
				BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
			}
			else Audio_Frame_State = 1;
		}
		break;
	}
	
	return 1;
}



/**
* @brief  ����AVI��Ƶ�ļ�
* @param  file:  ָ��Ҫ���ŵ�AVI�ļ�
* @param  Xpos:  ��LCD����ʾ����ʼλ��--x����
* @param  Ypos:  ��LCD����ʾ����ʼλ��--y���� 
* @param  vol:  	����
* @param  time:  ���ŵ���ʼʱ�䣬����Ϊ��λ
* @retval None
*/
unsigned char AVI_Player( unsigned char *file, unsigned short int Xpos, unsigned short int Ypos, unsigned char vol, unsigned int time )
{
	unsigned int i, temp1, offset1;
	unsigned int count, temp2;
	
	
	if( Xpos > LCD_With || Ypos > LCD_Heigh )
	{
		AVI_Player_Info.ErrorFlag = 1;
		return 0;
	}
	
	if( f_open( &AVI, (const TCHAR*)file,  FA_READ ) != FR_OK )
	{
		AVI_Player_Info.ErrorFlag = 1;
		printf("��ʧ�ܡ�\r\n" );
		return 0;
	}
	
	
	/*	enable fast seek feature	*/
	if( f_lseek(&AVI, 4)!= FR_OK )
	{
		AVI_Player_Info.ErrorFlag = 1;
		f_close( &AVI );
		return 0;
	}
	
	AVI.cltbl = clmt;                       					/* Enable fast seek feature (cltbl != NULL) */
  clmt[0] = 2400;																		/* Set table size */
	
	if( f_lseek(&AVI, CREATE_LINKMAP) != FR_OK )				/* Create CLMT */
	{
		AVI_Player_Info.ErrorFlag = 1;
		f_close( &AVI );
		return 0;
	}
	
	if( f_lseek(&AVI, 0)!= FR_OK )
	{
		AVI_Player_Info.ErrorFlag = 1;
		f_close( &AVI );
		return 0;
	}
	
	if( AVI_Init(  ) == 0 )
	{
		f_close( &AVI );
		AVI_Player_Info.ErrorFlag = 1;
		printf("AVI��������ʼ��ʧ�ܡ�\r\n" );
		return 0;
	}
	
	if( ( Xpos + AVI_Player_Info.ImageWith ) > 800 || ( Ypos + AVI_Player_Info.ImageHeigh ) > 480 )
	{
		f_close( &AVI );
		AVI_Player_Info.ErrorFlag = 1;
		printf("��������\r\n" );
		return 0;
	}
	
	if( AVI_Audio_Init( vol, AVI_Audio_Info.nSamplesPerSec ) != 1 )
	{
		f_close( &AVI );
		AVI_Player_Info.ErrorFlag = 1;
		printf("��Ƶ��ʼ������\r\n" );
		return 0;
	}
	
	
	AVI_Player_Info.StateFlag = 0;
	
	if( AVI_MainHeader.dwStreams == 2 )
	{
		AVI_Player_Info.TotalTime = (AVI_StreamHeader[1].dwLength / AVI_StreamHeader[1].dwRate / AVI_StreamHeader[1].dwScale ) * 1000;
	}
	else AVI_Player_Info.TotalTime = (AVI_StreamHeader[0].dwLength / AVI_StreamHeader[0].dwRate / AVI_StreamHeader[0].dwScale ) * 1000;
	
	if( ( AVI_Player_Info.TotalTime / 1000 ) <= time )
	{
		f_close( &AVI );
		AVI_Player_Info.ErrorFlag = 1;
		return 0;
	}
	AVI_Player_Info.CurrenPlayTime = time * 1000;
	AVI_Player_Info.VideoFrameRate = AVI_StreamHeader[0].dwRate / AVI_StreamHeader[0].dwScale;
	
	
	AVI_IN_BUF1_State = 0;
	AVI_IN_BUF2_State = 0;
	AVI_BUF_FLAG = 0;
	VideoCurrenFrameNum = AVI_Player_Info.CurrenPlayTime / 1000 * AVI_MainHeader.dwTotalFrames * 1000 / AVI_Player_Info.TotalTime;
	VideoFrameAddr = AVI_Frame_Index + 8 * VideoCurrenFrameNum;
	
	if( AVI_Player_Info.AudioStreamFlag == 1 )
	{
		AudioCurrenFrameNum = AVI_Player_Info.CurrenPlayTime * AudioFrameTotal / AVI_Player_Info.TotalTime;
		AudioFrameAddr = AVI_Audio_Index + AudioCurrenFrameNum * 4;
	}
	
	
	for( i=0; i<2; i++ )
	{
		SDRAM_Read_32B( VideoFrameAddr, &offset1, 1 );
		VideoFrameAddr += 4;
		
		SDRAM_Read_32B( VideoFrameAddr, &temp1, 1 );
		VideoFrameAddr += 4;
		
		if( f_lseek( &AVI, offset1 ) != FR_OK )
		{
			f_close( &AVI );
			AVI_Player_Info.ErrorFlag = 1;
			return 0;
		}
		
		if( i == 0 )
		{
			if( Load_AVIFrame_Data( &AVI, (unsigned char *)(AVI_IN_BUF1 + SDRAM_BANK1_ADDR), temp1 ) == 0 )
			{
				f_close( &AVI );
				AVI_Player_Info.ErrorFlag = 1;
				return 0;
			}
		
			AVI_IN_BUF1_State = 1;
			AVI_IN_BUF1_Len = temp1;
		}
		else
		{
			if( Load_AVIFrame_Data( &AVI, (unsigned char *)(AVI_IN_BUF2 + SDRAM_BANK1_ADDR), temp1 ) == 0 )
			{
				f_close( &AVI );
				AVI_Player_Info.ErrorFlag = 1;
				return 0;
			}
		
			AVI_IN_BUF2_State = 1;
			AVI_IN_BUF2_Len = temp1;
		}
	}
	
	
	if( AVI_Player_Info.AudioStreamFlag == 1 )
	{
		for( i=0; i<2; i++ )
		{
			SDRAM_Read_32B( AudioFrameAddr, &offset1, 1 );
			AudioFrameAddr += 4;
			
			if( f_lseek( &AVI, offset1 ) != FR_OK )
			{
				f_close( &AVI );
				AVI_Player_Info.ErrorFlag = 1;
				return 0;
			}
			
			if( i == 0 )
			{
				if( f_read( &AVI, (unsigned char *)audio_output_buffer, 4096, &temp2 ) != FR_OK )
				{
					f_close( &AVI );
					AVI_Player_Info.ErrorFlag = 1;
					return 0;
				}
			}
			else
			{
				if( f_read( &AVI, (unsigned char *)(audio_output_buffer+2048), 4096, &temp2 ) != FR_OK )
				{
					f_close( &AVI );
					AVI_Player_Info.ErrorFlag = 1;
					return 0;
				}
			}
		}
	}
	
	printf("%d\r\n", AudioFrameTotal );
	printf("%d\r\n", AVI_MainHeader.dwTotalFrames );
	count = ms_count;

	while( (AudioCurrenFrameNum + VideoCurrenFrameNum ) < ( AVI_MainHeader.dwTotalFrames + AudioFrameTotal ) )
	{	
		if( VideoCurrenFrameNum < AVI_MainHeader.dwTotalFrames )
		{
			if( Video_Frame_Play( Xpos, Ypos ) == 0 )
			{
				f_close( &AVI );
				BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
				AVI_Player_Info.ErrorFlag = 1;
				return 0;
			}
		}
		
		if( AVI_Player_Info.AudioStreamFlag == 1 && AudioCurrenFrameNum < AudioFrameTotal )
		{
			if( Audio_Frame_Play( ) == 0 )
			{
				f_close( &AVI );
				AVI_Player_Info.ErrorFlag = 1;
				return 0;
			}
		}
	}
	
	BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
	printf("%d\r\n", ms_count - count);
	f_close( &AVI );
	
	return 1;
}



