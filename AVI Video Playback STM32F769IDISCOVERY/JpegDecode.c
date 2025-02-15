#include "stm32f7xx.h"
#include "stm32f7xx_hal.h"

#include "JpegDecode.h"
#include "LCD_Driver.h"
#include "SD_Driver.h"
#include "SDRAM.h"
#include "jpeg_utils.h"

#include "ff.h"	

#include "string.h"	

#include "jpeglib.h"
#include "tjpgd.h"





#if (JPEG_Decode_Mode == 0 || JPEG_Decode_Mode == 1 || JPEG_Decode_Mode == 2 )
	#define CHUNK_SIZE_IN  ((uint32_t)(4*1024)) 
	#define CHUNK_SIZE_OUT ((uint32_t)(768))

	#define JPEG_BUFFER_EMPTY 0
	#define JPEG_BUFFER_FULL  1

	#define NB_OUTPUT_DATA_BUFFERS      2
	#define NB_INPUT_DATA_BUFFERS       2
#endif


extern volatile unsigned int AVI_IN_BUF1_Len;
extern volatile unsigned int AVI_IN_BUF2_Len;
extern volatile unsigned char AVI_BUF_FLAG;


extern LTDC_HandleTypeDef hltdc;
extern DMA2D_HandleTypeDef hdma2d;
extern unsigned short int LCD_With;											
extern unsigned short int LCD_Heigh;


extern volatile unsigned int ms_count;															//ms计数器

extern FIL AVI;



#if (JPEG_Decode_Mode == 0 || JPEG_Decode_Mode == 1 || JPEG_Decode_Mode == 2 )
	unsigned char MCU_Data_OutBuffer0[CHUNK_SIZE_OUT];
	unsigned char MCU_Data_OutBuffer1[CHUNK_SIZE_OUT];

	unsigned char JPEG_Data_InBuffer0[CHUNK_SIZE_IN];
	unsigned char JPEG_Data_InBuffer1[CHUNK_SIZE_IN];

	JPEG_Data_BufferTypeDef Jpeg_OUT_BufferTab[NB_OUTPUT_DATA_BUFFERS] =
	{
		{JPEG_BUFFER_EMPTY , MCU_Data_OutBuffer0 , 0},
		{JPEG_BUFFER_EMPTY , MCU_Data_OutBuffer1, 0}
	};

	JPEG_Data_BufferTypeDef Jpeg_IN_BufferTab[NB_INPUT_DATA_BUFFERS] =
	{
		{JPEG_BUFFER_EMPTY , JPEG_Data_InBuffer0, 0},
		{JPEG_BUFFER_EMPTY , JPEG_Data_InBuffer1, 0}
	};

	_Jpeg_Decode_Info HW_Jpeg_Decode_Info;
	
	volatile unsigned char Jpeg_Or_AVI = 0;																		
#endif



FIL video_object;

JPEG_HandleTypeDef hjpeg;
JPEG_ConfTypeDef jpegInfo;
DMA_HandleTypeDef jpeg_hdmain;
DMA_HandleTypeDef jpeg_hdmaout;
JPEG_YCbCrToRGB_Convert_Function JPEG_ToRGB;




volatile int display_xpos;
volatile int display_ypos;
volatile int image_with;
volatile int image_heigh;
JDEC TjpgLB_Jpeg;


/* This struct contains the JPEG decompression parameters */
struct jpeg_decompress_struct cinfo;
/* This struct represents a JPEG error handler */
struct jpeg_error_mgr jerr;
volatile unsigned short int line_counter;


#if ( JPEG_Decode_Mode == 3 || JPEG_Decode_Mode == 4  )
unsigned char jpeg_buffer[10*1024]  __attribute__ ((aligned(4)));
#endif






/**
* @brief  延时函数，延时Xms
* @note   
* @retval None
*/
void JPEG_Delay_1ms( unsigned int ms )
{  
	unsigned int i, j;
	
	for( i=0; i<ms; i++ )
	for( j=0; j<100000; j++ );
}



#if (JPEG_Decode_Mode == 0 || JPEG_Decode_Mode == 1 )
/**
  * @brief  JPEG Ouput Data BackGround Postprocessing .
  * @param hjpeg: JPEG handle pointer
  * @retval 1 : if JPEG processing has finiched, 0 : if JPEG processing still ongoing
  */
