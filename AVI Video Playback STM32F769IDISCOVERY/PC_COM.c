#include "stm32f7xx_hal.h"
#include <stdio.h>
#include <string.h>

#include "PC_COM.h"






#define UART1_BaudRate      115200  					//串口1的波特率



unsigned char PC_COM_RX_BUF[2048];						//PC串口的接收缓存器
volatile unsigned int PC_COM_RX_Length = 0;		//PC串口的接收缓存器长度
volatile unsigned int PC_COM_RX_Write = 0;		//PC串口接收缓存器的写指针
volatile unsigned int PC_COM_RX_Read = 0;			//PC串口接收缓存器的读指针

unsigned char PC_COM_TX_BUF[1200];						//PC串口的发送缓存器
volatile unsigned int PC_COM_TX_Length = 0;		//PC串口的发送缓存器长度
volatile unsigned int PC_COM_TX_Write = 0;		//PC串口发送缓存器的写指针
volatile unsigned int PC_COM_TX_Read = 0;			//PC串口发送缓存器的读指针

volatile unsigned char PC_COM_TXD_FLAG = 0;		//PC串口发送数据标志


struct DataFrame PC_COM_Frame;


const unsigned int CmdNum = 40;		//定义指令的数量
const unsigned int DataNum = 1024;	//定义数据区存放数据的最大数量

const unsigned char ACKCMD[ ] =	{ 0x0a, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x16 };															//应答指令

const unsigned char USART_SoftUpdateCMD[ ] =	{ 0x0a, 0xa0, 0x01, 0x00, 0x00, 0x01, 0x16 };								//软件升级指令（使用串口升级）
const unsigned char USART_SoftUpdateACKCMD1[ ] =	{ 0x0a, 0xa0, 0x01, 0x00, 0x01, 0X01, 0x03, 0x16 };			//软件升级成功应答指令
const unsigned char USART_SoftUpdateACKCMD2[ ] =	{ 0x0a, 0xa0, 0x01, 0x00, 0x01, 0X00, 0x02, 0x16 };			//软件升级失败应答指令

const unsigned char TFTP_SoftUpdateCMD[ ] =	{ 0x0a, 0xa0, 0x02, 0x00, 0x00, 0x02, 0x16 };									//软件升级指令（使用TFTP升级）
const unsigned char TFTP_SoftUpdateACKCMD1[ ] =	{ 0x0a, 0xa0, 0x02, 0x00, 0x01, 0X01, 0x04, 0x16 };				//软件升级成功应答指令（使用TFTP升级）
const unsigned char TFTP_SoftUpdateACKCMD2[ ] =	{ 0x0a, 0xa0, 0x02, 0x00, 0x01, 0X00, 0x03, 0x16 };				//软件升级失败应答指令（使用TFTP升级）

const unsigned char CS5460_XiaoZhunCMD[ ] =	{ 0x0a, 0xa0, 0x03, 0x00, 0x00, 0x03, 0x16 };									//CS5460校准指令
const unsigned char CS5460_XiaoZhunACKCMD1[ ] =	{ 0x0a, 0xa0, 0x03, 0x00, 0x01, 0X01, 0x05, 0x16 };				//CS5460校准成功应答指令
const unsigned char CS5460_XiaoZhunACKCMD2[ ] =	{ 0x0a, 0xa0, 0x03, 0x00, 0x01, 0X00, 0x04, 0x16 };				//CS5460校准失败应答指令

const unsigned char OS_MessageCMD1[ ] =	{ 0x0a, 0xa0, 0x04, 0x00, 0x01, 0x01, 0x06, 0x16 };								//允许输出UCOS全部任务的信息
const unsigned char OS_MessageCMD2[ ] =	{ 0x0a, 0xa0, 0x04, 0x00, 0x01, 0x00, 0x05, 0x16 };								//禁止输出UCOS全部任务的信息

