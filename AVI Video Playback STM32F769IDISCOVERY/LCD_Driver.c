#include "stm32f7xx_hal.h"

#include "PC_COM.h"
#include "SDRAM.h"
#include "LCD_Driver.h"
#include "otm8009a.h"
//#include "ZiFu.h"
//#include "hz16.h"

//#include  "os.h"




#define ABS(X)  ((X) > 0 ? (X) : -(X))


#define  LCD_OTM8009A_ID 		0
/**
  * @brief  OTM8009A_480X800 Timing parameters for Portrait orientation mode
  */
#define  OTM8009A_480X800_HSYNC             ((uint16_t)63)     /* Horizontal synchronization: This value is set to limit value mentionned 
                                                                   in otm8009a spec to fit with USB functional clock configuration constraints */
#define  OTM8009A_480X800_HBP               ((uint16_t)120)     /* Horizontal back porch      */
#define  OTM8009A_480X800_HFP               ((uint16_t)120)     /* Horizontal front porch     */
#define  OTM8009A_480X800_VSYNC             ((uint16_t)12)      /* Vertical synchronization   */
#define  OTM8009A_480X800_VBP               ((uint16_t)12)      /* Vertical back porch        */
#define  OTM8009A_480X800_VFP               ((uint16_t)12)      /* Vertical front porch       */





DSI_HandleTypeDef hdsi;
DSI_VidCfgTypeDef VidCfg;
LTDC_HandleTypeDef hltdc;
DMA2D_HandleTypeDef hdma2d;


unsigned short int LCD_With;											
unsigned short int LCD_Heigh;






/**
* @brief  延时函数，延时Xms
* @note   
* @retval None
*/
void LCD_Delay_1ms( unsigned int ms )
{  
//	OS_ERR   p_err;
	
	unsigned int i, j;
	
	for( i=0; i<ms; i++ )
	for( j=0; j<100000; j++ );
	
//	OSTimeDly ( ms, OS_OPT_TIME_DLY, &p_err );
}




/**
* @brief  配置MCU与LCD的接口.
* @note   
* @retval None
*/
void LCD_GPIO_Config( void )
{
	GPIO_InitTypeDef GPIO_InitStruct;
	
	
	__HAL_RCC_GPIOJ_CLK_ENABLE();
	
	GPIO_InitStruct.Pin = GPIO_PIN_15;															//PJ15--LCD_REST（低电平复位）
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

	HAL_GPIO_Init( GPIOJ, &GPIO_InitStruct );
	
//	GPIO_InitStruct.Pin = GPIO_PIN_2;																//PJ2--DSI_TE
//	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//	GPIO_InitStruct.Pull = GPIO_NOPULL;
//	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
//	GPIO_InitStruct.Alternate = GPIO_AF13_DSI;

//	HAL_GPIO_Init( GPIOJ, &GPIO_InitStruct );
	
	HAL_GPIO_WritePin( GPIOJ, GPIO_PIN_15, GPIO_PIN_RESET );
	LCD_Delay_1ms( 200 );
	HAL_GPIO_WritePin( GPIOJ, GPIO_PIN_15, GPIO_PIN_SET );
	LCD_Delay_1ms( 100 );
}




/**
* @brief  配置DSI的工作模式.
* @param  None
* @note   
* @retval None
*/
void DSI_Config( void )
{
	unsigned int laneByteClk_kHz = 0;
	unsigned int LcdClock = 27429; /*!< LcdClk = 27429 kHz */
	DSI_PLLInitTypeDef PLLInit;
	unsigned int  VSA; /*!< Vertical start active time in units of lines */
  unsigned int  VBP; /*!< Vertical Back Porch time in units of lines */
  unsigned int  VFP; /*!< Vertical Front Porch time in units of lines */
  unsigned int  VACT; /*!< Vertical Active time in units of lines = imageSize Y in pixels to display */
  unsigned int  HSA; /*!< Horizontal start active time in units of lcdClk */
  unsigned int  HBP; /*!< Horizontal Back Porch time in units of lcdClk */
  unsigned int  HFP; /*!< Horizontal Front Porch time in units of lcdClk */
  unsigned int  HACT; /*!< Horizontal Active time in units of lcdClk = imageSize X in pixels to display */
	
	
	__HAL_RCC_DSI_CLK_ENABLE();
	/** Sw reset of DSI IP */
	__HAL_RCC_DSI_FORCE_RESET();
	LCD_Delay_1ms( 10 );
  __HAL_RCC_DSI_RELEASE_RESET();
	LCD_Delay_1ms( 10 );
	
	
	hdsi.Instance = DSI;
	HAL_DSI_DeInit(&hdsi);
	
	
	PLLInit.PLLNDIV  = 100;
	PLLInit.PLLIDF   = DSI_PLL_IN_DIV5;
  PLLInit.PLLODF  = DSI_PLL_OUT_DIV1;
  laneByteClk_kHz = 62500; /* 500 MHz / 8 = 62.5 MHz = 62500 kHz */
	
	hdsi.Init.NumberOfLanes = DSI_TWO_DATA_LANES;
	/* TXEscapeCkdiv = f(LaneByteClk)/15.62 = 4 */
	hdsi.Init.TXEscapeCkdiv = laneByteClk_kHz/15620;
	HAL_DSI_Init( &hdsi, &PLLInit );
	
	
	/* The following values are same for portrait and landscape orientations */
  VSA  = OTM8009A_480X800_VSYNC;        /* 12  */
  VBP  = OTM8009A_480X800_VBP;          /* 12  */
  VFP  = OTM8009A_480X800_VFP;          /* 12  */
  HSA  = OTM8009A_480X800_HSYNC;        /* 63  */
  HBP  = OTM8009A_480X800_HBP;          /* 120 */
  HFP  = OTM8009A_480X800_HFP;          /* 120 */  
	HACT = LCD_With;
	VACT = LCD_Heigh;
	
	VidCfg.VirtualChannelID = LCD_OTM8009A_ID;
  VidCfg.ColorCoding = DSI_RGB888;
  VidCfg.VSPolarity = DSI_VSYNC_ACTIVE_HIGH;
  VidCfg.HSPolarity = DSI_HSYNC_ACTIVE_HIGH;
  VidCfg.DEPolarity = DSI_DATA_ENABLE_ACTIVE_HIGH;  
  VidCfg.Mode = DSI_VID_MODE_BURST; /* Mode Video burst ie : one LgP per line */
  VidCfg.NullPacketSize = 0xFFF;
  VidCfg.NumberOfChunks = 0;
  VidCfg.PacketSize                = HACT; /* Value depending on display orientation choice portrait/landscape */ 
  VidCfg.HorizontalSyncActive      = (HSA * laneByteClk_kHz)/LcdClock;
  VidCfg.HorizontalBackPorch       = (HBP * laneByteClk_kHz)/LcdClock;
  VidCfg.HorizontalLine            = ((HACT + HSA + HBP + HFP) * laneByteClk_kHz)/LcdClock; /* Value depending on display orientation choice portrait/landscape */
  VidCfg.VerticalSyncActive        = VSA;
  VidCfg.VerticalBackPorch         = VBP;
  VidCfg.VerticalFrontPorch        = VFP;
  VidCfg.VerticalActive            = VACT; /* Value depending on display orientation choice portrait/landscape */
	
	/* Enable or disable sending LP command while streaming is active in video mode */
  VidCfg.LPCommandEnable = DSI_LP_COMMAND_ENABLE; /* Enable sending commands in mode LP (Low Power) */

  /* Largest packet size possible to transmit in LP mode in VSA, VBP, VFP regions */
  /* Only useful when sending LP packets is allowed while streaming is active in video mode */
  VidCfg.LPLargestPacketSize = 16;

  /* Largest packet size possible to transmit in LP mode in HFP region during VACT period */
  /* Only useful when sending LP packets is allowed while streaming is active in video mode */
  VidCfg.LPVACTLargestPacketSize = 0;

  /* Specify for each region of the video frame, if the transmission of command in LP mode is allowed in this region */
  /* while streaming is active in video mode                                                                         */
  VidCfg.LPHorizontalFrontPorchEnable = DSI_LP_HFP_ENABLE;   /* Allow sending LP commands during HFP period */
  VidCfg.LPHorizontalBackPorchEnable  = DSI_LP_HBP_ENABLE;   /* Allow sending LP commands during HBP period */
  VidCfg.LPVerticalActiveEnable = DSI_LP_VACT_ENABLE;  /* Allow sending LP commands during VACT period */
  VidCfg.LPVerticalFrontPorchEnable = DSI_LP_VFP_ENABLE;   /* Allow sending LP commands during VFP period */
  VidCfg.LPVerticalBackPorchEnable = DSI_LP_VBP_ENABLE;   /* Allow sending LP commands during VBP period */
  VidCfg.LPVerticalSyncActiveEnable = DSI_LP_VSYNC_ENABLE; /* Allow sending LP commands during VSync = VSA period */

  /* Configure DSI Video mode timings with settings set above */
  HAL_DSI_ConfigVideoMode(&hdsi, &VidCfg);
}
	