uint32_t JPEG_OutputHandler(JPEG_HandleTypeDef *hjpeg)
{
  uint32_t ConvertedDataCount;
  
  if(Jpeg_OUT_BufferTab[HW_Jpeg_Decode_Info.JPEG_OUT_Read_BufferIndex].State == JPEG_BUFFER_FULL)
  {  
    HW_Jpeg_Decode_Info.MCU_BlockIndex += JPEG_ToRGB(Jpeg_OUT_BufferTab[HW_Jpeg_Decode_Info.JPEG_OUT_Read_BufferIndex].DataBuffer, (uint8_t *)HW_Jpeg_Decode_Info.Data_Addr, HW_Jpeg_Decode_Info.MCU_BlockIndex, Jpeg_OUT_BufferTab[HW_Jpeg_Decode_Info.JPEG_OUT_Read_BufferIndex].DataBufferSize, &ConvertedDataCount);   
    
    Jpeg_OUT_BufferTab[HW_Jpeg_Decode_Info.JPEG_OUT_Read_BufferIndex].State = JPEG_BUFFER_EMPTY;
    Jpeg_OUT_BufferTab[HW_Jpeg_Decode_Info.JPEG_OUT_Read_BufferIndex].DataBufferSize = 0;
    
    HW_Jpeg_Decode_Info.JPEG_OUT_Read_BufferIndex++;
    if(HW_Jpeg_Decode_Info.JPEG_OUT_Read_BufferIndex >= NB_OUTPUT_DATA_BUFFERS)
    {
      HW_Jpeg_Decode_Info.JPEG_OUT_Read_BufferIndex = 0;
    }
    
    if(HW_Jpeg_Decode_Info.MCU_BlockIndex == HW_Jpeg_Decode_Info.MCU_TotalNb )
    {
      return 1;
    }
  }
  else if((HW_Jpeg_Decode_Info.Output_Is_Paused == 1) && \
          (HW_Jpeg_Decode_Info.JPEG_OUT_Write_BufferIndex == HW_Jpeg_Decode_Info.JPEG_OUT_Read_BufferIndex) && \
          (Jpeg_OUT_BufferTab[HW_Jpeg_Decode_Info.JPEG_OUT_Read_BufferIndex].State == JPEG_BUFFER_EMPTY))
  {
    HW_Jpeg_Decode_Info.Output_Is_Paused = 0;
    HAL_JPEG_Resume(hjpeg, JPEG_PAUSE_RESUME_OUTPUT);            
  }

  return 0;  
}





/**
  * @brief  JPEG Input Data BackGround processing .
  * @param hjpeg: JPEG handle pointer
  * @retval None
  */
void JPEG_InputHandler(JPEG_HandleTypeDef *hjpeg )
{
  if(Jpeg_IN_BufferTab[HW_Jpeg_Decode_Info.JPEG_IN_Write_BufferIndex].State == JPEG_BUFFER_EMPTY)
  {
    if(f_read( &video_object, Jpeg_IN_BufferTab[HW_Jpeg_Decode_Info.JPEG_IN_Write_BufferIndex].DataBuffer , CHUNK_SIZE_IN, (UINT*)(&Jpeg_IN_BufferTab[HW_Jpeg_Decode_Info.JPEG_IN_Write_BufferIndex].DataBufferSize)) == FR_OK)
    {  
      Jpeg_IN_BufferTab[HW_Jpeg_Decode_Info.JPEG_IN_Write_BufferIndex].State = JPEG_BUFFER_FULL;
    }
    else
    {
			HAL_JPEG_Pause(hjpeg, JPEG_PAUSE_RESUME_INPUT_OUTPUT);
      HW_Jpeg_Decode_Info.Input_Is_Paused = 1;
			HW_Jpeg_Decode_Info.Output_Is_Paused = 1;
			HW_Jpeg_Decode_Info.Error_Flag = 1;
			return;
    }
		
    if((HW_Jpeg_Decode_Info.Input_Is_Paused == 1) && (HW_Jpeg_Decode_Info.JPEG_IN_Write_BufferIndex == HW_Jpeg_Decode_Info.JPEG_IN_Read_BufferIndex))
    {
      HW_Jpeg_Decode_Info.Input_Is_Paused = 0;
      HAL_JPEG_ConfigInputBuffer(hjpeg,Jpeg_IN_BufferTab[HW_Jpeg_Decode_Info.JPEG_IN_Read_BufferIndex].DataBuffer, Jpeg_IN_BufferTab[HW_Jpeg_Decode_Info.JPEG_IN_Read_BufferIndex].DataBufferSize); 
			
      HAL_JPEG_Resume(hjpeg, JPEG_PAUSE_RESUME_INPUT); 
    }
    
    HW_Jpeg_Decode_Info.JPEG_IN_Write_BufferIndex++;
    if(HW_Jpeg_Decode_Info.JPEG_IN_Write_BufferIndex >= NB_INPUT_DATA_BUFFERS)
    {
      HW_Jpeg_Decode_Info.JPEG_IN_Write_BufferIndex = 0;
    }            
  }
}
#endif




#if (JPEG_Decode_Mode == 0 || JPEG_Decode_Mode == 1 || JPEG_Decode_Mode == 2 )

/**
  * @brief  JPEG Info ready callback
  * @param hjpeg: JPEG handle pointer
  * @param pInfo: JPEG Info Struct pointer
  * @retval None
  */
void HAL_JPEG_InfoReadyCallback(JPEG_HandleTypeDef *hjpeg, JPEG_ConfTypeDef *pInfo)
{
  JPEG_GetDecodeColorConvertFunc(pInfo, &JPEG_ToRGB, (unsigned int *)&HW_Jpeg_Decode_Info.MCU_TotalNb);
}

/**
  * @brief  JPEG Get Data callback
  * @param hjpeg: JPEG handle pointer
  * @param NbDecodedData: Number of decoded (consummed) bytes from input buffer
  * @retval None
  */