const unsigned char SendFlash_CMD[ ] =	{ 0x0a, 0xa0, 0x05, 0x00, 0x00, 0x05, 0x16 };											//将本机的程序传送到远端
const unsigned char SendFlashACK_CMD1[ ] =	{ 0x0a, 0xa0, 0x05, 0x00, 0x01, 0X01, 0x07, 0x16 };						//传送成功应答指令
const unsigned char SendFlashACK_CMD2[ ] =	{ 0x0a, 0xa0, 0x05, 0x00, 0x01, 0X00, 0x06, 0x16 };						//传送失败应答指令

const unsigned char TFTP_UploadFileToSDCMD[ ] =	{ 0x0a, 0xa0, 0x06, 0x00, 0x00, 0x06, 0x16 };							//使用TFTP接收客户端传送的文件，并存放在SD卡中
const unsigned char TFTP_UploadFileToSDACKCMD1[ ] =	{ 0x0a, 0xa0, 0x06, 0x00, 0x01, 0X01, 0x08, 0x16 };		//传送成功应答指令（使用TFTP）
const unsigned char TFTP_UploadFileToSDACKCMD2[ ] =	{ 0x0a, 0xa0, 0x06, 0x00, 0x01, 0X00, 0x07, 0x16 };		//传送失败应答指令（使用TFTP）

const unsigned char TFTP_DownloadFileFromSDCMD[ ] =	{ 0x0a, 0xa0, 0x07, 0x00, 0x00, 0x07, 0x16 };						//客户端从TFTP服务器下载存放在SD卡里的文件
const unsigned char TFTP_DownloadFileFromSDACKCMD1[ ] =	{ 0x0a, 0xa0, 0x07, 0x00, 0x01, 0X01, 0x09, 0x16 };	//传送成功应答指令（使用TFTP）
const unsigned char TFTP_DownloadFileFromSDACKCMD2[ ] =	{ 0x0a, 0xa0, 0x07, 0x00, 0x01, 0X00, 0x08, 0x16 };	//传送失败应答指令（使用TFTP）

const unsigned char UpdateAnimation_CMD[ ] =	{ 0x0a, 0xa0, 0x08, 0x00, 0x00, 0x08, 0x16 };									//更新开机动画
const unsigned char UpdateAnimation_CMD1[ ] =	{ 0x0a, 0xa0, 0x08, 0x00, 0x01, 0X01, 0x0A, 0x16 };						//更新开机动画成功应答指令
const unsigned char UpdateAnimation_CMD2[ ] =	{ 0x0a, 0xa0, 0x08, 0x00, 0x01, 0X00, 0x09, 0x16 };						//更新开机动画失败应答指令

const unsigned char UpdateBackground_CMD[ ] =	{ 0x0a, 0xa0, 0x09, 0x00, 0x00, 0x09, 0x16 };									//更新背景图片
const unsigned char UpdateBackground_CMD1[ ] =	{ 0x0a, 0xa0, 0x09, 0x00, 0x01, 0X01, 0x0b, 0x16 };					//更新背景图片成功应答指令
const unsigned char UpdateBackground_CMD2[ ] =	{ 0x0a, 0xa0, 0x09, 0x00, 0x01, 0X00, 0x0a, 0x16 };					//更新背景图片失败应答指令

const unsigned char SendFlashConfigTable_CMD[ ] = { 0x0a, 0xa0, 0x0a, 0x00, 0x00, 0x0a, 0x16 };							//将本机的Flash配置表传送到远端
const unsigned char SendFlashConfigTable_CMD1[ ] =	{ 0x0a, 0xa0, 0x0a, 0x00, 0x01, 0X01, 0x0c, 0x16 };			//将本机的Flash配置表传送到远端成功应答指令
const unsigned char SendFlashConfigTable_CMD2[ ] =	{ 0x0a, 0xa0, 0x0a, 0x00, 0x01, 0X00, 0x0b, 0x16 };			//将本机的Flash配置表传送到远端失败应答指令

