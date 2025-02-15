#include "stm32f7xx_hal.h"

#include "PC_COM.h"
#include "SDRAM.h"

//#include  "os.h"


#ifdef HAL_SD_MODULE_ENABLED
	#include "ff.h"	
	
	FILINFO load_information;
	FIL load_object;
#endif 






#define SDRAM_MODEREG_BURST_LENGTH_1             ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             ((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             ((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             ((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      ((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     ((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              ((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              ((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED ((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     ((uint16_t)0x0200)

#define SDRAM_BANK_ADDR                 ((uint32_t)0xC0000000)





SDRAM_HandleTypeDef  hsdram;					//SDRAM操作句柄


//extern OS_MUTEX SDRAM_Mutex;					//SDRAM互斥信号量
//extern OS_MUTEX SD_Mutex;
//extern OS_MUTEX Flash_Mutex;					//外部Flash互斥信号量





/**
* @brief  延时函数，延时Xms
* @note   
* @retval None
*/
void SDRAM_Delay_1ms( unsigned int ms )
{  
//	OS_ERR   p_err;
	
	unsigned int i, j;
	
	for( i=0; i<ms; i++ )
	for( j=0; j<100000; j++ );
	
//	OSTimeDly ( ms, OS_OPT_TIME_DLY, &p_err );
}