void HAL_JPEG_GetDataCallback(JPEG_HandleTypeDef *hjpeg, uint32_t NbDecodedData)
{
	if( Jpeg_Or_AVI == 0 )
	{
		#if ( JPEG_Decode_Mode == 0 || JPEG_Decode_Mode == 1 )
			if(NbDecodedData == Jpeg_IN_BufferTab[HW_Jpeg_Decode_Info.JPEG_IN_Read_BufferIndex].DataBufferSize)
			{  
				Jpeg_IN_BufferTab[HW_Jpeg_Decode_Info.JPEG_IN_Read_BufferIndex].State = JPEG_BUFFER_EMPTY;
				Jpeg_IN_BufferTab[HW_Jpeg_Decode_Info.JPEG_IN_Read_BufferIndex].DataBufferSize = 0;
			
				HW_Jpeg_Decode_Info.JPEG_IN_Read_BufferIndex++;
				if(HW_Jpeg_Decode_Info.JPEG_IN_Read_BufferIndex >= NB_INPUT_DATA_BUFFERS)
				{
					HW_Jpeg_Decode_Info.JPEG_IN_Read_BufferIndex = 0;        
				}
			
				if(Jpeg_IN_BufferTab[HW_Jpeg_Decode_Info.JPEG_IN_Read_BufferIndex].State == JPEG_BUFFER_EMPTY)
				{
					HAL_JPEG_Pause(hjpeg, JPEG_PAUSE_RESUME_INPUT);
					HW_Jpeg_Decode_Info.Input_Is_Paused = 1;
				}
				else
				{    
					HAL_JPEG_ConfigInputBuffer(hjpeg,Jpeg_IN_BufferTab[HW_Jpeg_Decode_Info.JPEG_IN_Read_BufferIndex].DataBuffer, Jpeg_IN_BufferTab[HW_Jpeg_Decode_Info.JPEG_IN_Read_BufferIndex].DataBufferSize);    
				}
			}
			else
			{
				HAL_JPEG_ConfigInputBuffer( hjpeg,Jpeg_IN_BufferTab[HW_Jpeg_Decode_Info.JPEG_IN_Read_BufferIndex].DataBuffer + NbDecodedData, Jpeg_IN_BufferTab[HW_Jpeg_Decode_Info.JPEG_IN_Read_BufferIndex].DataBufferSize - NbDecodedData);      
			}
		#endif

		#if ( JPEG_Decode_Mode == 2 )
			unsigned int temp = 0;
			if(f_read( &video_object, Jpeg_IN_BufferTab[0].DataBuffer , NbDecodedData, (UINT*)&temp) == FR_OK)
			{  
				Jpeg_IN_BufferTab[0].State = JPEG_BUFFER_FULL;
			}
		#endif
	}
	else
	{
		if( AVI_BUF_FLAG == 0 )
		{	
			if( NbDecodedData != AVI_IN_BUF1_Len )
			{
				HAL_JPEG_ConfigInputBuffer( hjpeg, (unsigned char *)(AVI_IN_BUF1 + SDRAM_BANK1_ADDR + NbDecodedData), AVI_IN_BUF1_Len - NbDecodedData); 
			}	
		}
		else
		{
			if( NbDecodedData != AVI_IN_BUF2_Len )
			{
				HAL_JPEG_ConfigInputBuffer( hjpeg, (unsigned char *)(AVI_IN_BUF2 + SDRAM_BANK1_ADDR + NbDecodedData), AVI_IN_BUF2_Len - NbDecodedData); 
			}	
		}
	}
	//	printf("%x\r\n", NbDecodedData );
}





/**
  * @brief  JPEG Data Ready callback
  * @param hjpeg: JPEG handle pointer
  * @param pDataOut: pointer to the output data buffer
  * @param OutDataLength: length of output buffer in bytes
  * @retval None
  */
void HAL_JPEG_DataReadyCallback (JPEG_HandleTypeDef *hjpeg, uint8_t *pDataOut, uint32_t OutDataLength)
{
	#if ( JPEG_Decode_Mode == 0 || JPEG_Decode_Mode == 1  )
		Jpeg_OUT_BufferTab[HW_Jpeg_Decode_Info.JPEG_OUT_Write_BufferIndex].State = JPEG_BUFFER_FULL;
		Jpeg_OUT_BufferTab[HW_Jpeg_Decode_Info.JPEG_OUT_Write_BufferIndex].DataBufferSize = OutDataLength;
			
		HW_Jpeg_Decode_Info.JPEG_OUT_Write_BufferIndex++;
		if(HW_Jpeg_Decode_Info.JPEG_OUT_Write_BufferIndex >= NB_OUTPUT_DATA_BUFFERS)
		{
			HW_Jpeg_Decode_Info.JPEG_OUT_Write_BufferIndex = 0;        
		}

		if(Jpeg_OUT_BufferTab[HW_Jpeg_Decode_Info.JPEG_OUT_Write_BufferIndex].State != JPEG_BUFFER_EMPTY)
		{
			HAL_JPEG_Pause(hjpeg, JPEG_PAUSE_RESUME_OUTPUT);
			HW_Jpeg_Decode_Info.Output_Is_Paused = 1;
		}
		HAL_JPEG_ConfigOutputBuffer(hjpeg, Jpeg_OUT_BufferTab[HW_Jpeg_Decode_Info.JPEG_OUT_Write_BufferIndex].DataBuffer, CHUNK_SIZE_OUT); 
	#endif
	
	#if ( JPEG_Decode_Mode == 2 )
//		printf("%d\r\n", OutDataLength );
		unsigned int temp = 0;
		HW_Jpeg_Decode_Info.MCU_BlockIndex += JPEG_ToRGB(Jpeg_OUT_BufferTab[0].DataBuffer, (uint8_t *)HW_Jpeg_Decode_Info.Data_Addr, HW_Jpeg_Decode_Info.MCU_BlockIndex, OutDataLength, &temp);  
	#endif
}




/**
  * @brief  JPEG Decode complete callback
  * @param hjpeg: JPEG handle pointer
  * @retval None
  */
void HAL_JPEG_DecodeCpltCallback(JPEG_HandleTypeDef *hjpeg)
{    
	HW_Jpeg_Decode_Info.End_Flag = 1; 
}