const unsigned char UpdateFlashConfigTable_CMD[ ] = { 0x0a, 0xa0, 0x0b, 0x00, 0x00, 0x0b, 0x16 };						//更新Flash配置表
const unsigned char UpdateFlashConfigTable_CMD1[ ] =	{ 0x0a, 0xa0, 0x0b, 0x00, 0x01, 0X01, 0x0b, 0x16 };		//更新Flash配置表成功应答指令
const unsigned char UpdateFlashConfigTable_CMD2[ ] =	{ 0x0a, 0xa0, 0x0b, 0x00, 0x01, 0X00, 0x0c, 0x16 };		//更新Flash配置表失败应答指令

const unsigned char UpdateHanZiFont_CMD[ ] = { 0x0a, 0xa0, 0x0c, 0x00, 0x00, 0x0c, 0x16 };									//更新汉字字体
const unsigned char UpdateHanZiFont_CMD1[ ] =	{ 0x0a, 0xa0, 0x0c, 0x00, 0x01, 0X01, 0x0e, 0x16 };						//更新汉字字体成功应答指令
const unsigned char UpdateHanZiFont_CMD2[ ] =	{ 0x0a, 0xa0, 0x0c, 0x00, 0x01, 0X00, 0x0d, 0x16 };						//更新汉字字体失败应答指令

volatile unsigned char PC_COM_FrameHeadFlag = 0;
volatile unsigned char PC_COM_FrameFlag = 0;



UART_HandleTypeDef huart1;		//USART1句柄




/**
* @brief  配置MCU与PC串口的接口.
* @note   
* @retval None
*/
void PC_COM_GPIO_Config( void )
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	__HAL_RCC_GPIOA_CLK_ENABLE();
	
	GPIO_InitStruct.Pin = GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
	
	HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_10;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
	
	HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );
}






/**
  * @brief  配置串口1的工作模式--PC串口
  *         波特率：115200（默认）
  *         数据位：8位
  *         停止位：1位
  *         奇偶校验：无
  * @param  None
  * @retval None
  */
void USART1_Init( void )
{
	 __USART1_CLK_ENABLE();
	
	huart1.Instance = USART1;
	
	huart1.Init.BaudRate = UART1_BaudRate;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.Mode = UART_MODE_TX;
	
	huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	
	HAL_NVIC_SetPriority( USART1_IRQn, 0, 0 );
	HAL_NVIC_EnableIRQ( USART1_IRQn );

	HAL_UART_Init(&huart1);
}




/**
* @brief  PC串口的初始化函数.
* @note   
* @retval NONE
*/
void PC_COM_Init( void )
{
	PC_COM_GPIO_Config( );
	
	USART1_Init( );
}



/**
  * @brief  配置USART1的接收中断
  * @param  NewState: new state of the USART mute mode.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void PC_COM_RX_INT_Config( FunctionalState NewState )
{
	
	/* Disable the Peripheral */
  __HAL_UART_DISABLE(&huart1);
	
	if( NewState == DISABLE )
	{
		huart1.Instance = USART1;
	
		huart1.Init.BaudRate = UART1_BaudRate;
		huart1.Init.WordLength = UART_WORDLENGTH_8B;
		huart1.Init.StopBits = UART_STOPBITS_1;
		huart1.Init.Parity = UART_PARITY_NONE;
		huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
		huart1.Init.Mode = UART_MODE_TX;
		
		 __HAL_UART_DISABLE_IT( &huart1, UART_IT_RXNE );
	}
	
	if( NewState == ENABLE )
	{
		huart1.Instance = USART1;
	
		huart1.Init.BaudRate = UART1_BaudRate;
		huart1.Init.WordLength = UART_WORDLENGTH_8B;
		huart1.Init.StopBits = UART_STOPBITS_1;
		huart1.Init.Parity = UART_PARITY_NONE;
		huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
		huart1.Init.Mode = UART_MODE_TX_RX;
		
		__HAL_UART_CLEAR_IT( &huart1, UART_CLEAR_RTOF );
		__HAL_UART_CLEAR_IT( &huart1, UART_CLEAR_OREF );
		/* Clear RXNE interrupt flag */
    __HAL_UART_SEND_REQ( &huart1, UART_RXDATA_FLUSH_REQUEST );
		
		__HAL_UART_ENABLE_IT( &huart1, UART_IT_RXNE );
	}
	
	HAL_UART_Init(&huart1);
}