/**
* @brief  配置LTDC的工作模式.
* @note   
* @retval None
*/
void LTDC_Config( void )
{
	RCC_PeriphCLKInitTypeDef PeriphClkInit;
	
	__HAL_RCC_LTDC_CLK_ENABLE();
	/** Sw reset of LTDC IP */
  __HAL_RCC_LTDC_FORCE_RESET();
	LCD_Delay_1ms( 10 );
  __HAL_RCC_LTDC_RELEASE_RESET();
	LCD_Delay_1ms( 10 );
	
	__HAL_RCC_DMA2D_CLK_ENABLE();
	/** Sw reset of DAM2D IP */
	__HAL_RCC_DMA2D_FORCE_RESET();
	LCD_Delay_1ms( 10 );
  __HAL_RCC_DMA2D_RELEASE_RESET();
	LCD_Delay_1ms( 10 );
	
	
	/*	配置LCD的时钟	*/
	/* RK043FN48H LCD clock configuration */
  /* PLLSAI_VCO Input = HSE_VALUE/PLL_M = 1 Mhz */
  /* PLLSAI_VCO Output = PLLSAI_VCO Input * PLLSAIN = 384 Mhz */
  /* PLLLCDCLK = PLLSAI_VCO Output/PLLSAIR = 384/7 = 54.85 Mhz */
  /* LTDC clock frequency = PLLLCDCLK / LTDC_PLLSAI_DIVR_2 = 54.85/2 = 27.429Mhz */
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_LTDC;
	
	PeriphClkInit.PLLSAI.PLLSAIN = 384;
	PeriphClkInit.PLLSAI.PLLSAIQ = 8;
	PeriphClkInit.PLLSAI.PLLSAIR = 7;
	PeriphClkInit.PLLSAI.PLLSAIP = 8;
	
	PeriphClkInit.PLLSAIDivR = RCC_PLLSAIDIVR_2;
	
	HAL_RCCEx_PeriphCLKConfig( &PeriphClkInit );
	
	/*	配置LTDC的工作模式	*/
	hltdc.Instance = LTDC;
	
//	hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
//	hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
//	hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
	hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
	hltdc.Init.HorizontalSync = (OTM8009A_480X800_HSYNC - 1);
//	hltdc.Init.VerticalSync = (RK043FN48H_VSYNC - 1);
	hltdc.Init.AccumulatedHBP = (OTM8009A_480X800_HSYNC + OTM8009A_480X800_HBP - 1);
//	hltdc.Init.AccumulatedVBP = (RK043FN48H_VSYNC + RK043FN48H_VBP - 1);
	hltdc.Init.AccumulatedActiveW = (LCD_With + OTM8009A_480X800_HSYNC + OTM8009A_480X800_HBP - 1);
//	hltdc.Init.AccumulatedActiveH = (RK043FN48H_HEIGHT + RK043FN48H_VSYNC + RK043FN48H_VBP - 1);
	hltdc.Init.TotalWidth = (LCD_With + OTM8009A_480X800_HSYNC + OTM8009A_480X800_HBP + OTM8009A_480X800_HFP - 1);
//	hltdc.Init.TotalHeigh = (RK043FN48H_HEIGHT + RK043FN48H_VSYNC + RK043FN48H_VBP + RK043FN48H_VFP - 1);
	hltdc.Init.Backcolor.Blue = 0;
	hltdc.Init.Backcolor.Green = 0;
	hltdc.Init.Backcolor.Red = 0;
	
	hltdc.LayerCfg[0].ImageWidth = LCD_With;
	hltdc.LayerCfg[0].ImageHeight = LCD_Heigh;
	hltdc.LayerCfg[1].ImageWidth = LCD_With;
	hltdc.LayerCfg[1].ImageHeight = LCD_Heigh;
	
	/* Get LTDC Configuration from DSI Configuration */
	HAL_LTDC_StructInitFromVideoConfig( &hltdc, &VidCfg);
	/* Initialize the LTDC */  
	HAL_LTDC_Init( &hltdc );
	
//	/* Set LTDC Interrupt to the lowest priority */
//  HAL_NVIC_SetPriority(LTDC_IRQn, 0x0E, 0);
//  /* Enable LTDC Interrupt */
//  HAL_NVIC_EnableIRQ(LTDC_IRQn);
//	
	HAL_LTDC_EnableDither( &hltdc );

	 /* Enable the DSI host and wrapper after the LTDC initialization
     To avoid any synchronization issue, the DSI shall be started after enabling the LTDC */
  HAL_DSI_Start(&hdsi);
}
	
	
	
	
	
/**
* @brief  配置LCD_Layer0的工作模式.
* @note   
* @retval None
*/
void LCD_Layer0_Config( void )
{
	LTDC_LayerCfgTypeDef pLayerCfg;
	
	pLayerCfg.WindowX0 = 0;
	pLayerCfg.WindowX1 = LCD_With-1;
	pLayerCfg.WindowY0 = 0;
	pLayerCfg.WindowY1 = LCD_Heigh-1;
	pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB888;
	pLayerCfg.Alpha = 255;
	pLayerCfg.Alpha0 = 0X00;
	pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
	pLayerCfg.FBStartAdress = LCD_LAYER0_FRAME_BUFFER;
	pLayerCfg.ImageWidth = LCD_With;
	pLayerCfg.ImageHeight = LCD_Heigh;
	pLayerCfg.Backcolor.Blue = 0;
	pLayerCfg.Backcolor.Green = 0;
	pLayerCfg.Backcolor.Red = 0;
	
	HAL_LTDC_ConfigLayer( &hltdc, &pLayerCfg,  0 );
	
	__HAL_LTDC_LAYER_ENABLE( &hltdc, 0 );
	
	__HAL_LTDC_RELOAD_CONFIG(&hltdc);
}

	
	
	
	
/**
* @brief  配置LCD_Layer1的工作模式.
* @note   
* @retval None
*/
void LCD_Layer1_Config( void )
{
	LTDC_LayerCfgTypeDef pLayerCfg;
	
	pLayerCfg.WindowX0 = 0;
	pLayerCfg.WindowX1 = LCD_With-1;
	pLayerCfg.WindowY0 = 0;
	pLayerCfg.WindowY1 = LCD_Heigh-1;
	pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB888;
	pLayerCfg.Alpha = 255;
	pLayerCfg.Alpha0 = 0X00;
	pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
	pLayerCfg.FBStartAdress = LCD_LAYER1_FRAME_BUFFER;
	pLayerCfg.ImageWidth = LCD_With;
	pLayerCfg.ImageHeight = LCD_Heigh;
	pLayerCfg.Backcolor.Blue = 0;
	pLayerCfg.Backcolor.Green = 0;
	pLayerCfg.Backcolor.Red = 0;
	
	HAL_LTDC_ConfigLayer( &hltdc, &pLayerCfg,  1 );
	
	__HAL_LTDC_LAYER_ENABLE( &hltdc, 1 );
	
	__HAL_LTDC_RELOAD_CONFIG(&hltdc);
}




///**
//  * @brief  Enables the display.
//  * @retval None
//  */
//void LCD_DisplayOn( void )
//{
//  /* Display On */
//  HAL_GPIO_WritePin( GPIOI, GPIO_PIN_12, GPIO_PIN_SET );        /* Assert LCD_DISP pin */
//  HAL_GPIO_WritePin( GPIOK, GPIO_PIN_3, GPIO_PIN_SET );  				/* Assert LCD_BL_CTRL pin */
//}






///**
//  * @brief  Disables the display.
//  * @retval None
//  */
//void LCD_DisplayOff(void)
//{
//  /* Display Off */
//  HAL_GPIO_WritePin( GPIOI, GPIO_PIN_12, GPIO_PIN_RESET );      /* De-assert LCD_DISP pin */
//  HAL_GPIO_WritePin( GPIOK, GPIO_PIN_3, GPIO_PIN_RESET );				/* De-assert LCD_BL_CTRL pin */
//}




/**
  * @brief  Configures the transparency.
  * @param  LayerIndex: Layer foreground or background.
  * @param  Transparency: Transparency
  *           This parameter must be a number between Min_Data = 0x00 and Max_Data = 0xFF 
  * @retval None
  */
void LCD_SetTransparency( unsigned int LayerIndex, unsigned char Transparency )
{
	__HAL_LTDC_LAYER_DISABLE( &hltdc, LayerIndex );
	
  HAL_LTDC_SetAlpha( &hltdc, Transparency, LayerIndex );
	__HAL_LTDC_RELOAD_CONFIG(&hltdc);
	
	__HAL_LTDC_LAYER_ENABLE( &hltdc, LayerIndex );
	HAL_LTDC_EnableDither( &hltdc );
}





/**
* @brief  LCD的初始化函数
* @param	orientation：显示方向（1：横屏显示		0：竖屏显示）
* @note   
* @retval None
*/
void LCD_Init( unsigned char orientation )
{
	if( orientation == 0 )
	{
		LCD_With = 480;
		LCD_Heigh = 800;
	}
	else
	{
		LCD_With = 800;
		LCD_Heigh = 480;
	}
		
	LCD_GPIO_Config( );																			// 配置MCU与LCD的接口
	DSI_Config( );																					// 配置DSI的工作模式.
	LTDC_Config( );																					// 配置LTDC的工作模式.
	OTM8009A_Init( OTM8009A_FORMAT_RGB888,orientation );		// 配置LCD工作参数
	LCD_Layer0_Config( );																		// 配置LCD_Layer0的工作模式.
	LCD_Layer1_Config( );																		// 配置LCD_Layer1的工作模式.
	LCD_SetTransparency( 0, 255 );
	LCD_SetTransparency( 1, 255 );
	LCD_SetLayerVisible( 0, 0 );
	LCD_Clear( 1, LCD_COLOR_BLACK );
}