/**
  * @brief  JPEG Error callback
  * @param hjpeg: JPEG handle pointer
  * @retval None
  */
void HAL_JPEG_ErrorCallback(JPEG_HandleTypeDef *hjpeg)
{
  HW_Jpeg_Decode_Info.Error_Flag = 1;
}

#endif

	


#if (JPEG_Decode_Mode == 0 || JPEG_Decode_Mode == 1 || JPEG_Decode_Mode == 2 )

/**
* @brief  初始化硬件JPEG编码/解码器
* @note   
* @retval NONE
*/
void HW_JPEG_Init( void )
{
	__HAL_RCC_JPEG_CLK_ENABLE();
	/** Sw reset of JPEG IP */
	__HAL_RCC_JPEG_FORCE_RESET();
	JPEG_Delay_1ms( 2 );
  __HAL_RCC_JPEG_RELEASE_RESET();
	JPEG_Delay_1ms( 2 );
	
	  /* Enable DMA clock */
  __HAL_RCC_DMA2_CLK_ENABLE();
	
	
	hjpeg.Instance = JPEG;
	HAL_JPEG_DeInit( &hjpeg );
	
	HAL_JPEG_Init( &hjpeg );
	
	HAL_NVIC_SetPriority( JPEG_IRQn, 0x0D, 0x00);
  HAL_NVIC_EnableIRQ( JPEG_IRQn );
	
	HAL_NVIC_SetPriority(DMA2_Stream3_IRQn, 0x0E, 0x00);
	HAL_NVIC_EnableIRQ(DMA2_Stream3_IRQn);
	
	HAL_NVIC_SetPriority(DMA2_Stream4_IRQn, 0x0E, 0x00);
  HAL_NVIC_EnableIRQ(DMA2_Stream4_IRQn);  
	
  
	/* Init The JPEG Look Up Tables used for YCbCr to RGB conversion   */    /* Init The JPEG Color Look Up Tables used for YCbCr to RGB conversion   */ 
  JPEG_InitColorTables();
}