/**
* @brief  配置MCU与SDRAM的接口.
* @note   
* @retval None
*/
void SDRAM_GPIO_Config( void )
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOI_CLK_ENABLE();
//	__HAL_RCC_SYSCFG_CLK_ENABLE();
	
	/*	配置SDRAM的地址总线		*/
	GPIO_InitStruct.Pin = GPIO_PIN_0;														//PF0--A0
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
	
	HAL_GPIO_Init( GPIOF, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_1;														//PF1--A1
	HAL_GPIO_Init( GPIOF, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_2;														//PF2--A2
	HAL_GPIO_Init( GPIOF, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_3;														//PF3--A3
	HAL_GPIO_Init( GPIOF, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_4;														//PF4--A4
	HAL_GPIO_Init( GPIOF, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_5;														//PF5--A5
	HAL_GPIO_Init( GPIOF, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_12;													//PF12--A6
	HAL_GPIO_Init( GPIOF, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_13;													//PF13--A7
	HAL_GPIO_Init( GPIOF, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_14;													//PF14--A8
	HAL_GPIO_Init( GPIOF, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_15;													//PF15--A9
	HAL_GPIO_Init( GPIOF, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_0;														//PG0--A10
	HAL_GPIO_Init( GPIOG, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_1;														//PG1--A11
	HAL_GPIO_Init( GPIOG, &GPIO_InitStruct );
	
	/*	配置SDRAM的数据总线		*/
	GPIO_InitStruct.Pin = GPIO_PIN_14;													//PD14--D0
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
	
	HAL_GPIO_Init( GPIOD, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_15;													//PD15--D1
	HAL_GPIO_Init( GPIOD, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_0;														//PD0--D2
	HAL_GPIO_Init( GPIOD, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_1;														//PD1--D3
	HAL_GPIO_Init( GPIOD, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_7;														//PE7--D4
	HAL_GPIO_Init( GPIOE, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_8;														//PE8--D5
	HAL_GPIO_Init( GPIOE, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_9;														//PE9--D6
	HAL_GPIO_Init( GPIOE, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_10;													//PE10--D7
	HAL_GPIO_Init( GPIOE, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_11;													//PE11--D8
	HAL_GPIO_Init( GPIOE, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_12;													//PE12--D9
	HAL_GPIO_Init( GPIOE, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_13;													//PE13--D10
	HAL_GPIO_Init( GPIOE, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_14;													//PE14--D11
	HAL_GPIO_Init( GPIOE, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_15;													//PE15--D12
	HAL_GPIO_Init( GPIOE, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_8;														//PD8--D13
	HAL_GPIO_Init( GPIOD, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_9;														//PD9--D14
	HAL_GPIO_Init( GPIOD, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_10;													//PD10--D15
	HAL_GPIO_Init( GPIOD, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_8;														//PH8--D16
	HAL_GPIO_Init( GPIOH, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_9;														//PH9--D17
	HAL_GPIO_Init( GPIOH, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_10;													//PH10--D18
	HAL_GPIO_Init( GPIOH, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_11;													//PH11--D19
	HAL_GPIO_Init( GPIOH, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_12;													//PH12--D20
	HAL_GPIO_Init( GPIOH, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_13;													//PH13--D21
	HAL_GPIO_Init( GPIOH, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_14;													//PH14--D22
	HAL_GPIO_Init( GPIOH, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_15;													//PH15--D23
	HAL_GPIO_Init( GPIOH, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_0;														//PI0--D24
	HAL_GPIO_Init( GPIOI, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_1;														//PI1--D25
	HAL_GPIO_Init( GPIOI, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_2;														//PI2--D26
	HAL_GPIO_Init( GPIOI, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_3;														//PI3--D27
	HAL_GPIO_Init( GPIOI, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_6;														//PI6--D28
	HAL_GPIO_Init( GPIOI, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_7;														//PI7--D29
	HAL_GPIO_Init( GPIOI, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_9;														//PI9--D30
	HAL_GPIO_Init( GPIOI, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_10;													//PI10--D31
	HAL_GPIO_Init( GPIOI, &GPIO_InitStruct );
	
	/*	配置SDRAM的控制线	*/
	GPIO_InitStruct.Pin = GPIO_PIN_4;														//PG4--BA0
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
	
	HAL_GPIO_Init( GPIOG, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_5;														//PG5--BA1
	HAL_GPIO_Init( GPIOG, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_5;														//PH5--WE
	HAL_GPIO_Init( GPIOH, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_15;													//PG15--CAS
	HAL_GPIO_Init( GPIOG, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_11;													//PF11--RAS
	HAL_GPIO_Init( GPIOF, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_3;														//PH3--CS
	HAL_GPIO_Init( GPIOH, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_2;														//PH2--CKE
	HAL_GPIO_Init( GPIOH, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_8;														//PG8--CLK
	HAL_GPIO_Init( GPIOG, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_0;														//PE0--DQM0
	HAL_GPIO_Init( GPIOE, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_1;														//PE1--DQM1
	HAL_GPIO_Init( GPIOE, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_4;														//PI4--DQM2
	HAL_GPIO_Init( GPIOI, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_5;														//PI5--DQM3
	HAL_GPIO_Init( GPIOI, &GPIO_InitStruct );
}




/**
* @brief  配置SDRAM的工作参数
* @note   
* @retval None
*/
void SDRAM_Config( void )
{
	FMC_SDRAM_TimingTypeDef Timing;
	FMC_SDRAM_CommandTypeDef Command;
	uint32_t temp;
	
	__HAL_RCC_FMC_CLK_ENABLE();
	/** Sw reset of FMC IP */
	__HAL_RCC_FMC_FORCE_RESET();
	SDRAM_Delay_1ms( 10 );
  __HAL_RCC_FMC_RELEASE_RESET();
	SDRAM_Delay_1ms( 10 );
	
	
	hsdram.Instance = FMC_Bank5_6;
	HAL_SDRAM_DeInit( &hsdram );
	
	hsdram.Init.SDBank = FMC_SDRAM_BANK1;
	hsdram.Init.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_8;
	hsdram.Init.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_12;
	hsdram.Init.MemoryDataWidth = FMC_SDRAM_MEM_BUS_WIDTH_32;
	hsdram.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
	hsdram.Init.CASLatency = FMC_SDRAM_CAS_LATENCY_2;
	hsdram.Init.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
	hsdram.Init.SDClockPeriod = FMC_SDRAM_CLOCK_PERIOD_2;
	hsdram.Init.ReadBurst = FMC_SDRAM_RBURST_ENABLE;
	hsdram.Init.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_0;
	
	//TMRD
	Timing.LoadToActiveDelay = 2;
	//TXSR
	Timing.ExitSelfRefreshDelay = 7;		//6	
	//TRAS
	Timing.SelfRefreshTime = 5;					//4	
	//TRC
	Timing.RowCycleDelay = 7;						//5	
	//TWR
	Timing.WriteRecoveryTime = 2;				//1
	//TRP
	Timing.RPDelay = 2;									//2
	//TRCD
	Timing.RCDDelay = 2;								//2
	
	HAL_SDRAM_Init( &hsdram, &Timing );
	
	/* Configure a clock configuration enable command */
	Command.CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;
	Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
	Command.AutoRefreshNumber = 1;
  Command.ModeRegisterDefinition = 0;
	HAL_SDRAM_SendCommand( &hsdram, &Command, 100 );
	
	SDRAM_Delay_1ms( 10 );
	
	/* Configure a PALL (precharge all) command */
  Command.CommandMode = FMC_SDRAM_CMD_PALL;
  Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
  Command.AutoRefreshNumber = 1;
  Command.ModeRegisterDefinition = 0;
	HAL_SDRAM_SendCommand( &hsdram, &Command, 100 );
	
	/* Configure a Auto-Refresh command */
  Command.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
  Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
  Command.AutoRefreshNumber = 8;
  Command.ModeRegisterDefinition = 0;
	HAL_SDRAM_SendCommand( &hsdram, &Command, 100 );
	
	/* Configure a Auto-Refresh command */
  Command.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
  Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
  Command.AutoRefreshNumber = 8;
  Command.ModeRegisterDefinition = 0;
	HAL_SDRAM_SendCommand( &hsdram, &Command, 100 );
	
	/* Program the external memory mode register */
	temp = SDRAM_MODEREG_WRITEBURST_MODE_SINGLE 	| 
				 SDRAM_MODEREG_OPERATING_MODE_STANDARD 	|
				 SDRAM_MODEREG_CAS_LATENCY_2						|
				 SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL		|
				 SDRAM_MODEREG_BURST_LENGTH_8;
				
	Command.CommandMode = FMC_SDRAM_CMD_LOAD_MODE;
  Command.CommandTarget = FMC_SDRAM_CMD_TARGET_BANK1;
  Command.AutoRefreshNumber = 1;
  Command.ModeRegisterDefinition = temp;
	HAL_SDRAM_SendCommand( &hsdram, &Command, 100 );
	
	/*  Set the refresh rate counter */
  /* (15.62 us x Freq) - 20 */
  /* Set the device refresh counter */
//	HAL_SDRAM_ProgramRefreshRate( &hsdram, 1230 );
	HAL_SDRAM_ProgramRefreshRate( &hsdram, 1542);
}






/**
* @brief  Writes 8-bit data buffer to SDRAM memory.
* @param  addr: 存放写入地址
* @param  buffer: 存放写入的数据  
* @param	num：写入的字节数量
* @retval 
*/
void SDRAM_Write_8B( unsigned int addr, unsigned char *buf, unsigned int num )
{
	while( HAL_SDRAM_Write_8b( &hsdram, (unsigned int *)(addr+SDRAM_BANK_ADDR), buf, num ) != HAL_OK );
}





/**
* @brief  Reads 8-bit data buffer from the SDRAM memory.
* @param  addr: 存放读出地址
* @param  buffer: 存放读取的数据  
* @param	num：读取的字节数量
* @retval 
*/
void SDRAM_Read_8B( unsigned int addr, unsigned char *buf, unsigned int num )
{
	while( HAL_SDRAM_Read_8b( &hsdram, (unsigned int *)(addr+SDRAM_BANK_ADDR), buf, num ) != HAL_OK );
}





/**
* @brief  Writes 16-bit data buffer to SDRAM memory.
* @param  addr: 存放写入地址
* @param  buffer: 存放写入的数据  
* @param	num：写入的字节数量
* @retval 
*/
void SDRAM_Write_16B( unsigned int addr, unsigned short int *buf, unsigned int num )
{
	while( HAL_SDRAM_Write_16b( &hsdram, (unsigned int *)(addr+SDRAM_BANK_ADDR), buf, num ) != HAL_OK );
}




/**
* @brief  Reads 16-bit data buffer from the SDRAM memory.
* @param  addr: 存放读出地址
* @param  buffer: 存放读取的数据  
* @param	num：读取的字节数量
* @retval 
*/
void SDRAM_Read_16B( unsigned int addr, unsigned short int *buf, unsigned int num )
{
	while( HAL_SDRAM_Read_16b( &hsdram, (unsigned int *)(addr+SDRAM_BANK_ADDR), buf, num ) != HAL_OK );
}





/**
* @brief  Writes 32-bit data buffer to SDRAM memory.
* @param  addr: 存放写入地址
* @param  buffer: 存放写入的数据  
* @param	num：写入的字节数量
* @retval 
*/
void SDRAM_Write_32B( unsigned int addr, unsigned int *buf, unsigned int num )
{
	while(  HAL_SDRAM_Write_32b( &hsdram, (unsigned int *)(addr+SDRAM_BANK_ADDR), buf, num ) != HAL_OK );
}




/**
* @brief  Reads 32-bit data buffer from the SDRAM memory.
* @param  addr: 存放读出地址
* @param  buffer: 存放读取的数据  
* @param	num：读取的字节数量
* @retval 
*/
void SDRAM_Read_32B( unsigned int addr, unsigned int *buf, unsigned int num )
{
	while( HAL_SDRAM_Read_32b( &hsdram,(unsigned int *)(addr+SDRAM_BANK_ADDR), buf, num ) != HAL_OK );
}




/**
* @brief  检查SDRAM与MCU连接是否正常
* @note   
* @retval 1: 成功		0：失败
*/
unsigned char SDRAM_Check( void )
{
	unsigned char buf1[250];
//	unsigned short int buf2[250];
//	unsigned int buf3[250];
	unsigned short int i;
	
	for( i=0; i<250; i++ )
	{
		buf1[i] = i;
	}
	
	SDRAM_Write_8B( 0x00600000, buf1, 250 );
	
	for( i=0; i<250; i++ )
	{
		buf1[i] = 0;
	}
	
	SDRAM_Read_8B( 0x00600000, buf1, 250 );
	
	for( i=0; i<250; i++ )
	{
		if( buf1[i] != (unsigned char )i ) return 0;
	}

//	for( i=0; i<250; i++ )
//	{
//		buf2[i] = i;
//	}
//	
//	SDRAM_Write_16B( 0x00600000, buf2, 250 );
//	
//	for( i=0; i<250; i++ )
//	{
//		buf2[i] = 0;
//	}
//	
//	SDRAM_Read_16B( 0x00600000, buf2, 250 );
//	
//	printf("\r\n");
//	for( i=0; i<250; i++ )
//	{
//		if( buf2[i] != i ) return 0;
//		printf(" %d", buf2[i] );
//	}
//	
//	
//	for( i=0; i<250; i++ )
//	{
//		buf3[i] = i;
//	}
//	
//	SDRAM_Write_32B( 0x00600000, buf3, 250 );
//	
//	for( i=0; i<250; i++ )
//	{
//		buf3[i] = 0;
//	}
//	
//	SDRAM_Read_32B( 0x00600000, buf3, 250 );
//	
//	printf("\r\n");
//	for( i=0; i<250; i++ )
//	{
//		if( buf3[i] != i ) return 0;
//		printf(" %d", buf3[i] );
//	}
	return 1;
}
	
	
	
	
/**
* @brief  初始化SDRAM
* @note   
* @retval 1: 成功		0：失败
*/
unsigned char SDRAM_Init( void )
{
	SDRAM_GPIO_Config(  );			//配置MCU与SDRAM的接口
	SDRAM_Config( );						//配置SDRAM的工作参数
	
	if( SDRAM_Check( ) != 1 ) 	//检查SDRAM与MCU连接是否正常
	{
		printf("SDRAM初始化失败。\r\n");
		return 0;		
	}		
	
	printf("SDRAM初始化成功。\r\n");
	return 1;
}





#ifdef HAL_SD_MODULE_ENABLED
/**
* @brief：从SD卡加载文件到SDRAM
* @param  file_name:  加载文件的存放路径
* @param  file_size:  存放加载文件的大小
* @param  addr:  SDRAM的地址
* @retval 1: 成功		0：失败
*/
unsigned char Load_File_From_SD( unsigned char *file_name, unsigned int *file_size, unsigned int addr )
{
	unsigned int num, temp, sdram_addr;
	unsigned int read_num;
//	CPU_TS ts;
//  OS_ERR err;
//	
//	OSMutexPend ( &SD_Mutex,
//								0,
//								OS_OPT_PEND_BLOCKING,
//								&ts,
//								&err);
	
	if( f_open( &load_object, (const TCHAR*)file_name, FA_OPEN_EXISTING | FA_READ ) != FR_OK ) 
	{
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&err);
		return 0;
	}
	
	num = f_size( &load_object );
	
	if( num > 5*1024*1024 ) 
	{
		f_close( &load_object );
		
//		OSMutexPost ( &SD_Mutex,
//									OS_OPT_POST_NONE,
//									&err);
		return 0;
	}
	
//	OSMutexPost ( &SD_Mutex,
//								OS_OPT_POST_NONE,
//								&err);
	
	*file_size = num;
	sdram_addr = SDRAM_BANK1_ADDR + addr;
	
//	OSMutexPend ( &SDRAM_Mutex,
//								0,
//								OS_OPT_PEND_BLOCKING,
//								&ts,
//								&err);
	
//	OSMutexPend ( &SD_Mutex,
//								0,
//								OS_OPT_PEND_BLOCKING,
//								&ts,
//								&err);
	
	while( num != 0 )
	{
		if( num > 4096 ) temp = 4096;
		else temp = num;
		
		if( f_read( &load_object, (unsigned char *)sdram_addr, temp, &read_num ) != FR_OK ) 
		{
			f_close( &load_object );
			
//			OSMutexPost ( &SD_Mutex,
//										OS_OPT_POST_NONE,
//										&err);
//			
//			OSMutexPost ( &SDRAM_Mutex,
//										OS_OPT_POST_NONE,
//										&err);
			return 0;
		}
		
		sdram_addr += temp;
		
		num -= temp;
	}
	
	f_close( &load_object );	
	
//	OSMutexPost ( &SD_Mutex,
//								OS_OPT_POST_NONE,
//								&err);
//	
//	OSMutexPost ( &SDRAM_Mutex,
//								OS_OPT_POST_NONE,
//								&err);
	return 1;
}
#endif 




//#ifdef  HAL_FLASH_MODULE_ENABLED
//	#include "Flash.h"

///**
//* @brief：从Flash加载文件到SDRAM
//* @param  flash_addr:  Flash的地址
//* @param  file_size:  加载文件的大小
//* @param  sdram_addr:  SDRAM的地址
//* @retval 1: 成功		0：失败
//*/
//unsigned char Load_File_From_Flsah( unsigned int flash_addr, unsigned int file_size, unsigned int sdram_addr )
//{
//	unsigned int num, temp, addr1, addr2;
//	CPU_TS ts;
//  OS_ERR err;
//	
//	num = file_size;
//	
//	if( num > 3*1024*1024 ) 
//	{
//		return 0;
//	}
//	
//	addr1 = SDRAM_BANK1_ADDR + sdram_addr;
//	addr2 = flash_addr;
//	
//	OSMutexPend ( &SDRAM_Mutex,
//								0,
//								OS_OPT_PEND_BLOCKING,
//								&ts,
//								&err);
//	
//	OSMutexPend ( &Flash_Mutex,
//								0,
//								OS_OPT_PEND_BLOCKING,
//								&ts,
//								&err);
//	
//	while( num != 0 )
//	{
//		if( num > 512 ) temp = 512;
//		else temp = num;
//		
//		if( Byte_Read( addr2, (unsigned char *)addr1, temp ) != 1 )
//		{
//			OSMutexPost ( &Flash_Mutex,
//										OS_OPT_POST_NONE,
//										&err);
//			
//			OSMutexPost ( &SDRAM_Mutex,
//										OS_OPT_POST_NONE,
//										&err);
//			return 0;
//		}
//		
//		addr1 += temp;
//		addr2 += temp;
//		
//		num -= temp;
//	}
//	
//	OSMutexPost ( &Flash_Mutex,
//								OS_OPT_POST_NONE,
//								&err);
//	
//	OSMutexPost ( &SDRAM_Mutex,
//								OS_OPT_POST_NONE,
//								&err);
//	return 1;
//}

//#endif 