/**
  * @brief  Fills a buffer.
  * @param  LayerIndex: Layer index
  * @param  pDst: Pointer to destination buffer
  * @param  xSize: Buffer width
  * @param  ySize: Buffer height
  * @param  OffLine: Offset
  * @param  ColorIndex: Color index
  * @retval None
  */
static void FillBuffer( unsigned int LayerIndex, void *pDst, unsigned int xSize, unsigned int ySize, unsigned int OffLine, unsigned int ColorIndex) 
{ 
  hdma2d.Init.Mode  = DMA2D_R2M;
	hdma2d.Init.ColorMode = DMA2D_RGB888;
  hdma2d.Init.OutputOffset = OffLine;  
	
//	hdma2d.LayerCfg[LayerIndex].AlphaMode = DMA2D_NO_MODIF_ALPHA;
//	hdma2d.LayerCfg[LayerIndex].InputColorMode = CM_ARGB8888;
//	hdma2d.LayerCfg[LayerIndex].InputOffset = 0;
  
  hdma2d.Instance = DMA2D;
  
  /* DMA2D Initialization */
  if(HAL_DMA2D_Init(&hdma2d) == HAL_OK) 
  {
    if(HAL_DMA2D_ConfigLayer(&hdma2d, LayerIndex) == HAL_OK) 
    {
      if (HAL_DMA2D_Start(&hdma2d, ColorIndex, (uint32_t)pDst, xSize, ySize) == HAL_OK)
      {
        /* Polling For DMA transfer */  
        HAL_DMA2D_PollForTransfer( &hdma2d, 10);
      }
    }
  } 
}





///**
//  * @brief  Converts a line to an RGB888 pixel format.
//  * @param  pSrc: Pointer to source buffer
//  * @param  pDst: Output color
//  * @param  xSize: Buffer width
//  * @param  ColorMode: Input color mode   
//  * @retval None
//  */
//static void LL_ConvertLineToRGB888( unsigned int LayerIndex, void *pSrc, void *pDst, uint32_t xSize, uint32_t ColorMode)
//{    
//  /* Configure the DMA2D Mode, Color Mode and output offset */
//  hdma2d.Init.Mode         = DMA2D_M2M_PFC;
//  hdma2d.Init.ColorMode    = DMA2D_RGB888;
//  hdma2d.Init.OutputOffset = 0;     
//  
//  /* Foreground Configuration */
//  hdma2d.LayerCfg[LayerIndex].AlphaMode = DMA2D_NO_MODIF_ALPHA;
//  hdma2d.LayerCfg[LayerIndex].InputAlpha = 0xFF;
//  hdma2d.LayerCfg[LayerIndex].InputColorMode = ColorMode;
//  hdma2d.LayerCfg[LayerIndex].InputOffset = 0;
//  
//  hdma2d.Instance = DMA2D; 
//  
//  /* DMA2D Initialization */
//  if(HAL_DMA2D_Init(&hdma2d) == HAL_OK) 
//  {
//    if(HAL_DMA2D_ConfigLayer(&hdma2d, 1) == HAL_OK) 
//    {
//      if (HAL_DMA2D_Start(&hdma2d, (uint32_t)pSrc, (uint32_t)pDst, xSize, 1) == HAL_OK)
//      {
//        /* Polling For DMA transfer */  
//        HAL_DMA2D_PollForTransfer(&hdma2d, 10);
//      }
//    }
//  } 
//}






/**
  * @brief  Sets an LCD Layer visible
  * @param  LayerIndex: Visible Layer
  * @param  State: New state of the specified layer
  *          This parameter can be one of the following values:
  *            @arg 1
  *            @arg 0 
  * @retval None
  */
void LCD_SetLayerVisible( unsigned int LayerIndex, unsigned char State)
{
  if(State == 1)
  {
    __HAL_LTDC_LAYER_ENABLE(&hltdc, LayerIndex);
  }
  else
  {
    __HAL_LTDC_LAYER_DISABLE(&hltdc, LayerIndex);
  }
  __HAL_LTDC_RELOAD_CONFIG(&hltdc);
}





/**
  * @brief  Sets display window.
  * @param  LayerIndex: Layer index
  * @param  Xpos: LCD X position
  * @param  Ypos: LCD Y position
  * @param  Width: LCD window width
  * @param  Height: LCD window height  
  * @retval None
  */
void LCD_SetLayerWindow( unsigned short int LayerIndex, unsigned short int Xpos, unsigned short int Ypos, unsigned short int Width, unsigned short int Height)
{
  /* Reconfigure the layer size */
  HAL_LTDC_SetWindowSize(&hltdc, Width, Height, LayerIndex);
  
  /* Reconfigure the layer position */
  HAL_LTDC_SetWindowPosition(&hltdc, Xpos, Ypos, LayerIndex); 
}





/**
  * @brief  Draws a pixel on LCD.
  * @param  Xpos: X position
  * @param  Ypos: Y position
  * @param  RGB_Code: Pixel color in RGB mode (8-8-8)
  * @retval None
  */
void LCD_DrawPixel( unsigned int LayerIndex, unsigned short int Xpos, unsigned short int Ypos, unsigned int Color)
{
	unsigned int  Xaddress = 0;
  
  Xaddress = (hltdc.LayerCfg[LayerIndex].FBStartAdress) + 3*( LCD_With*Ypos + Xpos);
	FillBuffer(LayerIndex, (unsigned int *)Xaddress, 1, 1, 0, Color );
	
//	*(__IO uint32_t*) (hltdc.LayerCfg[LayerIndex].FBStartAdress + (3*(Ypos*LCD_With + Xpos))) = Color;
}




/**
  * @brief  Reads an LCD pixel.
  * @param  Xpos: X position 
  * @param  Ypos: Y position 
  * @retval RGB pixel color
  */
unsigned int LCD_ReadPixel( unsigned int LayerIndex, unsigned short int Xpos, unsigned short int Ypos)
{
  unsigned int addr, ret, temp;
	unsigned char buf[3];
	
	temp = LCD_With;
	temp = Ypos*temp;
	temp += Xpos;
	temp = 3*temp;
	
	addr = hltdc.LayerCfg[LayerIndex].FBStartAdress + temp - SDRAM_BANK1_ADDR;

	SDRAM_Read_8B( addr, buf, 3 );
	
	ret = buf[0];
	ret = ret << 8 | buf[1];
	ret = ret << 8 | buf[2];
	
  return ret;
}





/**
  * @brief  Clears the hole LCD.
  * @param  Color: Color of the background
  * @retval None
  */
void LCD_Clear( unsigned int LayerIndex, unsigned int Color)
{ 
	/* Clear the LCD */ 
	FillBuffer( LayerIndex, (unsigned int *)(hltdc.LayerCfg[LayerIndex].FBStartAdress), LCD_With,  LCD_Heigh, 0, Color );
}




/**
  * @brief  Draws an horizontal line.
  * @param  Xpos: X position
  * @param  Ypos: Y position
  * @param  Length: Line length
  * @retval None
  */
void LCD_DrawHLine( unsigned int LayerIndex, unsigned short int Xpos, unsigned short int Ypos, unsigned short int Length, unsigned int Color )
{
  unsigned int  Xaddress = 0;
  
  Xaddress = (hltdc.LayerCfg[LayerIndex].FBStartAdress) + 3*( LCD_With*Ypos + Xpos);
  
  /* Write line */
  FillBuffer(LayerIndex, (uint32_t *)Xaddress, Length, 1, 0, Color );
}




/**
  * @brief  Draws a vertical line.
  * @param  Xpos: X position
  * @param  Ypos: Y position
  * @param  Length: Line length
  * @retval None
  */
//void LCD_DrawVLine( unsigned int LayerIndex, unsigned short int Xpos, unsigned short int Ypos, unsigned short int Length, unsigned int Color )
//{
//  unsigned int  Xaddress = 0;
//  
//  Xaddress = (hltdc.LayerCfg[LayerIndex].FBStartAdress) + 3*(RK043FN48H_WIDTH*Ypos + Xpos);
//  
//  /* Write line */
//  FillBuffer( LayerIndex, (uint32_t *)Xaddress, 1, Length, (RK043FN48H_WIDTH - 1), Color );
//}




/**
  * @brief  Draws an uni-line (between two points).
  * @param  x1: Point 1 X position
  * @param  y1: Point 1 Y position
  * @param  x2: Point 2 X position
  * @param  y2: Point 2 Y position
  * @retval None
  */