/**
* @brief  硬件JPEG解码
* @param  file:  指向要解码的JPEG文件
* @param  Xpos:  在LCD上显示的起始位置--x坐标
* @param  Ypos:  在LCD上显示的起始位置--y坐标  
* @retval None
*/
void hw_jpeg_decode(unsigned char *file, unsigned short int Xpos, unsigned short int Ypos )
{
	unsigned int i;
	unsigned char temp = 0;
	unsigned int file_size;
	unsigned int time;
	
	Jpeg_Or_AVI  = 0;
	HW_Jpeg_Decode_Info.End_Flag = 0;
	HW_Jpeg_Decode_Info.Error_Flag = 0;
	HW_Jpeg_Decode_Info.Input_Is_Paused = 0;
	HW_Jpeg_Decode_Info.Output_Is_Paused =0;
	HW_Jpeg_Decode_Info.MCU_TotalNb = 0;
	HW_Jpeg_Decode_Info.Data_Length = 0;
	HW_Jpeg_Decode_Info.Data_Addr = SDRAM_BUFFER+SDRAM_BANK1_ADDR;
	HW_Jpeg_Decode_Info.JPEG_IN_Read_BufferIndex = 0;
	HW_Jpeg_Decode_Info.JPEG_IN_Write_BufferIndex = 0;
	HW_Jpeg_Decode_Info.JPEG_OUT_Read_BufferIndex = 0;
	HW_Jpeg_Decode_Info.JPEG_OUT_Write_BufferIndex = 0;

  HW_Jpeg_Decode_Info.MCU_BlockIndex = 0;
	
	if( Xpos >= LCD_With || Ypos >= LCD_Heigh ) return;
	
	if( f_open( &video_object, (const TCHAR*)file, FA_OPEN_EXISTING | FA_READ ) != FR_OK ) 
	{
		HW_Jpeg_Decode_Info.Error_Flag = 1;
		return;
	}
	
	time = ms_count;
	
	
	#if (JPEG_Decode_Mode == 0 )
		/* Input DMA */    
		/* Set the parameters to be configured */
		jpeg_hdmain.Instance = DMA2_Stream3;
		/* DeInitialize the DMA Stream */
		HAL_DMA_DeInit(&jpeg_hdmain);  
		
		jpeg_hdmain.Init.Channel = DMA_CHANNEL_9;
		jpeg_hdmain.Init.Direction = DMA_MEMORY_TO_PERIPH;
		jpeg_hdmain.Init.PeriphInc = DMA_PINC_DISABLE;
		jpeg_hdmain.Init.MemInc = DMA_MINC_ENABLE;
		jpeg_hdmain.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
		jpeg_hdmain.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
		jpeg_hdmain.Init.Mode = DMA_NORMAL;
		jpeg_hdmain.Init.Priority = DMA_PRIORITY_MEDIUM;
		jpeg_hdmain.Init.FIFOMode = DMA_FIFOMODE_ENABLE;         
		jpeg_hdmain.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
		jpeg_hdmain.Init.MemBurst = DMA_MBURST_INC4;
		jpeg_hdmain.Init.PeriphBurst = DMA_PBURST_INC4; 
		/* Initialize the DMA stream */
		HAL_DMA_Init(&jpeg_hdmain);
		
		 /* Associate the DMA handle */
		__HAL_LINKDMA(&hjpeg, hdmain, jpeg_hdmain);

		
		/* Output DMA */
		/* Set the parameters to be configured */ 
		jpeg_hdmaout.Instance = DMA2_Stream4;
		/* DeInitialize the DMA Stream */
		HAL_DMA_DeInit(&jpeg_hdmaout);  
		
		jpeg_hdmaout.Init.Channel = DMA_CHANNEL_9;
		jpeg_hdmaout.Init.Direction = DMA_PERIPH_TO_MEMORY;
		jpeg_hdmaout.Init.PeriphInc = DMA_PINC_DISABLE;
		jpeg_hdmaout.Init.MemInc = DMA_MINC_ENABLE;
		jpeg_hdmaout.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
		jpeg_hdmaout.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
		jpeg_hdmaout.Init.Mode = DMA_NORMAL;
		jpeg_hdmaout.Init.Priority = DMA_PRIORITY_HIGH;
		jpeg_hdmaout.Init.FIFOMode = DMA_FIFOMODE_ENABLE;         
		jpeg_hdmaout.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
		jpeg_hdmaout.Init.MemBurst = DMA_MBURST_INC4;
		jpeg_hdmaout.Init.PeriphBurst = DMA_PBURST_INC4;
		/* Initialize the DMA stream */
		HAL_DMA_Init(&jpeg_hdmaout);
		
		/* Associate the DMA handle */
		__HAL_LINKDMA(&hjpeg, hdmaout, jpeg_hdmaout);
	#endif
		
	#if (JPEG_Decode_Mode == 0 || JPEG_Decode_Mode == 1 )
		for( i=0; i<2; i++ )
		{
			if( f_read( &video_object, Jpeg_IN_BufferTab[i].DataBuffer, CHUNK_SIZE_IN, (unsigned int *)&Jpeg_IN_BufferTab[i].DataBufferSize ) != FR_OK )
			{
				f_close( &video_object );
				HW_Jpeg_Decode_Info.Error_Flag = 1;
				return;
			}
			
			Jpeg_IN_BufferTab[i].State = JPEG_BUFFER_FULL;
		}
		
		#if (JPEG_Decode_Mode == 0  )
			HAL_JPEG_Decode_DMA( &hjpeg ,Jpeg_IN_BufferTab[0].DataBuffer , Jpeg_IN_BufferTab[0].DataBufferSize , Jpeg_OUT_BufferTab[0].DataBuffer ,CHUNK_SIZE_OUT );
		#endif
		
		#if (JPEG_Decode_Mode == 1  )
			HAL_JPEG_Decode_IT( &hjpeg ,Jpeg_IN_BufferTab[0].DataBuffer ,Jpeg_IN_BufferTab[0].DataBufferSize , Jpeg_OUT_BufferTab[0].DataBuffer ,CHUNK_SIZE_OUT );
		#endif	
	#endif
	
	#if (JPEG_Decode_Mode == 2  )
		if( f_read( &video_object, Jpeg_IN_BufferTab[0].DataBuffer, CHUNK_SIZE_IN, (unsigned int *)&Jpeg_IN_BufferTab[0].DataBufferSize ) != FR_OK )
		{
			f_close( &video_object );
			HW_Jpeg_Decode_Info.Error_Flag = 1;
			return;
		}
		
		HAL_JPEG_Decode( &hjpeg, Jpeg_IN_BufferTab[0].DataBuffer, Jpeg_IN_BufferTab[0].DataBufferSize, Jpeg_OUT_BufferTab[0].DataBuffer ,CHUNK_SIZE_OUT, 100);
	#endif
			

	#if (JPEG_Decode_Mode == 0 || JPEG_Decode_Mode == 1 )
		while( temp == 0 )
		{
			JPEG_InputHandler(&hjpeg);
			temp = JPEG_OutputHandler(&hjpeg);
			if( HW_Jpeg_Decode_Info.Error_Flag == 1 )
			{
				f_close( &video_object );
				HAL_JPEG_Abort( &hjpeg );
				return;
			}
		}
	#endif
	
	#if (JPEG_Decode_Mode == 2 )
		if( HW_Jpeg_Decode_Info.Error_Flag == 1 )
		{
			f_close( &video_object );
			HAL_JPEG_Abort( &hjpeg );
			return;
		}
	#endif
		
	
	if( hjpeg.Conf.ImageWidth > 800 || hjpeg.Conf.ImageHeight > 480 || ( Xpos + hjpeg.Conf.ImageWidth) > 800 || ( Ypos + hjpeg.Conf.ImageHeight) > 480 ) 
	{
		HW_Jpeg_Decode_Info.Error_Flag = 1;
		f_close( &video_object );
		return;
	}
	
	
	hdma2d.Instance = DMA2D;
	HAL_DMA2D_DeInit( &hdma2d );
	
	hdma2d.Init.Mode = DMA2D_M2M;
	hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB888;
	hdma2d.Init.OutputOffset = LCD_With - hjpeg.Conf.ImageWidth -Xpos;
	hdma2d.Init.RedBlueSwap = DMA2D_RB_REGULAR;
	hdma2d.Init.AlphaInverted = DMA2D_REGULAR_ALPHA;
	
	hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB888;
	hdma2d.LayerCfg[1].InputOffset = 0;
	hdma2d.LayerCfg[1].InputAlpha = 0xFF;
	hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
	hdma2d.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA;
	hdma2d.LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR;
	
	if( HAL_DMA2D_Init( &hdma2d ) == HAL_OK )
	{
		if(HAL_DMA2D_ConfigLayer(&hdma2d, 1) == HAL_OK)
		{			
			if( HAL_DMA2D_Start( &hdma2d, SDRAM_BUFFER+SDRAM_BANK1_ADDR, LCD_LAYER1_FRAME_BUFFER, hjpeg.Conf.ImageWidth,  hjpeg.Conf.ImageHeight ) == HAL_OK )
			{
				/* Polling For DMA transfer */  
        HAL_DMA2D_PollForTransfer(&hdma2d, 10);
				
				time = ms_count - time;
				printf("%d", time );
			}
		}
	}
	
	f_close( &video_object );
}