/**
  * @brief  PC串口发送数据
	* @param  buf: 存放要发送的数据
	* @param  num: 发送的字节数
  * @retval None
  */
void PC_COM_Send( unsigned char *buf,  unsigned int num )
{
	while( HAL_UART_Transmit( &huart1, buf, num, 100 ) != HAL_OK );
}





/**
  * @brief  往PC串口发送缓存器里写入数据
	* @param  buffer: 存放写入的数据
	* @param  num: 要写入数据的字节数,最大不超多发送缓存器的大小
	* @retval 1: 成功		0：失败
  */
unsigned char PC_COM_TXDBufferWrite( unsigned char *buffer, unsigned int num )
{
	unsigned int i, len;
	
	if( num > 1200 ) return 0;			//超过发送缓存器的大小，退出
	
	len = 1200 - PC_COM_TX_Length;
	
	if(  num > len ) 								//写入的数据超过发送缓存器剩余空间的大小，退出
	{
		return 0;
	}		
	
	for( i=0; i<num; i++ )
	{
		PC_COM_TX_BUF[PC_COM_TX_Write] = *(buffer+i);
		PC_COM_TX_Write++;
		PC_COM_TX_Length++;
	}
	
	if( PC_COM_TXD_FLAG == 0 )
	{
		PC_COM_TXD_FLAG = 1;
		
		__HAL_UART_ENABLE_IT(&huart1, UART_IT_TXE);
	}

	return 1;
}








/**
  * @brief  检查数据帧的校验和
	* @param  Frame:	存放帧数据
  * @param  sum：存放校验和
  * @retval 0: 校验失败
						1: 校验成功
  */
unsigned char Check_SUM( struct DataFrame *Frame, unsigned char sum )
{
	unsigned char temp = 0;
	unsigned int i;
	
	for( i=0; i<Frame->Data_Len; i++ )
	{
		temp = temp + Frame->DataBuffer[i];
	}
	
	temp += Frame->CMD_Type;
	temp += ( ( Frame->Data_Len & 0xff00 ) >> 8 );
	temp += ( Frame->Data_Len & 0x00ff );
	
	if( sum != temp ) return 0;
	
	return 1;
}




/**
  * @brief Printf函数重映射
  * @param 
  * @retval 
  */
int fputc(int ch, FILE *f)
{
	unsigned char byte;
	
	byte = (unsigned char) ch;
	
	PC_COM_Send( &byte, 1 );
	
	return ch;
}






/**
  * @brief  从PC串口接收缓存器读取数据帧
  * @retval 0: 0--没有接收到数据帧
						1: 1--接收到数据帧
  */