//void LCD_DrawLine( unsigned int LayerIndex, unsigned short int x1, unsigned short int y1, unsigned short int x2, unsigned short int y2, unsigned int Color )
//{
//  short int deltax = 0, deltay = 0, x = 0, y = 0, xinc1 = 0, xinc2 = 0, 
//  yinc1 = 0, yinc2 = 0, den = 0, num = 0, num_add = 0, num_pixels = 0, 
//  curpixel = 0;
//  
//  deltax = ABS(x2 - x1);        /* The difference between the x's */
//  deltay = ABS(y2 - y1);        /* The difference between the y's */
//  x = x1;                       /* Start x off at the first pixel */
//  y = y1;                       /* Start y off at the first pixel */
//  
//  if (x2 >= x1)                 /* The x-values are increasing */
//  {
//    xinc1 = 1;
//    xinc2 = 1;
//  }
//  else                          /* The x-values are decreasing */
//  {
//    xinc1 = -1;
//    xinc2 = -1;
//  }
//  
//  if (y2 >= y1)                 /* The y-values are increasing */
//  {
//    yinc1 = 1;
//    yinc2 = 1;
//  }
//  else                          /* The y-values are decreasing */
//  {
//    yinc1 = -1;
//    yinc2 = -1;
//  }
//  
//  if (deltax >= deltay)         /* There is at least one x-value for every y-value */
//  {
//    xinc1 = 0;                  /* Don't change the x when numerator >= denominator */
//    yinc2 = 0;                  /* Don't change the y for every iteration */
//    den = deltax;
//    num = deltax / 2;
//    num_add = deltay;
//    num_pixels = deltax;         /* There are more x-values than y-values */
//  }
//  else                          /* There is at least one y-value for every x-value */
//  {
//    xinc2 = 0;                  /* Don't change the x for every iteration */
//    yinc1 = 0;                  /* Don't change the y when numerator >= denominator */
//    den = deltay;
//    num = deltay / 2;
//    num_add = deltax;
//    num_pixels = deltay;         /* There are more y-values than x-values */
//  }
//  
//  for (curpixel = 0; curpixel <= num_pixels; curpixel++)
//  {
//    LCD_DrawPixel(LayerIndex, x, y, Color );   /* Draw the current pixel */
//    num += num_add;                            /* Increase the numerator by the top of the fraction */
//    if (num >= den)                           /* Check if numerator >= denominator */
//    {
//      num -= den;                             /* Calculate the new numerator value */
//      x += xinc1;                             /* Change the x as appropriate */
//      y += yinc1;                             /* Change the y as appropriate */
//    }
//    x += xinc2;                               /* Change the x as appropriate */
//    y += yinc2;                               /* Change the y as appropriate */
//  }
//}




///**
//  * @brief  Draws a rectangle.
//  * @param  Xpos: X position
//  * @param  Ypos: Y position
//  * @param  Width: Rectangle width  
//  * @param  Height: Rectangle height
//  * @retval None
//  */
//void LCD_DrawRect(unsigned int LayerIndex, unsigned short int Xpos, unsigned short int Ypos, unsigned short int Width, unsigned short int Height, unsigned int Color)
//{
//  /* Draw horizontal lines */
//  LCD_DrawHLine( LayerIndex, Xpos, Ypos, Width, Color );
//  LCD_DrawHLine( LayerIndex, Xpos, (Ypos+ Height), Width, Color );
//  
//  /* Draw vertical lines */
//  LCD_DrawVLine( LayerIndex, Xpos, Ypos, Height, Color);
//  LCD_DrawVLine( LayerIndex, (Xpos + Width), Ypos, Height, Color );
//}





///**
//  * @brief  Draws a full rectangle.
//  * @param  Xpos: X position
//  * @param  Ypos: Y position
//  * @param  Width: Rectangle width  
//  * @param  Height: Rectangle height
//  * @retval None
//  */
//void LCD_FillRect( unsigned int LayerIndex, unsigned short int Xpos, unsigned short int Ypos, unsigned short int Width, unsigned short int Height, unsigned int Color )
//{
//  unsigned int  x_address = 0;
//  
//	x_address = (hltdc.LayerCfg[LayerIndex].FBStartAdress) + 3*(RK043FN48H_WIDTH*Ypos + Xpos);

//  /* Fill the rectangle */
//  FillBuffer(LayerIndex, (unsigned int *)x_address, Width, Height, (RK043FN48H_WIDTH - Width), Color );
//}






/**
  * @brief  Draws a circle.
  * @param  Xpos: X position
  * @param  Ypos: Y position
  * @param  Radius: Circle radius
  * @retval None
  */
void LCD_DrawCircle( unsigned int LayerIndex, unsigned short int Xpos, unsigned short int Ypos, unsigned short int Radius, unsigned int Color )
{
  int   decision;    /* Decision Variable */ 
  unsigned int  current_x;   /* Current X Value */
  unsigned int  current_y;   /* Current Y Value */
  
  decision = 3 - (Radius << 1);
  current_x = 0;
  current_y = Radius;
  
  while (current_x <= current_y)
  {
    LCD_DrawPixel( LayerIndex, (Xpos + current_x), (Ypos - current_y), Color );
    
    LCD_DrawPixel( LayerIndex, (Xpos - current_x), (Ypos - current_y), Color );
    
    LCD_DrawPixel( LayerIndex, (Xpos + current_y), (Ypos - current_x), Color );
    
    LCD_DrawPixel( LayerIndex, (Xpos - current_y), (Ypos - current_x), Color );
    
    LCD_DrawPixel( LayerIndex, (Xpos + current_x), (Ypos + current_y), Color );
    
    LCD_DrawPixel( LayerIndex, (Xpos - current_x), (Ypos + current_y), Color );
    
    LCD_DrawPixel( LayerIndex, (Xpos + current_y), (Ypos + current_x), Color );
    
    LCD_DrawPixel( LayerIndex, (Xpos - current_y), (Ypos + current_x), Color );
    
    if (decision < 0)
    { 
      decision += (current_x << 2) + 6;
    }
    else
    {
      decision += ((current_x - current_y) << 2) + 10;
      current_y--;
    }
    current_x++;
  } 
}






/**
  * @brief  Draws a full circle.
  * @param  Xpos: X position
  * @param  Ypos: Y position
  * @param  Radius: Circle radius
  * @retval None
  */
void LCD_FillCircle( unsigned int LayerIndex, unsigned short int Xpos, unsigned short int Ypos, unsigned short int Radius, unsigned int Color )
{
  int  decision;     /* Decision Variable */ 
  unsigned int  current_x;   /* Current X Value */
  unsigned int  current_y;   /* Current Y Value */
  
  decision = 3 - (Radius << 1);
  
  current_x = 0;
  current_y = Radius;
  
  
  while (current_x <= current_y)
  {
    if(current_y > 0) 
    {
      LCD_DrawHLine( LayerIndex, Xpos - current_y, Ypos + current_x, 2*current_y, Color );
			LCD_DrawHLine( LayerIndex, Xpos - current_y, Ypos - current_x, 2*current_y, Color );
    }
    
    if(current_x > 0) 
    {
			LCD_DrawHLine( LayerIndex, Xpos - current_x, Ypos - current_y, 2*current_x, Color);
			LCD_DrawHLine( LayerIndex, Xpos - current_x, Ypos + current_y, 2*current_x, Color );
    }
    if (decision < 0)
    { 
      decision += (current_x << 2) + 6;
    }
    else
    {
      decision += ((current_x - current_y) << 2) + 10;
      current_y--;
    }
    current_x++;
  }
  
  LCD_DrawCircle( LayerIndex, Xpos, Ypos, Radius, Color );
}





/**
  * @brief  Draws an poly-line (between many points).
  * @param  Points: Pointer to the points array
  * @param  PointCount: Number of points
  * @retval None
  */
//void LCD_DrawPolygon( unsigned int LayerIndex, pPoint Points, unsigned short int PointCount, unsigned int Color )
//{
//  short int x = 0, y = 0;
//  
//  if(PointCount < 2)
//  {
//    return;
//  }
//  
//  LCD_DrawLine( LayerIndex, Points->X, Points->Y, (Points+PointCount-1)->X, (Points+PointCount-1)->Y, Color );
//  
//  while(--PointCount)
//  {
//    x = Points->X;
//    y = Points->Y;
//    Points++;
//    LCD_DrawLine( LayerIndex, x, y, Points->X, Points->Y, Color );
//  }
//}





///**
//  * @brief  Draws an ellipse on LCD.
//  * @param  Xpos: X position
//  * @param  Ypos: Y position
//  * @param  XRadius: Ellipse X radius
//  * @param  YRadius: Ellipse Y radius
//  * @retval None
//  */
//void LCD_DrawEllipse( unsigned int LayerIndex, unsigned short int Xpos, unsigned short int Ypos, unsigned short int XRadius, unsigned short int YRadius, unsigned int Color)
//{
//  int x = 0, y = -YRadius, err = 2-2*XRadius, e2;
//  float k = 0, rad1 = 0, rad2 = 0;
//  
//  rad1 = XRadius;
//  rad2 = YRadius;
//  
//  k = (float)(rad2/rad1);  
//  
//  do { 
//    LCD_DrawPixel( LayerIndex, (Xpos-(uint16_t)(x/k)), (Ypos+y), Color );
//    LCD_DrawPixel( LayerIndex, (Xpos+(uint16_t)(x/k)), (Ypos+y), Color );
//    LCD_DrawPixel( LayerIndex, (Xpos+(uint16_t)(x/k)), (Ypos-y), Color );
//    LCD_DrawPixel( LayerIndex, (Xpos-(uint16_t)(x/k)), (Ypos-y), Color );      
//    
//    e2 = err;
//    if (e2 <= x) {
//      err += ++x*2+1;
//      if (-y == x && e2 <= y) e2 = 0;
//    }
//    if (e2 > y) err += ++y*2+1;     
//  }
//  while (y <= 0);
//}