#endif






#if (JPEG_Decode_Mode == 3 )
/**
  * @brief  Copy decompressed data to display buffer.
  * @param  Row: Output row buffer
  * @param  DataLength: Row width in output buffer
  * @retval None
  */
static uint8_t Jpeg_CallbackFunction(uint8_t* Row, uint32_t DataLength)
{
	unsigned int offset;
	
	
	offset = LCD_With * (image_heigh - line_counter - 1) * 3;
	offset += ( (LCD_With * display_ypos + display_xpos) * 3 + LCD_LAYER1_FRAME_BUFFER );
	
  
/* Configure the DMA2D Mode, Color Mode and output offset */
  hdma2d.Init.Mode         = DMA2D_M2M;
  hdma2d.Init.ColorMode    = DMA2D_OUTPUT_RGB888;
  hdma2d.Init.OutputOffset = 0;  
	hdma2d.Init.RedBlueSwap = DMA2D_RB_REGULAR;	
  
  /* Foreground Configuration */
  hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d.LayerCfg[1].InputAlpha = 0xFF;
  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB888;
  hdma2d.LayerCfg[1].InputOffset = 0;
	hdma2d.LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR;
  
  hdma2d.Instance = DMA2D; 
  
  /* DMA2D Initialization */
  if(HAL_DMA2D_Init(&hdma2d) == HAL_OK) 
  {
    if(HAL_DMA2D_ConfigLayer(&hdma2d, 1) == HAL_OK) 
    {
      if (HAL_DMA2D_Start(&hdma2d, (uint32_t)Row, (uint32_t)offset, image_with, 1) == HAL_OK)
      {
        /* Polling For DMA transfer */  
        HAL_DMA2D_PollForTransfer(&hdma2d, 10);
      }
    }
  }
	
	line_counter--;
	
  return 0;
}



	
	
	/**
  * @brief  使用LIBJPEG解码JPEG
  * @param  callback: line decoding callback
	* @param  file:  指向要解码的JPEG文件
	* @param  Xpos:  在LCD上显示的起始位置--x坐标
	* @param  Ypos:  在LCD上显示的起始位置--y坐标  
  * @retval None   1：成功		0：失败
  */
unsigned char libjpeg_jpeg_decode(unsigned char *file, unsigned short int Xpos, unsigned short int Ypos )
{ 
    
  /* Decode JPEG Image */
  JSAMPROW buffer[2] = {0}; /* Output row buffer */
  uint32_t row_stride = 0; /* physical row width in image buffer */
//	unsigned int time;
	
	
//	if( Xpos >= LCD_With || Ypos >= LCD_Heigh ) return;

//	display_xpos = Xpos;
//	display_ypos = Ypos;
	
//	if( f_open( &video_object, (const TCHAR *)file, FA_READ ) != FR_OK ) 
//	{
//		printf("打开文件失败\r\n");
//		return;
//	}
	
//	time = ms_count;
	
  buffer[0] = jpeg_buffer;
	
  /* Step 1: allocate and initialize JPEG decompression object */
  cinfo.err = jpeg_std_error(&jerr);

  /* Initialize the JPEG decompression object */  
  jpeg_create_decompress(&cinfo);
  
  jpeg_stdio_src (&cinfo, &AVI);

  /* Step 3: read image parameters with jpeg_read_header() */
  if( jpeg_read_header(&cinfo, TRUE) != JPEG_HEADER_OK )
	{
//		f_close( &video_object );
		jpeg_destroy_decompress(&cinfo);
		return 0;
	}
	
	if( cinfo.image_width > LCD_With || cinfo.image_height > LCD_Heigh || ( Xpos + cinfo.image_width ) > LCD_With || ( Ypos + cinfo.image_height ) > LCD_Heigh )
	{
//		f_close( &video_object );
		jpeg_destroy_decompress(&cinfo);
		return 0;
	}
	
	image_with = cinfo.image_width;
	image_heigh = cinfo.image_height;
	line_counter = cinfo.image_height - 1;
	row_stride = cinfo.image_width * 3;

  /* TBC */
  /* Step 4: set parameters for decompression */
  cinfo.dct_method = JDCT_FLOAT;
	cinfo.out_color_space = JCS_RGB;

  /* Step 5: start decompressor */
  if( jpeg_start_decompress(&cinfo) != TRUE )
	{
//		f_close( &video_object );
		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);
		return 0;
	}
		
  while (cinfo.output_scanline < cinfo.output_height)
  {
    (void) jpeg_read_scanlines(&cinfo, buffer, 1);
    
    /* TBC */
    if (Jpeg_CallbackFunction(buffer[0], row_stride) != 0)
    {
      break;
    }
  }
	
//	time = ms_count - time;
//	printf("%d", time );

  /* Step 6: Finish decompression */
  jpeg_finish_decompress(&cinfo);

  /* Step 7: Release JPEG decompression object */
  jpeg_destroy_decompress(&cinfo);
	
//	f_close( &video_object );
	
	return 1;
    
}

#endif





