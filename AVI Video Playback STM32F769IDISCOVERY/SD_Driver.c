#include "stm32f7xx_hal.h"
#include <stdio.h>
#include "PC_COM.h"
#include "ff_gen_drv.h"





DSTATUS sd_initialize( BYTE drv );
DSTATUS sd_status(BYTE drv);
DRESULT sd_read(BYTE drv, BYTE *buff, DWORD sector, UINT count);
DRESULT sd_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count);
DRESULT sd_ioctl(BYTE drv, BYTE cmd, void *buff);




SD_HandleTypeDef hsd;

FATFS SD;																																							//SD卡文件系统
unsigned char SD_Path[4];																															//SD卡Disk的路径

volatile unsigned char SD_Error_Flag = 0;																							//SD故障标志

static volatile DSTATUS SD_Stat = STA_NOINIT;

Diskio_drvTypeDef SD_Driver = 																												//SD卡驱动
{
	sd_initialize,
	sd_status,
	sd_read,
	
	#if _USE_WRITE == 1 
	sd_write,
	#endif
	
	#if _USE_IOCTL == 1 
	sd_ioctl
	#endif
};






/**
* @brief  延时函数，延时Xms
* @note   
* @retval None
*/
void SDMMC2_Delay_1ms( unsigned int ms )
{  
//	OS_ERR   p_err;
	
	unsigned int i, j;
	
	for( i=0; i<ms; i++ )
	for( j=0; j<100000; j++ );
	
//	OSTimeDly ( ms, OS_OPT_TIME_DLY, &p_err );
}