/**
  * @brief  Draws a full ellipse.
  * @param  Xpos: X position
  * @param  Ypos: Y position
  * @param  XRadius: Ellipse X radius
  * @param  YRadius: Ellipse Y radius  
  * @retval None
  */
//void LCD_FillEllipse( unsigned int LayerIndex, int Xpos, int Ypos, int XRadius, int YRadius, unsigned int Color )
//{
//  int x = 0, y = -YRadius, err = 2-2*XRadius, e2;
//  float k = 0, rad1 = 0, rad2 = 0;
//  
//  rad1 = XRadius;
//  rad2 = YRadius;
//  
//  k = (float)(rad2/rad1);
//  
//  do 
//  {       
//    LCD_DrawHLine( LayerIndex, (Xpos-(uint16_t)(x/k)), (Ypos+y), (2*(uint16_t)(x/k) + 1), Color );
//    LCD_DrawHLine( LayerIndex, (Xpos-(uint16_t)(x/k)), (Ypos-y), (2*(uint16_t)(x/k) + 1), Color );
//    
//    e2 = err;
//    if (e2 <= x) 
//    {
//      err += ++x*2+1;
//      if (-y == x && e2 <= y) e2 = 0;
//    }
//    if (e2 > y) err += ++y*2+1;
//  }
//  while (y <= 0);
//}




///**
//  * @brief  Draws a bitmap picture loaded in the internal Flash in RGB888 format (24 bits per pixel).
//  * @param  Xpos: Bmp X position in the LCD
//  * @param  Ypos: Bmp Y position in the LCD
//  * @param  pbmp: Pointer to Bmp picture address in the internal Flash
//  * @retval None
//  */
//void LCD_DrawBitmap( unsigned int LayerIndex, unsigned int Xpos, unsigned int Ypos, unsigned char *pbmp )
//{
//  unsigned int index = 0, width = 0, height = 0, bit_pixel = 0;
//  unsigned int address;
//  unsigned int input_color_mode = 0;
//  
//  /* Get bitmap data address offset */
//  index = *(__IO uint16_t *) (pbmp + 10);
//  index |= (*(__IO uint16_t *) (pbmp + 12)) << 16;
//  
//  /* Read bitmap width */
//  width = *(uint16_t *) (pbmp + 18);
//  width |= (*(uint16_t *) (pbmp + 20)) << 16;
//  
//  /* Read bitmap height */
//  height = *(uint16_t *) (pbmp + 22);
//  height |= (*(uint16_t *) (pbmp + 24)) << 16; 
//  
//  /* Read bit/pixel */
//  bit_pixel = *(uint16_t *) (pbmp + 28);   
//  
//  /* Set the address */
//  address =hltdc.LayerCfg[LayerIndex].FBStartAdress + ((( RK043FN48H_WIDTH*Ypos) + Xpos)*(3));
//	
//	/* Get the layer pixel format */    
//	input_color_mode = CM_RGB888;
//  
//  /* Bypass the bitmap header */
//  pbmp += (index + (width * (height - 1) * (bit_pixel/8)));  
//  
//  /* Convert picture to RGB888 pixel format */
//  for(index=0; index < height; index++)
//  {
//    /* Pixel format conversion */
//    LL_ConvertLineToRGB888( LayerIndex, (unsigned int *)pbmp, (unsigned int *)address, width, input_color_mode);
//    
//    /* Increment the source and destination buffers */
//    address = address + (RK043FN48H_WIDTH*3);
//    pbmp -= width*(bit_pixel/8);
//  } 
//}





/**
* @brief  指定位置(x,y)显示8x16字符
* @param  y: 分别指定显示区所在的行(0~16)
* @param  x: 分别指定显示区所在的列(0~59)
* @retval 1:成功 0：失败
*/
//unsigned char code_disp_8x16character( unsigned int LayerIndex, unsigned short int x, unsigned short int y, unsigned char *chn, unsigned int ZiFu_Color, unsigned int Background_Color )
//{
//	unsigned char	i, j, temp = 0;
//	unsigned char buf[8][3];
//	unsigned int  addr;
//	unsigned short int Xpos, Ypos;
//	
//	if( y>=17 || x>=60 ) return 0;
//	
//	Xpos = x * 8;
//	Ypos = y * 16;
//	
//	addr = hltdc.LayerCfg[LayerIndex].FBStartAdress;
//	addr = addr + 3*(RK043FN48H_WIDTH*Ypos + Xpos);
//	
//	for( i=0; i<16; i++ )
//	{
//		temp = *(chn+i);
//		
//		for( j=0; j<8; j++ )
//		{
//			if( ( temp & 0x80 ) == 0x80 ) 
//			{
//				buf[j][2] = ( unsigned char )( ( ZiFu_Color & 0x00FF0000) >> 16 );
//				buf[j][1] = ( unsigned char )( ( ZiFu_Color & 0x0000ff00) >> 8 );
//				buf[j][0] = ( unsigned char )( ZiFu_Color & 0x000000ff);
//			}				
//			else 
//			{
//				buf[j][2] = ( unsigned char )( ( Background_Color & 0x00FF0000) >> 16 );
//				buf[j][1] = ( unsigned char )( ( Background_Color & 0x0000ff00) >> 8 );
//				buf[j][0] = ( unsigned char )( Background_Color & 0x000000ff);
//			}
//			
//			temp = temp << 1;
//		}
//		
////		hdma2d.Init.Mode  = DMA2D_M2M;
////		hdma2d.Init.ColorMode = DMA2D_RGB888;
////		hdma2d.Init.OutputOffset = 0;  
////	
////		hdma2d.LayerCfg[LayerIndex].AlphaMode = DMA2D_NO_MODIF_ALPHA;
////		hdma2d.LayerCfg[LayerIndex].InputColorMode = CM_RGB888;
////		hdma2d.LayerCfg[LayerIndex].InputOffset = 0;
////		hdma2d.LayerCfg[LayerIndex].InputAlpha = 0xff;
////  
////		hdma2d.Instance = DMA2D;
////		
////		 /* DMA2D Initialization */
////		if(HAL_DMA2D_Init(&hdma2d) == HAL_OK) 
////		{
////			if(HAL_DMA2D_ConfigLayer(&hdma2d, LayerIndex) == HAL_OK) 
////			{
////				if (HAL_DMA2D_Start(&hdma2d, (unsigned int)&buf[0][0], addr, 8, 1) == HAL_OK)
////				{
////					/* Polling For DMA transfer */  
////					HAL_DMA2D_PollForTransfer( &hdma2d, 10);
////				}
////			}
////		} 
//		
//		SDRAM_Write_8B( addr-SDRAM_BANK1_ADDR, &buf[0][0], 24 );
//		
//		Ypos++;
//		addr = hltdc.LayerCfg[LayerIndex].FBStartAdress;
//		addr = addr + 3*(RK043FN48H_WIDTH*Ypos + Xpos);
//	}
//		
//	return 1; 
//}





/**
* @brief  指定位置(x,y)显示8x16字符
* @param  y: 分别指定显示区所在的行(0~271)
* @param  x: 分别指定显示区所在的列(0~479)
* @retval 1:成功 0：失败
*/
//unsigned char code_disp_8x16character1( unsigned int LayerIndex, unsigned short int x, unsigned short int y, unsigned char *chn, unsigned int ZiFu_Color, unsigned int Background_Color )
//{
//	unsigned char	i, j, temp = 0;
//	unsigned char buf[8][3];
//	unsigned int  addr;
//	unsigned short int Xpos, Ypos;
//	
//	if( y>=265 || x>=473 ) return 0;
//	
//	Xpos =  x;
//	Ypos =  y;
//	
//	addr = hltdc.LayerCfg[LayerIndex].FBStartAdress;
//	addr = addr + 3*(RK043FN48H_WIDTH*Ypos + Xpos);
//	
//	for( i=0; i<16; i++ )
//	{
//		temp = *(chn+i);
//		
//		for( j=0; j<8; j++ )
//		{
//			if( ( temp & 0x80 ) == 0x80 ) 
//			{
//				buf[j][2] = ( unsigned char )( ( ZiFu_Color & 0x00FF0000) >> 16 );
//				buf[j][1] = ( unsigned char )( ( ZiFu_Color & 0x0000ff00) >> 8 );
//				buf[j][0] = ( unsigned char )( ZiFu_Color & 0x000000ff);
//			}				
//			else 
//			{
//				buf[j][2] = ( unsigned char )( ( Background_Color & 0x00FF0000) >> 16 );
//				buf[j][1] = ( unsigned char )( ( Background_Color & 0x0000ff00) >> 8 );
//				buf[j][0] = ( unsigned char )( Background_Color & 0x000000ff);
//			}
//			
//			temp = temp << 1;
//		}
//		
////		hdma2d.Init.Mode  = DMA2D_M2M;
////		hdma2d.Init.ColorMode = DMA2D_RGB888;
////		hdma2d.Init.OutputOffset = 0;  
////	
////		hdma2d.LayerCfg[LayerIndex].AlphaMode = DMA2D_NO_MODIF_ALPHA;
////		hdma2d.LayerCfg[LayerIndex].InputColorMode = CM_RGB888;
////		hdma2d.LayerCfg[LayerIndex].InputOffset = 0;
////		hdma2d.LayerCfg[LayerIndex].InputAlpha = 0xff;
////  
////		hdma2d.Instance = DMA2D;
////		
////		 /* DMA2D Initialization */
////		if(HAL_DMA2D_Init(&hdma2d) == HAL_OK) 
////		{
////			if(HAL_DMA2D_ConfigLayer(&hdma2d, LayerIndex) == HAL_OK) 
////			{
////				if (HAL_DMA2D_Start(&hdma2d, (unsigned int)&buf[0][0], addr, 8, 1) == HAL_OK)
////				{
////					/* Polling For DMA transfer */  
////					HAL_DMA2D_PollForTransfer( &hdma2d, 10);
////				}
////			}
////		} 
//		
//		SDRAM_Write_8B( addr-SDRAM_BANK1_ADDR, &buf[0][0], 24 );
//		
//		Ypos++;
//		addr = hltdc.LayerCfg[LayerIndex].FBStartAdress;
//		addr = addr + 3*(RK043FN48H_WIDTH*Ypos + Xpos);
//	}
//		
//	return 1; 
//}