int PC_COM_Receive_Packet( void  )
{
	unsigned char c;
	static unsigned char flag = 0;
	static unsigned char temp = 0;
	static unsigned int num = 0;
	static unsigned int len = 0;
	unsigned int count = 0, size;
	
	
	if( PC_COM_Frame.ProcessFlag == 1 ) return 0;					//等待上一帧数据处理完成
	if( PC_COM_RX_Read == PC_COM_RX_Write ) return 0;		
	
	size = PC_COM_RX_Write - PC_COM_RX_Read;
	c = PC_COM_RX_BUF[PC_COM_RX_Read];
	
	if( PC_COM_FrameHeadFlag == 0 )
	{	
		switch ( c )
		{
			case 0x0a:
				flag = 1;
				break;
			case 0xa0:
				if( flag == 1 )
				{	
					PC_COM_Frame.FrameHead[0] = 0x0a;
					PC_COM_Frame.FrameHead[1] = 0xa0;
					PC_COM_FrameHeadFlag = 1;
					flag = 0;
				}
				break;
			default:
				flag = 0;
		}
	}
	else
	{
		if( flag == 0 )
		{
			if( c > CmdNum )
			{
				PC_COM_FrameHeadFlag = 0;
				flag = 0;
				num = 0;
				len = 0;
				temp = 0;
				if( PC_COM_RX_Read != 0 ) PC_COM_RX_Read--;		
			}
			else
			{
				PC_COM_Frame.CMD_Type = c;
				flag = 1;
			}
		}
		else if( flag == 1 )
		{
			len = c*256;
			flag = 2;
		}
		else if( flag == 2 )
		{
			len = len + c;
			
			if( len == 0 )
			{
				flag = 4;
				PC_COM_Frame.Data_Len = 0;
			}
			else if( len <= DataNum )
			{
				num = len;
				PC_COM_Frame.Data_Len = len;
				flag = 3;
			}
			else
			{
				PC_COM_FrameHeadFlag = 0;
				flag = 0;
				num = 0;
				len = 0;
				temp = 0;
				if( PC_COM_RX_Read > 0 && PC_COM_RX_Read <= 1  ) PC_COM_RX_Read = 0;		
				else if( PC_COM_RX_Read >= 2 ) PC_COM_RX_Read = PC_COM_RX_Read - 2;
			}
		}
		else if( flag == 3 )
		{
//			if( num > 0 ) 
//			{
//				PC_COM_Frame.DataBuffer[temp] = c;
//				temp++;
//				num--;
//				if( num == 0 ) flag = 4;
//			}
			while( ( num > 0 ) && ( size > 0 ) && ( count <= 10 ) ) 
			{
				PC_COM_Frame.DataBuffer[temp] = PC_COM_RX_BUF[PC_COM_RX_Read];
				temp++;
				num--;
				size--;
				count++;
				PC_COM_RX_Read++;
			}
			
			if( num == 0 )
			{
				flag = 4;
				PC_COM_RX_Read--;
			}
			else
			{
				PC_COM_RX_Read--;
			}
		}
		else if( flag == 4 )
		{
			if( Check_SUM( &PC_COM_Frame, c ) == 1 )
			{
				PC_COM_Frame.FrameSum = c;
				flag = 5;
			}
			else
			{
				PC_COM_FrameHeadFlag = 0;
				temp = 0;
				flag = 0;
				num = 0;
				len = 0;
				if( PC_COM_RX_Read >= ( 4 + PC_COM_Frame.Data_Len) ) PC_COM_RX_Read = PC_COM_RX_Read - 4 - PC_COM_Frame.Data_Len;	
				else if( PC_COM_RX_Read == 0 || PC_COM_RX_Read == 1 ) PC_COM_RX_Read = 0;
				else if( PC_COM_RX_Read == 2 )  PC_COM_RX_Read =  PC_COM_RX_Read - 2;
				else if( PC_COM_RX_Read == 3 ) 	PC_COM_RX_Read =  PC_COM_RX_Read - 3;
			}
		}
		else if( flag == 5 )
		{
			if( c == 0x16 )
			{
				PC_COM_Frame.FrameTail = 0x16;
				PC_COM_Frame.ProcessFlag = 1;
				PC_COM_FrameFlag = 1;
				PC_COM_FrameHeadFlag = 0;
				temp = 0;
				flag = 0;
				num = 0;
				len = 0;
			}
			else
			{
				PC_COM_FrameFlag = 0;
				PC_COM_FrameHeadFlag = 0;
				temp = 0;
				flag = 0;
				num = 0;
				len = 0;
				if( PC_COM_RX_Read != 0 ) PC_COM_RX_Read--;
			}
		}
	}

	PC_COM_RX_Read++;
	
	if( PC_COM_RX_Read == PC_COM_RX_Write )
	{
		PC_COM_RX_Read = 0;
		PC_COM_RX_Write = 0;
		PC_COM_RX_Length = 0;
		memset( PC_COM_RX_BUF, 0, 2048);
	}
	
	if( PC_COM_FrameFlag == 1 )
	{
		PC_COM_FrameFlag = 0;
		return 1;
	}
	else return 0;
}