#if (JPEG_Decode_Mode == 4 )
/**
* @brief  TJPG Get Data
* @note  callback：decoding callback
* @param  jd：Blank decompressor object    
* @param  buff:    数据存储区
* @param  ndata:   读取数据的字节数  
* @retval None
*/
UINT TjpegD_Input( JDEC *jd, BYTE* buff, UINT ndata )
{
	FIL *jpeg = (FIL *)jd->device; 					//待解码的文件信息，使用FATFS中的FIL结构类型定义
	UINT rb;
	
	if( buff )
	{
		f_read( jpeg, buff, ndata, &rb);
		
		return rb;														//返回读取到的字节数目
	}
	else
	{
		return (f_lseek( jpeg, f_tell(jpeg) + ndata) == FR_OK) ? ndata : 0;				//重新定位数据点，相当删除之前n字节数据
	}
}




/**
* @brief  TJPG Output Data
* @note  callback：decoding callback
* @param  jd：Blank decompressor object
* @param  bitmap：数据存储区
* @param  rect：输出数据信息
* @retval None
*/
UINT TjpegD_Output( JDEC *jd, void* bitmap, JRECT *rect )
{
	unsigned int dat, width, height, num;
	unsigned char *temp = (unsigned char *)bitmap;
	unsigned int addr = 0;
	
	
	width = rect->right - rect->left + 1;
	height = rect->bottom - rect->top + 1;
	
	num = LCD_With - width;
	
	dat = rect->top + display_ypos;
	addr = dat * LCD_With * 3;
	dat = rect->left + display_xpos;
	addr = addr + dat * 3 + LCD_LAYER1_FRAME_BUFFER;

	
	/* Configure the DMA2D Mode, Color Mode and output offset */
  hdma2d.Init.Mode         = DMA2D_M2M;
  hdma2d.Init.ColorMode    = DMA2D_RGB888;
  hdma2d.Init.OutputOffset = num;     
  
  /* Foreground Configuration */
  hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d.LayerCfg[1].InputAlpha = 0xFF;
  hdma2d.LayerCfg[1].InputColorMode = CM_RGB888;
  hdma2d.LayerCfg[1].InputOffset = 0;
  
  hdma2d.Instance = DMA2D;
	
	 /* DMA2D Initialization */
  if(HAL_DMA2D_Init(&hdma2d) == HAL_OK) 
  {
    if(HAL_DMA2D_ConfigLayer(&hdma2d, 1) == HAL_OK) 
    {
      if (HAL_DMA2D_Start(&hdma2d, (uint32_t)temp, (uint32_t)addr, width, height) == HAL_OK)
      {
        /* Polling For DMA transfer */  
        HAL_DMA2D_PollForTransfer(&hdma2d, 10);
      }
    }
  } 
	
	return 1;
}




/**
* @brief  使用TJPG解码JPEG
* @param  file:  指向要解码的JPEG文件
* @param  Xpos:  在LCD上显示的起始位置--x坐标
* @param  Ypos:  在LCD上显示的起始位置--y坐标  
* @retval None 1--成功		0--失败
*/
unsigned char tjpglb_jpeg_decode( unsigned char *file, unsigned short int Xpos, unsigned short int Ypos )
{
	JRESULT res;
	unsigned int time;
	

	if( Xpos >= LCD_With || Ypos >= LCD_Heigh ) return 0;
	
	display_xpos = Xpos;
	display_ypos = Ypos;
	
	if( f_open( &video_object, (const TCHAR *)file, FA_READ ) != FR_OK ) 
	{
		printf("打开文件失败\r\n");
		return 0;
	}
	
	time = ms_count;
	
	res = jd_prepare(&TjpgLB_Jpeg, TjpegD_Input, jpeg_buffer, 10*1024, &video_object );
	
	if( res != JDR_OK )
	{
		f_close( &video_object );
		
		printf("解码失败\r\n");
		return 0;
	}
	
	jd_decomp(&TjpgLB_Jpeg, TjpegD_Output, 0 );
	
	time = ms_count - time;
	printf("%d", time );
	
	f_close( &video_object );
	
	return 1;
}

#endif