///**
//* @brief  指定位置(x,y)显示字符串(不能显示汉字）
//* @param  y: 分别指定显示区所在的行(0~16)
//* @param  x: 分别指定显示区所在的列(0~59)
//* @param  str: 要显示的字符
//* @retval 1:成功 0：失败
//*/
//unsigned char disp_8x16character( unsigned int LayerIndex, unsigned int x, unsigned int y, unsigned char *str, unsigned char num, unsigned int ZiFu_Color, unsigned int Background_Color )
//{
//	unsigned char	temp1, j;
//	unsigned char i = 0;
//	signed char temp2 ;
//	
//	temp1 = 95;
//	
//	if( ( num + x  ) > 60 )
//	{
//		num = 60 - x;
//	}
//	
//	for( i=0; i<num; i++ )
//	{
//		for( j=0; j<temp1; j++ )
//		{
//			if( code_nAsciiDot[j][0] == *( str+i ) ) 
//			{
//				temp2 = j;
//				break;
//			}
//		}
//		
//		if( j == temp1 ) 
//		{
//			continue;
//		}
//		else
//		{
//			if( code_disp_8x16character( LayerIndex, x+i,  y, (unsigned char*)&code_nAsciiDot[temp2][1], ZiFu_Color, Background_Color ) != 1 ) return 0;
//		}
//	}
//	
//	return 1;
//}




///**
//* @brief  指定位置(x,y)显示字符串(不能显示汉字）
//* @param  y: 分别指定显示区所在的行(0~271)
//* @param  x: 分别指定显示区所在的列(0~479)
//* @param  str: 要显示的字符
//* @retval 1:成功 0：失败
//*/
//unsigned char disp_8x16character1( unsigned int LayerIndex, unsigned int x, unsigned int y, unsigned char *str, unsigned char num, unsigned int ZiFu_Color, unsigned int Background_Color )
//{
//	unsigned char	temp1, j;
//	unsigned char i = 0;
//	signed char temp2 ;
//	
//	temp1 = 95;
//	
//	if( ( num * 8 + x  ) > 480 )
//	{
//		num = (480 - x) / 8;
//	}
//	
//	if( y+16 > 272 ) return 0;
//	
//	for( i=0; i<num; i++ )
//	{
//		for( j=0; j<temp1; j++ )
//		{
//			if( code_nAsciiDot[j][0] == *( str+i ) ) 
//			{
//				temp2 = j;
//				break;
//			}
//		}
//		
//		if( j == temp1 ) 
//		{
//			continue;
//		}
//		else
//		{
//			if( code_disp_8x16character1( LayerIndex, x+i*8,  y, (unsigned char*)&code_nAsciiDot[temp2][1], ZiFu_Color, Background_Color ) != 1 ) return 0;
//		}
//	}
//	
//	return 1;
//}





///**
//* @brief  指定位置(x,y)显示16x24字符
//* @param  y: 分别指定显示区所在的行(0~10)
//* @param  x: 分别指定显示区所在的列(0~29)
//* @retval 1:成功 0：失败
//*/
//unsigned char code_disp_16x24character( unsigned int LayerIndex, unsigned short int x, unsigned short int y, unsigned char *chn, unsigned int ZiFu_Color, unsigned int Background_Color )
//{
//	unsigned short int	i, j, temp = 0;
//	unsigned char buf[16][3];
//	unsigned int  addr;
//	unsigned short int Xpos, Ypos;
//	
//	if( y>=11 || x>=30 ) return 0;
//	
//	Xpos = x * 16;
//	Ypos = y * 24;
//	
//	addr = hltdc.LayerCfg[LayerIndex].FBStartAdress;
//	addr = addr + 3*(RK043FN48H_WIDTH*Ypos + Xpos);
//	
//	for( i=0; i<24; i++ )
//	{
//		temp = *(chn+i*2);
//		temp = temp << 8;
//		temp = temp | *(chn+i*2+1);
//		
//		for( j=0; j<16; j++ )
//		{
//			if( ( temp & 0x8000 ) == 0x8000 ) 
//			{
//				buf[j][2] = ( unsigned char )( ( ZiFu_Color & 0x00FF0000) >> 16 );
//				buf[j][1] = ( unsigned char )( ( ZiFu_Color & 0x0000ff00) >> 8 );
//				buf[j][0] = ( unsigned char )( ZiFu_Color & 0x000000ff);
//			}				
//			else 
//			{
//				buf[j][2] = ( unsigned char )( ( Background_Color & 0x00FF0000) >> 16 );
//				buf[j][1] = ( unsigned char )( ( Background_Color & 0x0000ff00) >> 8 );
//				buf[j][0] = ( unsigned char )( Background_Color & 0x000000ff);
//			}
//			
//			temp = temp << 1;
//		}
//		
//		SDRAM_Write_8B( addr-SDRAM_BANK1_ADDR, &buf[0][0], 48 );
//		
//		Ypos++;
//		addr = hltdc.LayerCfg[LayerIndex].FBStartAdress;
//		addr = addr + 3*(RK043FN48H_WIDTH*Ypos + Xpos);
//	}
//		
//	return 1; 
//}






///**
//* @brief  指定位置(x,y)显示16x24字符
//* @param  y: 分别指定显示区所在的行(0~271)
//* @param  x: 分别指定显示区所在的列(0~479)
//* @retval 1:成功 0：失败
//*/
//unsigned char code_disp_16x24character1( unsigned int LayerIndex, unsigned short int x, unsigned short int y, unsigned char *chn, unsigned int ZiFu_Color, unsigned int Background_Color )
//{
//	unsigned short int	i, j, temp = 0;
//	unsigned char buf[16][3];
//	unsigned int  addr;
//	unsigned short int Xpos, Ypos;
//	
//	if( y>=249 || x>=465 ) return 0;
//	
//	Xpos = x;
//	Ypos = y;
//	
//	addr = hltdc.LayerCfg[LayerIndex].FBStartAdress;
//	addr = addr + 3*(RK043FN48H_WIDTH*Ypos + Xpos);
//	
//	for( i=0; i<24; i++ )
//	{
//		temp = *(chn+i*2);
//		temp = temp << 8;
//		temp = temp | *(chn+i*2+1);
//		
//		for( j=0; j<16; j++ )
//		{
//			if( ( temp & 0x8000 ) == 0x8000 ) 
//			{
//				buf[j][2] = ( unsigned char )( ( ZiFu_Color & 0x00FF0000) >> 16 );
//				buf[j][1] = ( unsigned char )( ( ZiFu_Color & 0x0000ff00) >> 8 );
//				buf[j][0] = ( unsigned char )( ZiFu_Color & 0x000000ff);
//			}				
//			else 
//			{
//				buf[j][2] = ( unsigned char )( ( Background_Color & 0x00FF0000) >> 16 );
//				buf[j][1] = ( unsigned char )( ( Background_Color & 0x0000ff00) >> 8 );
//				buf[j][0] = ( unsigned char )( Background_Color & 0x000000ff);
//			}
//			
//			temp = temp << 1;
//		}
//		
//		SDRAM_Write_8B( addr-SDRAM_BANK1_ADDR, &buf[0][0], 48 );
//		
//		Ypos++;
//		addr = hltdc.LayerCfg[LayerIndex].FBStartAdress;
//		addr = addr + 3*(RK043FN48H_WIDTH*Ypos + Xpos);
//	}
//		
//	return 1; 
//}