/**
* @brief  配置MCU与SD卡的接口.
* @note   
* @retval None
*/
void SD_GPIO_Config( void )
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOI_CLK_ENABLE();
	
	
	GPIO_InitStruct.Pin = GPIO_PIN_9;														//PG9--SD_D0
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF11_SDMMC2;
	
	HAL_GPIO_Init( GPIOG, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_10;													//PG10--SD_D1
	HAL_GPIO_Init( GPIOG, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_3;														//PB3--SD_D2
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF10_SDMMC2;
	HAL_GPIO_Init( GPIOB, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_4;														//PB4--SD_D3
	HAL_GPIO_Init( GPIOB, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_6;														//PD6--SD_CLK
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF11_SDMMC2;
	HAL_GPIO_Init( GPIOD, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_7;														//PD7--SD_CMD
	HAL_GPIO_Init( GPIOD, &GPIO_InitStruct );
	
	GPIO_InitStruct.Pin = GPIO_PIN_15;													//PI15--SD_Detect
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	
	HAL_GPIO_Init( GPIOI, &GPIO_InitStruct );
}





/**
  * @brief  检查SD卡是否插入卡槽
  * @retval Returns if SD is detected or not
  */
unsigned char SD_Detected( void )
{
  /* Check SD card detect pin */
  if(HAL_GPIO_ReadPin( GPIOI, GPIO_PIN_15 ) == GPIO_PIN_SET )
  {
   return 0;
  }

  return 1;
}





/**
* @brief  配置SD卡的工作参数
* @note   
* @retval 1: 成功		0：失败
*/
unsigned char SD_Config( void )
{
	HAL_SD_CardInfoTypedef SDCardInfo;
//	RCC_PeriphCLKInitTypeDef PeriphClkInit;
	
	
	__HAL_RCC_SDMMC2_CLK_ENABLE();
	/** Sw reset of SDMMC2 IP */
	__HAL_RCC_SDMMC2_FORCE_RESET();
	SDMMC2_Delay_1ms( 10 );
  __HAL_RCC_SDMMC2_RELEASE_RESET();
	SDMMC2_Delay_1ms( 10 );
	
	
//	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_CLK48;
//	PeriphClkInit.Clk48ClockSelection = RCC_CLK48SOURCE_PLLSAIP;
//	HAL_RCCEx_PeriphCLKConfig( &PeriphClkInit );
//	
//	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_SDMMC2;
//	PeriphClkInit.Sdmmc2ClockSelection = RCC_SDMMC2CLKSOURCE_CLK48;
//	HAL_RCCEx_PeriphCLKConfig( &PeriphClkInit );
	
	
	hsd.Instance = SDMMC2;
	HAL_SD_DeInit( &hsd );
	
	hsd.Init.ClockEdge =  SDMMC_CLOCK_EDGE_RISING; 
	hsd.Init.ClockBypass = SDMMC_CLOCK_BYPASS_DISABLE;
	hsd.Init.ClockPowerSave = SDMMC_CLOCK_POWER_SAVE_DISABLE;
	hsd.Init.BusWide = SDMMC_BUS_WIDE_1B;
	hsd.Init.HardwareFlowControl = SDMMC_HARDWARE_FLOW_CONTROL_ENABLE;
	hsd.Init.ClockDiv = 0;
	if( HAL_SD_Init(&hsd, &SDCardInfo) != SD_OK ) return 0;
	
	hsd.CardType = SDCardInfo.CardType;
//	printf("SD卡的大小：%lld\r\n", SDCardInfo.CardCapacity );
//	printf("SD卡数据块的大小：%d\r\n",  SDCardInfo.CardBlockSize );
	
	if( HAL_SD_WideBusOperation_Config( &hsd, SDMMC_BUS_WIDE_4B ) != SD_OK )  return 0;

	return 1;
}
	
	
	
	


/**
* @brief  初始化SD卡
* @note   
* @retval 1: 成功		0：失败
*/
unsigned char SD_Init( void )
{
	SD_GPIO_Config(  );															//配置MCU与SD卡的接口
	
	if( SD_Detected( ) == 0 ) 											//检查SD卡是否插入卡槽
	{
		printf("SD卡初始化失败。\r\n");
		SD_Error_Flag = 1;
		return 0;
	}		
	
	if( SD_Config(  ) == 0 )												//配置SD卡的工作参数  
	{
		printf("SD卡初始化失败。\r\n");
		SD_Error_Flag = 1;
		return 0;							
	}
	
	printf("SD卡初始化成功。\r\n");
	
	SD_Error_Flag = 0;
	
	return 1;
}




/**
  * @brief  Initializes a Drive
  * @param  drv : not used 
  * @retval DSTATUS: Operation status
  */
DSTATUS sd_initialize( BYTE drv )
{
	SD_Stat = STA_NOINIT;
	
	if( SD_Init( ) == 1 ) 
	{
		SD_Stat = 0;
	}
	
	return SD_Stat;
}
	
	

/**
  * @brief  Gets Disk Status
  * @param  drv : not used
  * @retval DSTATUS: Operation status
  */
DSTATUS sd_status(BYTE drv)
{
	SD_Stat = STA_NOINIT;
	
	if( HAL_SD_GetStatus(&hsd) == SD_TRANSFER_OK ) 
	{
		SD_Stat = 0;
	}
  
  return SD_Stat;
}




/**
  * @brief  Reads Sector(s)
  * @param  drv : not used
  * @param  *buff: Data buffer to store read data
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to read (1..128)
  * @retval DRESULT: Operation result
  */
DRESULT sd_read(BYTE drv, BYTE *buff, DWORD sector, UINT count)
{
	if( HAL_SD_ReadBlocks( &hsd, (unsigned int *)buff, sector*512, 512, count ) == SD_OK ) return RES_OK;
	
	return RES_ERROR;
}



/**
  * @brief  Writes Sector(s)
  * @param  drv : not used
  * @param  *buff: Data to be written
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to write (1..128)
  * @retval DRESULT: Operation result
  */
#if _USE_WRITE == 1
DRESULT sd_write(BYTE lun, const BYTE *buff, DWORD sector, UINT count)
{
  if( HAL_SD_WriteBlocks( &hsd, (unsigned int *)buff, sector*512, 512, count ) == SD_OK ) return RES_OK;
	
	return RES_ERROR;
}
#endif /* _USE_WRITE == 1 */






/**
  * @brief  I/O control operation
  * @param  drv : not used
  * @param  cmd: Control code
  * @param  *buff: Buffer to send/receive control data
  * @retval DRESULT: Operation result
  */
#if _USE_IOCTL == 1
DRESULT sd_ioctl(BYTE drv, BYTE cmd, void *buff)
{
  DRESULT res = RES_ERROR;
  HAL_SD_CardInfoTypedef CardInfo;
	
	if( SD_Stat & STA_NOINIT) return RES_NOTRDY; 
  
  switch (cmd)
  {
  /* Make sure that no pending write process */
  case CTRL_SYNC :
    res = RES_OK;
    break;
  
  /* Get number of sectors on the disk (DWORD) */
  case GET_SECTOR_COUNT :
    HAL_SD_Get_CardInfo( &hsd, &CardInfo);
    *(DWORD*)buff = CardInfo.CardCapacity / 512;
    res = RES_OK;
    break;
  
  /* Get R/W sector size (WORD) */
  case GET_SECTOR_SIZE :
    *(WORD*)buff = 512;
    res = RES_OK;
    break;
  
  /* Get erase block size in unit of sector (DWORD) */
  case GET_BLOCK_SIZE :
    *(DWORD*)buff = 512;
    break;
  
  default:
    res = RES_PARERR;
  }
  
  return res;
}
#endif /* _USE_IOCTL == 1 */




/**
  * @brief  Gets Time from RTC 
  * @param  None
  * @retval Time in DWORD
  */
#if _FS_NORTC == 0 


#endif
	
	