/**
* @brief  AVI视频帧解码
* @param  file:  存放视频帧数据
* @param  FrameSize:  视频帧的大小
* @retval None 1: 成功	0：失败
*/
unsigned char avi_video_decode(  unsigned char *file, unsigned int FrameSize )
{
	unsigned int i;
	unsigned char temp = 0;
	unsigned int file_size;
	unsigned int count;
	
	Jpeg_Or_AVI  = 1;
	HW_Jpeg_Decode_Info.End_Flag = 0;
	HW_Jpeg_Decode_Info.Error_Flag = 0;
	HW_Jpeg_Decode_Info.Input_Is_Paused = 0;
	HW_Jpeg_Decode_Info.Output_Is_Paused =0;
	HW_Jpeg_Decode_Info.MCU_TotalNb = 0;
	HW_Jpeg_Decode_Info.Data_Length = 0;
	HW_Jpeg_Decode_Info.Data_Addr = SDRAM_BUFFER+SDRAM_BANK1_ADDR;
	HW_Jpeg_Decode_Info.JPEG_IN_Read_BufferIndex = 0;
	HW_Jpeg_Decode_Info.JPEG_IN_Write_BufferIndex = 0;
	HW_Jpeg_Decode_Info.JPEG_OUT_Read_BufferIndex = 0;
	HW_Jpeg_Decode_Info.JPEG_OUT_Write_BufferIndex = 0;

	HW_Jpeg_Decode_Info.MCU_BlockIndex = 0;

	if( FrameSize == 0 ) return 0;

	hjpeg.Instance = JPEG;
	HAL_JPEG_DeInit( &hjpeg );

	HAL_JPEG_Init( &hjpeg );

	/* Input DMA */    
	/* Set the parameters to be configured */
	jpeg_hdmain.Instance = DMA2_Stream3;
	/* DeInitialize the DMA Stream */
	HAL_DMA_DeInit(&jpeg_hdmain);  
	
	jpeg_hdmain.Init.Channel = DMA_CHANNEL_9;
	jpeg_hdmain.Init.Direction = DMA_MEMORY_TO_PERIPH;
	jpeg_hdmain.Init.PeriphInc = DMA_PINC_DISABLE;
	jpeg_hdmain.Init.MemInc = DMA_MINC_ENABLE;
	jpeg_hdmain.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
	jpeg_hdmain.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
	jpeg_hdmain.Init.Mode = DMA_NORMAL;
	jpeg_hdmain.Init.Priority = DMA_PRIORITY_MEDIUM;
	jpeg_hdmain.Init.FIFOMode = DMA_FIFOMODE_ENABLE;         
	jpeg_hdmain.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
	jpeg_hdmain.Init.MemBurst = DMA_MBURST_INC4;
	jpeg_hdmain.Init.PeriphBurst = DMA_PBURST_INC4; 
	/* Initialize the DMA stream */
	HAL_DMA_Init(&jpeg_hdmain);
	
	 /* Associate the DMA handle */
	__HAL_LINKDMA(&hjpeg, hdmain, jpeg_hdmain);

	
	/* Output DMA */
	/* Set the parameters to be configured */ 
	jpeg_hdmaout.Instance = DMA2_Stream4;
	/* DeInitialize the DMA Stream */
	HAL_DMA_DeInit(&jpeg_hdmaout);  
	
	jpeg_hdmaout.Init.Channel = DMA_CHANNEL_9;
	jpeg_hdmaout.Init.Direction = DMA_PERIPH_TO_MEMORY;
	jpeg_hdmaout.Init.PeriphInc = DMA_PINC_DISABLE;
	jpeg_hdmaout.Init.MemInc = DMA_MINC_ENABLE;
	jpeg_hdmaout.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
	jpeg_hdmaout.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
	jpeg_hdmaout.Init.Mode = DMA_NORMAL;
	jpeg_hdmaout.Init.Priority = DMA_PRIORITY_HIGH;
	jpeg_hdmaout.Init.FIFOMode = DMA_FIFOMODE_ENABLE;         
	jpeg_hdmaout.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
	jpeg_hdmaout.Init.MemBurst = DMA_MBURST_INC4;
	jpeg_hdmaout.Init.PeriphBurst = DMA_PBURST_INC4;
	/* Initialize the DMA stream */
	HAL_DMA_Init(&jpeg_hdmaout);
	
	/* Associate the DMA handle */
	__HAL_LINKDMA(&hjpeg, hdmaout, jpeg_hdmaout);
	

	HAL_JPEG_Decode_DMA( &hjpeg ,file, FrameSize, Jpeg_OUT_BufferTab[0].DataBuffer ,CHUNK_SIZE_OUT );
	
	return 1;
}


	

/**
* @brief  查询解码是否完成
* @param  none
* @retval  1: 解码完成	0：正在解码
*/
unsigned char Check_JPEG_Decode( FIL *file )
{
	unsigned char temp1 = 0, temp2 = 0;
	
	
	temp1 = JPEG_OutputHandler(&hjpeg);
	
	if( HW_Jpeg_Decode_Info.Error_Flag == 1 )
	{
		HAL_JPEG_Abort( &hjpeg );
		temp2 = 1;
	}
	
	if( temp1 == 1 || temp2 == 1 ) return 1;
	
	return 0;
}





/**
* @brief  显示解码后的图片
* @param  Xpos:  在LCD上显示的起始位置--x坐标
* @param  Ypos:  在LCD上显示的起始位置--y坐标
* @retval None 
*/
void Display_Image( unsigned short int Xpos, unsigned short int Ypos )
{
	unsigned int addr;
	
	
	addr = LCD_LAYER1_FRAME_BUFFER + ( Ypos * LCD_With + Xpos ) * 3;
	
	
	hdma2d.Instance = DMA2D;
	HAL_DMA2D_DeInit( &hdma2d );

	hdma2d.Init.Mode = DMA2D_M2M;
	hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB888;
	hdma2d.Init.OutputOffset = LCD_With - hjpeg.Conf.ImageWidth;
	hdma2d.Init.RedBlueSwap = DMA2D_RB_REGULAR;
	hdma2d.Init.AlphaInverted = DMA2D_REGULAR_ALPHA;

	hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB888;
	hdma2d.LayerCfg[1].InputOffset = 0;
	hdma2d.LayerCfg[1].InputAlpha = 0xFF;
	hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
	hdma2d.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA;
	hdma2d.LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR;

	if( HAL_DMA2D_Init( &hdma2d ) == HAL_OK )
	{
		if(HAL_DMA2D_ConfigLayer(&hdma2d, 1) == HAL_OK)
		{	
			if( HAL_DMA2D_Start( &hdma2d, SDRAM_BUFFER+SDRAM_BANK1_ADDR, addr, hjpeg.Conf.ImageWidth,  hjpeg.Conf.ImageHeight ) == HAL_OK )
			{
				/* Polling For DMA transfer */  
				HAL_DMA2D_PollForTransfer(&hdma2d, 10);
			}
		}
	}
}