///**
//* @brief  指定位置(x,y)显示字符串(不能显示汉字）
//* @param  y: 分别指定显示区所在的行(0~271)
//* @param  x: 分别指定显示区所在的列(0~479)
//* @param  str: 要显示的字符
//* @retval 1:成功 0：失败
//*/
//unsigned char disp_16x24character1( unsigned int LayerIndex, unsigned int x, unsigned int y, unsigned char *str, unsigned char num, unsigned int ZiFu_Color, unsigned int Background_Color )
//{
//	unsigned char	temp1, j;
//	unsigned char i = 0;
//	signed char temp2 ;
//	
//	temp1 = 95;
//	
//	if( ( num*16 + x  ) > 480 )
//	{
//		num = (480 - x) / 16;
//	}
//	
//	if( y > 249 ) return 0;
//	
//	for( i=0; i<num; i++ )
//	{
//		for( j=0; j<temp1; j++ )
//		{
//			if( code_nAsciiDot[j][0] == *( str+i ) ) 
//			{
//				temp2 = j;
//				break;
//			}
//		}
//		
//		if( j == temp1 ) 
//		{
//			continue;
//		}
//		else
//		{
//			if( code_disp_16x24character1( LayerIndex, x+i*16,  y, (unsigned char*)&code_nAsciiDot1[temp2][1], ZiFu_Color, Background_Color ) != 1 ) return 0;
//		}
//	}
//	
//	return 1;
//}






///**
//* @brief  指定位置(x,y)显示字符串(不能显示汉字）
//* @param  y: 分别指定显示区所在的行(0~10)
//* @param  x: 分别指定显示区所在的列(0~29)
//* @param  str: 要显示的字符
//* @retval 1:成功 0：失败
//*/
//unsigned char disp_16x24character( unsigned int LayerIndex, unsigned int x,unsigned int y, unsigned char *str, unsigned char num, unsigned int ZiFu_Color, unsigned int Background_Color )
//{
//	unsigned char	temp1, j;
//	unsigned char i = 0;
//	signed char temp2 ;
//	
//	temp1 = 95;
//	
//	if( ( num + x  ) > 30 )
//	{
//		num = 30 - x;
//	}
//	
//	if( y > 11 ) return 0;
//	
//	for( i=0; i<num; i++ )
//	{
//		for( j=0; j<temp1; j++ )
//		{
//			if( code_nAsciiDot[j][0] == *( str+i ) ) 
//			{
//				temp2 = j;
//				break;
//			}
//		}
//		
//		if( j == temp1 ) 
//		{
//			continue;
//		}
//		else
//		{
//			if( code_disp_16x24character( LayerIndex, x+i,  y, (unsigned char*)&code_nAsciiDot1[temp2][1], ZiFu_Color, Background_Color ) != 1 ) return 0;
//		}
//	}
//	
//	return 1;
//}





///**
//* @brief  指定位置(x,y)显示16x16中文字体
//* @param  y: 分别指定显示区所在的行(0~16)
//* @param  x: 分别指定显示区所在的列(0~29)
//* @retval 1:成功 0：失败
//*/
//unsigned char code_disp_16x16GB( unsigned int LayerIndex, unsigned short int x, unsigned short int y, unsigned char *chn, unsigned int ZiFu_Color, unsigned int Background_Color )
//{
//	unsigned short int	i, j, temp = 0;
//	unsigned char buf[16][3];
//	unsigned int  addr;
//	unsigned short int Xpos, Ypos;
//	
//	if( y>=17 || x>=30 ) return 0;
//	
//	Xpos = x * 16;
//	Ypos = y * 16;
//	
//	addr = hltdc.LayerCfg[LayerIndex].FBStartAdress;
//	addr = addr + 3*(RK043FN48H_WIDTH*Ypos + Xpos);
//	
//	for( i=0; i<16; i++ )
//	{
//		temp = *(chn+i*2);
//		temp = temp << 8;
//		temp = temp | *(chn+i*2+1);
//		
//		for( j=0; j<16; j++ )
//		{
//			if( ( temp & 0x8000 ) == 0x8000 ) 
//			{
//				buf[j][2] = ( unsigned char )( ( ZiFu_Color & 0x00FF0000) >> 16 );
//				buf[j][1] = ( unsigned char )( ( ZiFu_Color & 0x0000ff00) >> 8 );
//				buf[j][0] = ( unsigned char )( ZiFu_Color & 0x000000ff);
//			}				
//			else 
//			{
//				buf[j][2] = ( unsigned char )( ( Background_Color & 0x00FF0000) >> 16 );
//				buf[j][1] = ( unsigned char )( ( Background_Color & 0x0000ff00) >> 8 );
//				buf[j][0] = ( unsigned char )( Background_Color & 0x000000ff);
//			}
//			
//			temp = temp << 1;
//		}
//		
//		SDRAM_Write_8B( addr-SDRAM_BANK1_ADDR, &buf[0][0], 48 );
//		
//		Ypos++;
//		addr = hltdc.LayerCfg[LayerIndex].FBStartAdress;
//		addr = addr + 3*(RK043FN48H_WIDTH*Ypos + Xpos);
//	}
//	
//	return 1; 
//}






///**
//* @brief  指定位置(x,y)显示16x16中文字体
//* @param  y: 分别指定显示区所在的行(0~271)
//* @param  x: 分别指定显示区所在的列(0~479)
//* @retval 1:成功 0：失败
//*/
//unsigned char code_disp_16x16GB1( unsigned int LayerIndex, unsigned short int x, unsigned short int y, unsigned char *chn, unsigned int ZiFu_Color, unsigned int Background_Color )
//{
//	unsigned short int	i, j, temp = 0;
//	unsigned char buf[16][3];
//	unsigned int  addr;
//	unsigned short int Xpos, Ypos;
//	
//	if( y>=257 || x>=465 ) return 0;
//	
//	Xpos = x;
//	Ypos = y;
//	
//	addr = hltdc.LayerCfg[LayerIndex].FBStartAdress;
//	addr = addr + 3*(RK043FN48H_WIDTH*Ypos + Xpos);
//	
//	for( i=0; i<16; i++ )
//	{
//		temp = *(chn+i*2);
//		temp = temp << 8;
//		temp = temp | *(chn+i*2+1);
//		
//		for( j=0; j<16; j++ )
//		{
//			if( ( temp & 0x8000 ) == 0x8000 ) 
//			{
//				buf[j][2] = ( unsigned char )( ( ZiFu_Color & 0x00FF0000) >> 16 );
//				buf[j][1] = ( unsigned char )( ( ZiFu_Color & 0x0000ff00) >> 8 );
//				buf[j][0] = ( unsigned char )( ZiFu_Color & 0x000000ff);
//			}				
//			else 
//			{
//				buf[j][2] = ( unsigned char )( ( Background_Color & 0x00FF0000) >> 16 );
//				buf[j][1] = ( unsigned char )( ( Background_Color & 0x0000ff00) >> 8 );
//				buf[j][0] = ( unsigned char )( Background_Color & 0x000000ff);
//			}
//			
//			temp = temp << 1;
//		}
//		
//		SDRAM_Write_8B( addr-SDRAM_BANK1_ADDR, &buf[0][0], 48 );
//		
//		Ypos++;
//		addr = hltdc.LayerCfg[LayerIndex].FBStartAdress;
//		addr = addr + 3*(RK043FN48H_WIDTH*Ypos + Xpos);
//	}
//	
//	return 1; 
//}




///**
//* @brief  指定位置(x,y)显示汉字(不能显示字符）
//* @param  x: 分别指定显示区所在的行(0~29)
//* @param  y: 分别指定显示区所在的列(0~16)
//* @param  str: 要显示的汉字
//* @param  lenth: 要显示的汉字的个数
//* @retval 1:成功 0：失败
//*/
//unsigned char disp_16x16GB( unsigned int LayerIndex, unsigned int x, unsigned int y, unsigned char *str,  unsigned char lenth, unsigned int ZiFu_Color, unsigned int Background_Color )
//{
//	unsigned int	temp1, j;
//	unsigned char	buf[2];
//	unsigned int num = 0;
//	unsigned char i = 0;
//	unsigned int temp2;
//	
//	num = lenth;
//	temp1 = 19; 
//	
//	if( ( num + x ) > 30 )
//	{
//		num = 30 - x;
//	}
//	
//	if( y > 16 ) return 0;
//	
//	for( i=0; i<num; i++ )
//	{
//		buf[0] = *(str+i*2);
//		buf[1] = *(str+i*2+1);
//		
//		for( j=0; j<temp1; j++ )
//		{
//			if( (buf[0] == code_GB_16[j].Index[0]) && (buf[1] == code_GB_16[j].Index[1]) )
//			{
//				temp2 = j;
//				break;
//			}
//		}
//				
//		if( j == temp1 ) 
//		{
//			continue;
//		}
//		else
//		{
//			if( code_disp_16x16GB( LayerIndex, x+i,  y, (unsigned char*)&code_GB_16[temp2].Msk[0], ZiFu_Color, Background_Color ) != 1 ) return 0;
//		}
//	}
//	
//	return 1;
//}




///**
//* @brief  指定位置(x,y)显示汉字(不能显示字符）
//* @param  x: 分别指定显示区所在的行(0~479)
//* @param  y: 分别指定显示区所在的列(0~271)
//* @param  str: 要显示的汉字
//* @param  lenth: 要显示的汉字的个数
//* @retval 1:成功 0：失败
//*/
//unsigned char disp_16x16GB1( unsigned int LayerIndex, unsigned int x, unsigned int y, unsigned char *str,  unsigned char lenth, unsigned int ZiFu_Color, unsigned int Background_Color )
//{
//	unsigned int	temp1, j;
//	unsigned char	buf[2];
//	unsigned int num = 0;
//	unsigned char i = 0;
//	unsigned int temp2;
//	
//	num = lenth;
//	temp1 = 19; 
//	
//	if( ( num*16 + x ) > 480 )
//	{
//		num = (480 - x) / 16;
//	}
//	
//	if( y >= 257 ) return 0;
//	
//	for( i=0; i<num; i++ )
//	{
//		buf[0] = *(str+i*2);
//		buf[1] = *(str+i*2+1);
//		
//		for( j=0; j<temp1; j++ )
//		{
//			if( (buf[0] == code_GB_16[j].Index[0]) && (buf[1] == code_GB_16[j].Index[1]) )
//			{
//				temp2 = j;
//				break;
//			}
//		}
//				
//		if( j == temp1 ) 
//		{
//			continue;
//		}
//		else
//		{
//			if( code_disp_16x16GB1( LayerIndex, x+i*16,  y, (unsigned char*)&code_GB_16[temp2].Msk[0], ZiFu_Color, Background_Color ) != 1 ) return 0;
//		}
//	}
//	
//	return 1;
//}






///**
//* @brief  指定位置(x,y)显示24x24中文字体
//* @param  y: 分别指定显示区所在的行(0~19)
//* @param  x: 分别指定显示区所在的列(0~10)
//* @retval 1:成功 0：失败
//*/
//unsigned char code_disp_24x24GB( unsigned int LayerIndex, unsigned short int x, unsigned short int y, unsigned char *chn, unsigned int ZiFu_Color, unsigned int Background_Color )
//{
//	unsigned int	i, j, temp = 0;
//	unsigned char buf[24][3];
//	unsigned int  addr;
//	unsigned short int Xpos, Ypos;
//	
//	if( y>=11 || x>=20 ) return 0;
//	
//	Xpos = x * 24;
//	Ypos = y * 24;
//	
//	addr = hltdc.LayerCfg[LayerIndex].FBStartAdress;
//	addr = addr + 3*(RK043FN48H_WIDTH*Ypos + Xpos);
//	
//	for( i=0; i<24; i++ )
//	{
//		temp = *(chn+i*3);
//		temp = temp << 8;
//		temp = temp | *(chn+i*3+1);
//		temp = (temp << 8) | *(chn+i*3+2);
//		
//		for( j=0; j<24; j++ )
//		{
//			if( ( temp & 0x800000 ) == 0x800000 ) 
//			{
//				buf[j][2] = ( unsigned char )( ( ZiFu_Color & 0x00FF0000) >> 16 );
//				buf[j][1] = ( unsigned char )( ( ZiFu_Color & 0x0000ff00) >> 8 );
//				buf[j][0] = ( unsigned char )( ZiFu_Color & 0x000000ff);
//			}				
//			else 
//			{
//				buf[j][2] = ( unsigned char )( ( Background_Color & 0x00FF0000) >> 16 );
//				buf[j][1] = ( unsigned char )( ( Background_Color & 0x0000ff00) >> 8 );
//				buf[j][0] = ( unsigned char )( Background_Color & 0x000000ff);
//			}
//			
//			temp = temp << 1;
//		}
//		
//		SDRAM_Write_8B( addr-SDRAM_BANK1_ADDR, &buf[0][0], 72 );
//		
//		Ypos++;
//		addr = hltdc.LayerCfg[LayerIndex].FBStartAdress;
//		addr = addr + 3*(RK043FN48H_WIDTH*Ypos + Xpos);
//	}
//	
//	return 1; 
//}





///**
//* @brief  指定位置(x,y)显示24x24中文字体
//* @param  y: 分别指定显示区所在的行(0~271)
//* @param  x: 分别指定显示区所在的列(0~479)
//* @retval 1:成功 0：失败
//*/
//unsigned char code_disp_24x24GB1( unsigned int LayerIndex, unsigned short int x, unsigned short int y, unsigned char *chn, unsigned int ZiFu_Color, unsigned int Background_Color )
//{
//	unsigned int	i, j, temp = 0;
//	unsigned char buf[24][3];
//	unsigned int  addr;
//	unsigned short int Xpos, Ypos;
//	
//	if( y>=249 || x>=457 ) return 0;
//	
//	Xpos = x;
//	Ypos = y;
//	
//	addr = hltdc.LayerCfg[LayerIndex].FBStartAdress;
//	addr = addr + 3*(RK043FN48H_WIDTH*Ypos + Xpos);
//	
//	for( i=0; i<24; i++ )
//	{
//		temp = *(chn+i*3);
//		temp = temp << 8;
//		temp = temp | *(chn+i*3+1);
//		temp = (temp << 8) | *(chn+i*3+2);
//		
//		for( j=0; j<24; j++ )
//		{
//			if( ( temp & 0x800000 ) == 0x800000 ) 
//			{
//				buf[j][2] = ( unsigned char )( ( ZiFu_Color & 0x00FF0000) >> 16 );
//				buf[j][1] = ( unsigned char )( ( ZiFu_Color & 0x0000ff00) >> 8 );
//				buf[j][0] = ( unsigned char )( ZiFu_Color & 0x000000ff);
//			}				
//			else 
//			{
//				buf[j][2] = ( unsigned char )( ( Background_Color & 0x00FF0000) >> 16 );
//				buf[j][1] = ( unsigned char )( ( Background_Color & 0x0000ff00) >> 8 );
//				buf[j][0] = ( unsigned char )( Background_Color & 0x000000ff);
//			}
//			
//			temp = temp << 1;
//		}
//		
//		SDRAM_Write_8B( addr-SDRAM_BANK1_ADDR, &buf[0][0], 72 );
//		
//		Ypos++;
//		addr = hltdc.LayerCfg[LayerIndex].FBStartAdress;
//		addr = addr + 3*(RK043FN48H_WIDTH*Ypos + Xpos);
//	}
//	
//	return 1; 
//}





///**
//* @brief  指定位置(x,y)显示汉字(不能显示字符）
//* @param  x: 分别指定显示区所在的行(0~19)
//* @param  y: 分别指定显示区所在的列(0~10)
//* @param  str: 要显示的汉字
//* @param  lenth: 要显示的汉字的个数
//* @retval 1:成功 0：失败
//*/
//unsigned char disp_24x24GB( unsigned int LayerIndex, unsigned int x, unsigned int y, unsigned char *str,  unsigned char lenth, unsigned int ZiFu_Color, unsigned int Background_Color )
//{
//	unsigned int	temp1, j;
//	unsigned char	buf[2];
//	unsigned int num = 0;
//	unsigned char i = 0;
//	unsigned int temp2;
//	
//	num = lenth;
//	temp1 = 42; 
//	
//	if( ( num + x ) > 20 )
//	{
//		num = 20- x;
//	}
//	
//	if( y > 10 ) return 0;
//	
//	for( i=0; i<num; i++ )
//	{
//		buf[0] = *(str+i*2);
//		buf[1] = *(str+i*2+1);
//		
//		for( j=0; j<temp1; j++ )
//		{
//			if( (buf[0] == code_GB_24[j].Index[0]) && (buf[1] == code_GB_24[j].Index[1]) )
//			{
//				temp2 = j;
//				break;
//			}
//		}
//				
//		if( j == temp1 ) 
//		{
//			continue;
//		}
//		else
//		{
//			if( code_disp_24x24GB( LayerIndex, x+i,  y, (unsigned char*)&code_GB_24[temp2].Msk[0], ZiFu_Color, Background_Color ) != 1 ) return 0;
//		}
//	}
//	
//	return 1;
//}






///**
//* @brief  指定位置(x,y)显示汉字(不能显示字符）
//* @param  x: 分别指定显示区所在的行(0~479)
//* @param  y: 分别指定显示区所在的列(0~271)
//* @param  str: 要显示的汉字
//* @param  lenth: 要显示的汉字的个数
//* @retval 1:成功 0：失败
//*/
//unsigned char disp_24x24GB1( unsigned int LayerIndex, unsigned int x, unsigned int y, unsigned char *str,  unsigned char lenth, unsigned int ZiFu_Color, unsigned int Background_Color )
//{
//	unsigned int	temp1, j;
//	unsigned char	buf[2];
//	unsigned int num = 0;
//	unsigned char i = 0;
//	unsigned int temp2;
//	
//	num = lenth;
//	temp1 = 42; 
//	
//	if( ( num * 24 + x ) > 480 )
//	{
//		num = (480- x) / 24;
//	}
//	
//	if( y >= 249 ) return 0;
//	
//	for( i=0; i<num; i++ )
//	{
//		buf[0] = *(str+i*2);
//		buf[1] = *(str+i*2+1);
//		
//		for( j=0; j<temp1; j++ )
//		{
//			if( (buf[0] == code_GB_24[j].Index[0]) && (buf[1] == code_GB_24[j].Index[1]) )
//			{
//				temp2 = j;
//				break;
//			}
//		}
//				
//		if( j == temp1 ) 
//		{
//			continue;
//		}
//		else
//		{
//			if( code_disp_24x24GB1( LayerIndex, x+i*24,  y, (unsigned char*)&code_GB_24[temp2].Msk[0], ZiFu_Color, Background_Color ) != 1 ) return 0;
//		}
//	}
//	
//	return 1;
//}




	
	
	
	






		
	
	
	
	
	
	
	
	
	
	
	
	
	








