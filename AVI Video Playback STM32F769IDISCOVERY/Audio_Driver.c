#include "Audio_Driver.h"
#include "wm8994.h"




SAI_HandleTypeDef         haudio_out_sai={0};
SAI_HandleTypeDef         haudio_in_sai={0};
DMA_HandleTypeDef hdma_sai_tx;
DMA_HandleTypeDef hdma_sai_rx;


uint16_t __IO AudioInVolume = 80;




extern volatile unsigned char AudioOutFlag;
extern volatile unsigned char TxCpltFlag;
extern volatile unsigned char TxHalfCpltFlag;

extern volatile unsigned char RxCpltFlag;
extern volatile unsigned char RxHalfCpltFlag;
    






/**
  * @brief  Configures the audio peripherals.
  * @param  OutputDevice: OUTPUT_DEVICE_SPEAKER, OUTPUT_DEVICE_HEADPHONE,
  *                       or OUTPUT_DEVICE_BOTH.
  * @param  Volume: Initial volume level (from 0 (Mute) to 100 (Max))
  * @param  AudioFreq: Audio frequency used to play the audio stream.
  * @note   The I2S PLL input clock must be done in the user application.  
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t BSP_AUDIO_OUT_Init(uint16_t OutputDevice, uint8_t Volume, uint32_t AudioFreq)
{ 
	uint8_t ret = AUDIO_ERROR;
  uint32_t deviceid = 0x00;
	

	__HAL_RCC_SAI1_CLK_ENABLE();
	/** Sw reset of I2C4 IP */
	__HAL_RCC_SAI1_FORCE_RESET();
	WM8994_Delay_1ms( 2 );
  __HAL_RCC_SAI1_RELEASE_RESET();
	WM8994_Delay_1ms( 2 );
	
	__HAL_RCC_DMA2_CLK_ENABLE();
	

  /* PLL clock is set depending on the AudioFreq (44.1khz vs 48khz groups) */
  BSP_AUDIO_OUT_ClockConfig(&haudio_out_sai, AudioFreq, NULL);
	
	
	/* Initialize the haudio_out_sai Instance parameter */
  haudio_out_sai.Instance = SAI1_Block_A;
  HAL_SAI_DeInit(&haudio_out_sai);
	
	/* Configure SAI_Block_x 
  LSBFirst: Disabled 
  DataSize: 16 */
	haudio_out_sai.Init.MonoStereoMode = SAI_STEREOMODE;
  haudio_out_sai.Init.AudioFrequency = AudioFreq;
  haudio_out_sai.Init.AudioMode = SAI_MODEMASTER_TX;
  haudio_out_sai.Init.NoDivider = SAI_MASTERDIVIDER_ENABLED;
  haudio_out_sai.Init.Protocol = SAI_FREE_PROTOCOL;
  haudio_out_sai.Init.DataSize = SAI_DATASIZE_16;
  haudio_out_sai.Init.FirstBit = SAI_FIRSTBIT_MSB;
  haudio_out_sai.Init.ClockStrobing = SAI_CLOCKSTROBING_RISINGEDGE;
  haudio_out_sai.Init.Synchro = SAI_ASYNCHRONOUS;
  haudio_out_sai.Init.OutputDrive = SAI_OUTPUTDRIVE_ENABLED;
  haudio_out_sai.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_1QF;
	haudio_out_sai.Init.SynchroExt  = SAI_SYNCEXT_DISABLE;
  haudio_out_sai.Init.CompandingMode = SAI_NOCOMPANDING;
  haudio_out_sai.Init.TriState = SAI_OUTPUT_NOTRELEASED;
  haudio_out_sai.Init.Mckdiv = 0;
  
  /* Configure SAI_Block_x Frame 
  Frame Length: 64
  Frame active Length: 32
  FS Definition: Start frame + Channel Side identification
  FS Polarity: FS active Low
  FS Offset: FS asserted one bit before the first bit of slot 0 */ 
  haudio_out_sai.FrameInit.FrameLength = 64; 
  haudio_out_sai.FrameInit.ActiveFrameLength = 32;
  haudio_out_sai.FrameInit.FSDefinition = SAI_FS_CHANNEL_IDENTIFICATION;
  haudio_out_sai.FrameInit.FSPolarity = SAI_FS_ACTIVE_LOW;
  haudio_out_sai.FrameInit.FSOffset = SAI_FS_BEFOREFIRSTBIT;
  
  /* Configure SAI Block_x Slot 
  Slot First Bit Offset: 0
  Slot Size  : 16
  Slot Number: 4
  Slot Active: All slot actives */
  haudio_out_sai.SlotInit.FirstBitOffset = 0;
  haudio_out_sai.SlotInit.SlotSize = SAI_SLOTSIZE_DATASIZE;
  haudio_out_sai.SlotInit.SlotNumber = 4; 
  haudio_out_sai.SlotInit.SlotActive = CODEC_AUDIOFRAME_SLOT_0123;

  HAL_SAI_Init(&haudio_out_sai);
  
  /* Enable SAI peripheral to generate MCLK */
  __HAL_SAI_ENABLE(&haudio_out_sai);


/* Configure the hdma_saiTx handle parameters */   
	hdma_sai_tx.Instance =  DMA2_Stream1;
	/* Deinitialize the Stream for new transfer */
	HAL_DMA_DeInit(&hdma_sai_tx);
	
	hdma_sai_tx.Init.Channel             = DMA_CHANNEL_0;
	hdma_sai_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
	hdma_sai_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
	hdma_sai_tx.Init.MemInc              = DMA_MINC_ENABLE;
	hdma_sai_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
	hdma_sai_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
	hdma_sai_tx.Init.Mode                = DMA_CIRCULAR;
	hdma_sai_tx.Init.Priority            = DMA_PRIORITY_VERY_HIGH;
	hdma_sai_tx.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;         
	hdma_sai_tx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
	hdma_sai_tx.Init.MemBurst            = DMA_MBURST_SINGLE;
	hdma_sai_tx.Init.PeriphBurst         = DMA_PBURST_SINGLE; 
    
	/* Associate the DMA handle */
	__HAL_LINKDMA(&haudio_out_sai, hdmatx, hdma_sai_tx);
	
	/* Configure the DMA Stream */
	HAL_DMA_Init(&hdma_sai_tx); 
  
  /* SAI DMA IRQ Channel configuration */
  HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 0X0C, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn); 
	
	
	if( wm8994_drv.Init(AUDIO_I2C_ADDRESS, OutputDevice, Volume, AudioFreq) != 0 )	return AUDIO_ERROR;

  /* wm8994 codec initialization */
  deviceid = wm8994_drv.ReadID(AUDIO_I2C_ADDRESS);
  
  if((deviceid) == WM8994_ID)
  {  
    /* Initialize the audio driver structure */
//    audio_drv = &wm8994_drv; 
    ret = AUDIO_OK;
  }
  else
  {
    ret = AUDIO_ERROR;
  }
 
  return ret;
}

/**
  * @brief  Starts playing audio stream from a data buffer for a determined size. 
  * @param  pBuffer: Pointer to the buffer 
  * @param  Size: Number of audio data in BYTES unit.
  *         In memory, first element is for left channel, second element is for right channel
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t BSP_AUDIO_OUT_Play(uint16_t* pBuffer, uint32_t Size)
{
  /* Call the audio Codec Play function */
  if(wm8994_drv.Play(AUDIO_I2C_ADDRESS, pBuffer, Size) != 0)
  {  
    return AUDIO_ERROR;
  }
  else
  {
    /* Update the Media layer and enable it for play */  
    HAL_SAI_Transmit_DMA(&haudio_out_sai, (uint8_t*) pBuffer, Size/2 );
    
    return AUDIO_OK;
  }
}





/**
  * @brief  This function Pauses the audio file stream. In case
  *         of using DMA, the DMA Pause feature is used.
  * @note When calling BSP_AUDIO_OUT_Pause() function for pause, only
  *          BSP_AUDIO_OUT_Resume() function should be called for resume (use of BSP_AUDIO_OUT_Play() 
  *          function for resume could lead to unexpected behaviour).
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t BSP_AUDIO_OUT_Pause(void)
{    
  /* Call the Audio Codec Pause/Resume function */
  if(wm8994_drv.Pause(AUDIO_I2C_ADDRESS) != 0)
  {
    return AUDIO_ERROR;
  }
  else
  {
    /* Call the Media layer pause function */
    HAL_SAI_DMAPause(&haudio_out_sai);
    
    /* Return AUDIO_OK when all operations are correctly done */
    return AUDIO_OK;
  }
}

/**
  * @brief  This function  Resumes the audio file stream.  
  * @note When calling BSP_AUDIO_OUT_Pause() function for pause, only
  *          BSP_AUDIO_OUT_Resume() function should be called for resume (use of BSP_AUDIO_OUT_Play() 
  *          function for resume could lead to unexpected behaviour).
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t BSP_AUDIO_OUT_Resume(void)
{    
  /* Call the Audio Codec Pause/Resume function */
  if(wm8994_drv.Resume(AUDIO_I2C_ADDRESS) != 0)
  {
    return AUDIO_ERROR;
  }
  else
  {
    /* Call the Media layer pause/resume function */
    HAL_SAI_DMAResume(&haudio_out_sai);
    
    /* Return AUDIO_OK when all operations are correctly done */
    return AUDIO_OK;
  }
}




/**
  * @brief  Stops audio playing and Power down the Audio Codec. 
  * @param  Option: could be one of the following parameters 
  *           - CODEC_PDWN_SW: for software power off (by writing registers). 
  *                            Then no need to reconfigure the Codec after power on.
  *           - CODEC_PDWN_HW: completely shut down the codec (physically). 
  *                            Then need to reconfigure the Codec after power on.  
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t BSP_AUDIO_OUT_Stop(uint32_t Option)
{
  /* Call the Media layer stop function */
  HAL_SAI_DMAStop(&haudio_out_sai);
  
  /* Call Audio Codec Stop function */
  if(wm8994_drv.Stop(AUDIO_I2C_ADDRESS, Option) != 0)
  {
    return AUDIO_ERROR;
  }
  else
  {
    if(Option == CODEC_PDWN_HW)
    { 
      /* Wait at least 100us */
      WM8994_Delay_1ms( 1 );
    }
    /* Return AUDIO_OK when all operations are correctly done */
    return AUDIO_OK;
  }
}

/**
  * @brief  Controls the current audio volume level. 
  * @param  Volume: Volume level to be set in percentage from 0% to 100% (0 for 
  *         Mute and 100 for Max volume level).
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t BSP_AUDIO_OUT_SetVolume(uint8_t Volume)
{
  /* Call the codec volume control function with converted volume value */
  if(wm8994_drv.SetVolume(AUDIO_I2C_ADDRESS, Volume) != 0)
  {
    return AUDIO_ERROR;
  }
  else
  {
    /* Return AUDIO_OK when all operations are correctly done */
    return AUDIO_OK;
  }
}

/**
  * @brief  Enables or disables the MUTE mode by software 
  * @param  Cmd: Could be AUDIO_MUTE_ON to mute sound or AUDIO_MUTE_OFF to 
  *         unmute the codec and restore previous volume level.
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t BSP_AUDIO_OUT_SetMute(uint32_t Cmd)
{ 
  /* Call the Codec Mute function */
  if(wm8994_drv.SetMute(AUDIO_I2C_ADDRESS, Cmd) != 0)
  {
    return AUDIO_ERROR;
  }
  else
  {
    /* Return AUDIO_OK when all operations are correctly done */
    return AUDIO_OK;
  }
}

/**
  * @brief  Switch dynamically (while audio file is played) the output target 
  *         (speaker or headphone).
  * @param  Output: The audio output target: OUTPUT_DEVICE_SPEAKER,
  *         OUTPUT_DEVICE_HEADPHONE or OUTPUT_DEVICE_BOTH
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t BSP_AUDIO_OUT_SetOutputMode(uint8_t Output)
{
  /* Call the Codec output device function */
  if(wm8994_drv.SetOutputMode(AUDIO_I2C_ADDRESS, Output) != 0)
  {
    return AUDIO_ERROR;
  }
  else
  {
    /* Return AUDIO_OK when all operations are correctly done */
    return AUDIO_OK;
  }
}


/**
  * @brief  Updates the audio frequency.
  * @param  AudioFreq: Audio frequency used to play the audio stream.
  * @note   This API should be called after the BSP_AUDIO_OUT_Init() to adjust the
  *         audio frequency.
  * @retval None
  */
void BSP_AUDIO_OUT_SetFrequency(uint32_t AudioFreq)
{ 
  /* PLL clock is set depending by the AudioFreq (44.1khz vs 48khz groups) */ 
  BSP_AUDIO_OUT_ClockConfig(&haudio_out_sai, AudioFreq, NULL);

  /* Disable SAI peripheral to allow access to SAI internal registers */
  __HAL_SAI_DISABLE(&haudio_out_sai);
  
  /* Update the SAI audio frequency configuration */
  haudio_out_sai.Init.AudioFrequency = AudioFreq;
  HAL_SAI_Init(&haudio_out_sai);
  
  /* Enable SAI peripheral to generate MCLK */
  __HAL_SAI_ENABLE(&haudio_out_sai);
}


/**
  * @brief  Updates the Audio frame slot configuration.
  * @param  AudioFrameSlot: specifies the audio Frame slot
  *         This parameter can be one of the following values
  *            @arg CODEC_AUDIOFRAME_SLOT_0123
  *            @arg CODEC_AUDIOFRAME_SLOT_02
  *            @arg CODEC_AUDIOFRAME_SLOT_13
  * @note   This API should be called after the BSP_AUDIO_OUT_Init() to adjust the
  *         audio frame slot.
  * @retval None
  */
void BSP_AUDIO_OUT_SetAudioFrameSlot(uint32_t AudioFrameSlot)
{ 
  /* Disable SAI peripheral to allow access to SAI internal registers */
  __HAL_SAI_DISABLE(&haudio_out_sai);
  
  /* Update the SAI audio frame slot configuration */
  haudio_out_sai.SlotInit.SlotActive = AudioFrameSlot;
  HAL_SAI_Init(&haudio_out_sai);
  
  /* Enable SAI peripheral to generate MCLK */
  __HAL_SAI_ENABLE(&haudio_out_sai);
}



/**
  * @brief  Tx Transfer completed callbacks.
  * @param  hsai: SAI handle
  * @retval None
  */
void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai)
{
  /* Manage the remaining file size and new address offset: This function 
     should be coded by user (its prototype is already declared in stm32746g_discovery_audio.h) */
	TxCpltFlag = 1;
	AudioOutFlag = 0;
}

/**
  * @brief  Tx Half Transfer completed callbacks.
  * @param  hsai: SAI handle
  * @retval None
  */
void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
  /* Manage the remaining file size and new address offset: This function 
     should be coded by user (its prototype is already declared in stm32746g_discovery_audio.h) */
	TxHalfCpltFlag = 1;
	AudioOutFlag = 0;
}

/**
  * @brief  SAI error callbacks.
  * @param  hsai: SAI handle
  * @retval None
  */
void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai)
{
  HAL_SAI_StateTypeDef audio_out_state;
  HAL_SAI_StateTypeDef audio_in_state;

  audio_out_state = HAL_SAI_GetState(&haudio_out_sai);
  audio_in_state = HAL_SAI_GetState(&haudio_in_sai);

  /* Determines if it is an audio out or audio in error */
  if ((audio_out_state == HAL_SAI_STATE_BUSY) || (audio_out_state == HAL_SAI_STATE_BUSY_TX))
  {
    BSP_AUDIO_OUT_Error_CallBack();
  }

  if ((audio_in_state == HAL_SAI_STATE_BUSY) || (audio_in_state == HAL_SAI_STATE_BUSY_RX))
  {
    BSP_AUDIO_IN_Error_CallBack();
  }
}

/**
  * @brief  Manages the DMA full Transfer complete event.
  * @retval None
  */
__weak void BSP_AUDIO_OUT_TransferComplete_CallBack(void)
{
}

/**
  * @brief  Manages the DMA Half Transfer complete event.
  * @retval None
  */
__weak void BSP_AUDIO_OUT_HalfTransfer_CallBack(void)
{ 
}

/**
  * @brief  Manages the DMA FIFO error event.
  * @retval None
  */
__weak void BSP_AUDIO_OUT_Error_CallBack(void)
{
}




/**
  * @brief  Clock Config.
  * @param  hsai: might be required to set audio peripheral predivider if any.
  * @param  AudioFreq: Audio frequency used to play the audio stream.
  * @param  Params  
  * @note   This API is called by BSP_AUDIO_OUT_Init() and BSP_AUDIO_OUT_SetFrequency()
  *         Being __weak it can be overwritten by the application     
  * @retval None
  */
__weak void BSP_AUDIO_OUT_ClockConfig(SAI_HandleTypeDef *hsai, uint32_t AudioFreq, void *Params)
{ 
  RCC_PeriphCLKInitTypeDef rcc_ex_clk_init_struct;

  HAL_RCCEx_GetPeriphCLKConfig(&rcc_ex_clk_init_struct);
  
  /* Set the PLL configuration according to the audio frequency */
  if((AudioFreq == AUDIO_FREQUENCY_11K) || (AudioFreq == AUDIO_FREQUENCY_22K) || (AudioFreq == AUDIO_FREQUENCY_44K))
  {
    /* Configure PLLI2S prescalers */
    /* PLLI2S_VCO: VCO_429M
    I2S_CLK(first level) = PLLI2S_VCO/PLLI2SQ = 429/2 = 214.5 Mhz
    I2S_CLK_x = I2S_CLK(first level)/PLLI2SDIVQ = 214.5/19 = 11.289 Mhz */
    rcc_ex_clk_init_struct.PeriphClockSelection = RCC_PERIPHCLK_SAI1;
    rcc_ex_clk_init_struct.Sai1ClockSelection = RCC_SAI1CLKSOURCE_PLLI2S;
    rcc_ex_clk_init_struct.PLLI2S.PLLI2SN = 429;
    rcc_ex_clk_init_struct.PLLI2S.PLLI2SQ = 2;
    rcc_ex_clk_init_struct.PLLI2SDivQ = 19;
    
    HAL_RCCEx_PeriphCLKConfig(&rcc_ex_clk_init_struct);
    
  }
  else /* AUDIO_FREQUENCY_8K, AUDIO_FREQUENCY_16K, AUDIO_FREQUENCY_48K), AUDIO_FREQUENCY_96K */
  {
    /* I2S clock config
    PLLI2S_VCO: VCO_344M
    I2S_CLK(first level) = PLLI2S_VCO/PLLI2SQ = 344/7 = 49.142 Mhz
    I2S_CLK_x = I2S_CLK(first level)/PLLI2SDIVQ = 49.142/1 = 49.142 Mhz */
    rcc_ex_clk_init_struct.PeriphClockSelection = RCC_PERIPHCLK_SAI1;
    rcc_ex_clk_init_struct.Sai1ClockSelection = RCC_SAI1CLKSOURCE_PLLI2S;
    rcc_ex_clk_init_struct.PLLI2S.PLLI2SN = 344;
    rcc_ex_clk_init_struct.PLLI2S.PLLI2SQ = 7;
    rcc_ex_clk_init_struct.PLLI2SDivQ = 1;
    
    HAL_RCCEx_PeriphCLKConfig(&rcc_ex_clk_init_struct);
  }
}
/*******************************************************************************
                            Static Functions
*******************************************************************************/
/**
  * @brief  Initializes wave recording.
  * @param  InputDevice: INPUT_DEVICE_DIGITAL_MICROPHONE_2 or INPUT_DEVICE_INPUT_LINE_1
  * @param  Volume: Initial volume level (in range 0(Mute)..80(+0dB)..100(+17.625dB))
  * @param  AudioFreq: Audio frequency to be configured for the SAI peripheral.
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t BSP_AUDIO_IN_Init(uint16_t InputDevice, uint8_t Volume, uint32_t AudioFreq)
{
 uint8_t ret = AUDIO_ERROR;
  uint32_t deviceid = 0x00;
  uint32_t slot_active;

  if ((InputDevice != INPUT_DEVICE_INPUT_LINE_1) && (InputDevice != INPUT_DEVICE_DIGITAL_MICROPHONE_2))			/* Only INPUT_LINE_1 and MICROPHONE_2 inputs supported */
  {
    ret = AUDIO_ERROR;
  }
  else
  {
		__HAL_RCC_SAI1_CLK_ENABLE();
		/** Sw reset of I2C4 IP */
		__HAL_RCC_SAI1_FORCE_RESET();
		WM8994_Delay_1ms( 2 );
		__HAL_RCC_SAI1_RELEASE_RESET();
		WM8994_Delay_1ms( 2 );
		
		__HAL_RCC_DMA2_CLK_ENABLE();
		
		
		/* PLL clock is set depending on the AudioFreq (44.1khz vs 48khz groups) */
    BSP_AUDIO_OUT_ClockConfig(&haudio_in_sai, AudioFreq, NULL); /* Clock config is shared between AUDIO IN and OUT */
		
		
    /* Disable SAI */
    /* Initialize the haudio_in_sai Instance parameter */
		haudio_in_sai.Instance = SAI1_Block_B;
		HAL_SAI_DeInit(&haudio_in_sai);
		
		haudio_out_sai.Instance = SAI1_Block_A;
		HAL_SAI_DeInit(&haudio_out_sai);
		

    
		
		
		/* Configure SAI in master RX mode :
     *   - SAI2_block_A in master RX mode
     *   - SAI2_block_B in slave RX mode synchronous from SAI2_block_A
     */
    if (InputDevice == INPUT_DEVICE_DIGITAL_MICROPHONE_2)
    {
      slot_active = CODEC_AUDIOFRAME_SLOT_13;
    }
    else
    {
      slot_active = CODEC_AUDIOFRAME_SLOT_02;
    }
		
		/* Initialize SAI2 block A in MASTER RX */
		/* Initialize the haudio_out_sai Instance parameter */
		haudio_out_sai.Instance = SAI2_Block_A;

		/* Configure SAI_Block_x
		LSBFirst: Disabled
		DataSize: 16 */
		haudio_out_sai.Init.AudioFrequency = AudioFreq;
		haudio_out_sai.Init.AudioMode = SAI_MODEMASTER_RX;
		haudio_out_sai.Init.NoDivider = SAI_MASTERDIVIDER_ENABLED;
		haudio_out_sai.Init.Protocol = SAI_FREE_PROTOCOL;
		haudio_out_sai.Init.DataSize = SAI_DATASIZE_16;
		haudio_out_sai.Init.FirstBit = SAI_FIRSTBIT_MSB;
		haudio_out_sai.Init.ClockStrobing = SAI_CLOCKSTROBING_RISINGEDGE;
		haudio_out_sai.Init.Synchro = SAI_ASYNCHRONOUS;
		haudio_out_sai.Init.OutputDrive = SAI_OUTPUTDRIVE_ENABLED;
		haudio_out_sai.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_1QF;

		/* Configure SAI_Block_x Frame
		Frame Length: 64
		Frame active Length: 32
		FS Definition: Start frame + Channel Side identification
		FS Polarity: FS active Low
		FS Offset: FS asserted one bit before the first bit of slot 0 */
		haudio_out_sai.FrameInit.FrameLength = 64;
		haudio_out_sai.FrameInit.ActiveFrameLength = 32;
		haudio_out_sai.FrameInit.FSDefinition = SAI_FS_CHANNEL_IDENTIFICATION;
		haudio_out_sai.FrameInit.FSPolarity = SAI_FS_ACTIVE_LOW;
		haudio_out_sai.FrameInit.FSOffset = SAI_FS_BEFOREFIRSTBIT;

		/* Configure SAI Block_x Slot
		Slot First Bit Offset: 0
		Slot Size  : 16
		Slot Number: 4
		Slot Active: All slot actives */
		haudio_out_sai.SlotInit.FirstBitOffset = 0;
		haudio_out_sai.SlotInit.SlotSize = SAI_SLOTSIZE_16B;
		haudio_out_sai.SlotInit.SlotNumber = 4;
		haudio_out_sai.SlotInit.SlotActive = slot_active;

		HAL_SAI_Init(&haudio_out_sai);
		

		/* Initialize SAI2 block B in SLAVE RX synchronous from SAI2 block A */
		/* Initialize the haudio_in_sai Instance parameter */
		haudio_in_sai.Instance = SAI2_Block_B;
		
		/* Configure SAI_Block_x
		LSBFirst: Disabled
		DataSize: 16 */
		haudio_in_sai.Init.AudioFrequency = AudioFreq;
		haudio_in_sai.Init.AudioMode = SAI_MODESLAVE_RX;
		haudio_in_sai.Init.NoDivider = SAI_MASTERDIVIDER_ENABLED;
		haudio_in_sai.Init.Protocol = SAI_FREE_PROTOCOL;
		haudio_in_sai.Init.DataSize = SAI_DATASIZE_16;
		haudio_in_sai.Init.FirstBit = SAI_FIRSTBIT_MSB;
		haudio_in_sai.Init.ClockStrobing = SAI_CLOCKSTROBING_RISINGEDGE;
		haudio_in_sai.Init.Synchro = SAI_SYNCHRONOUS;
		haudio_in_sai.Init.OutputDrive = SAI_OUTPUTDRIVE_DISABLED;
		haudio_in_sai.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_1QF;
		
		/* Configure SAI_Block_x Frame
		Frame Length: 64
		Frame active Length: 32
		FS Definition: Start frame + Channel Side identification
		FS Polarity: FS active Low
		FS Offset: FS asserted one bit before the first bit of slot 0 */
		haudio_in_sai.FrameInit.FrameLength = 64;
		haudio_in_sai.FrameInit.ActiveFrameLength = 32;
		haudio_in_sai.FrameInit.FSDefinition = SAI_FS_CHANNEL_IDENTIFICATION;
		haudio_in_sai.FrameInit.FSPolarity = SAI_FS_ACTIVE_LOW;
		haudio_in_sai.FrameInit.FSOffset = SAI_FS_BEFOREFIRSTBIT;
		
		/* Configure SAI Block_x Slot
		Slot First Bit Offset: 0
		Slot Size  : 16
		Slot Number: 4
		Slot Active: All slot active */
		haudio_in_sai.SlotInit.FirstBitOffset = 0;
		haudio_in_sai.SlotInit.SlotSize = SAI_SLOTSIZE_16B;
		haudio_in_sai.SlotInit.SlotNumber = 4;
		haudio_in_sai.SlotInit.SlotActive = slot_active;

		HAL_SAI_Init(&haudio_in_sai);

		/* Enable SAI peripheral to generate MCLK */
		__HAL_SAI_ENABLE(&haudio_out_sai);
		/* Enable SAI peripheral */
		__HAL_SAI_ENABLE(&haudio_in_sai);
		
		
		/* Configure the hdma_sai_rx handle parameters */
    hdma_sai_rx.Init.Channel             =  DMA_CHANNEL_0;
    hdma_sai_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma_sai_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_sai_rx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_sai_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
    hdma_sai_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
    hdma_sai_rx.Init.Mode                = DMA_CIRCULAR;
    hdma_sai_rx.Init.Priority            = DMA_PRIORITY_HIGH;
    hdma_sai_rx.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;
    hdma_sai_rx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    hdma_sai_rx.Init.MemBurst            = DMA_MBURST_SINGLE;
    hdma_sai_rx.Init.PeriphBurst         = DMA_MBURST_SINGLE;
    
    hdma_sai_rx.Instance = DMA2_Stream7;
    
    /* Associate the DMA handle */
    __HAL_LINKDMA(&haudio_in_sai, hdmarx, hdma_sai_rx);
    
    /* Deinitialize the Stream for new transfer */
    HAL_DMA_DeInit(&hdma_sai_rx);
    
    /* Configure the DMA Stream */
    HAL_DMA_Init(&hdma_sai_rx);
  
		/* SAI DMA IRQ Channel configuration */
		HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 4, 0);
		HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);
		
		if( wm8994_drv.Init(AUDIO_I2C_ADDRESS, InputDevice, Volume, AudioFreq) != 0 ) return AUDIO_ERROR;

    /* wm8994 codec initialization */
    deviceid = wm8994_drv.ReadID(AUDIO_I2C_ADDRESS);

    if((deviceid) == WM8994_ID)
    {
      ret = AUDIO_OK;
    }
    else
    {
      ret = AUDIO_ERROR;
    }
  }
  return ret;
}





/**
  * @brief  Initializes wave recording and playback in parallel.
  * @param  InputDevice: INPUT_DEVICE_DIGITAL_MICROPHONE_2
  * @param  OutputDevice: OUTPUT_DEVICE_SPEAKER, OUTPUT_DEVICE_HEADPHONE,
  *                       or OUTPUT_DEVICE_BOTH.
  * @param  Volume: Initial volume level (in range 0(Mute)..80(+0dB)..100(+17.625dB))
  * @param  AudioFreq: Audio frequency to be configured for the SAI peripheral.
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t BSP_AUDIO_IN_OUT_Init(uint16_t InputDevice, uint16_t OutputDevice, uint8_t Volume, uint32_t AudioFreq)
{
  uint8_t ret = AUDIO_ERROR;
  uint32_t deviceid = 0x00;
  uint32_t slot_active;

  if (InputDevice != INPUT_DEVICE_DIGITAL_MICROPHONE_2)	return AUDIO_ERROR;
  
 /* Disable SAI */
	/* Initialize the haudio_in_sai Instance parameter */
	haudio_in_sai.Instance = SAI2_Block_B;

	/* Disable SAI peripheral */
	__HAL_SAI_DISABLE(&haudio_in_sai);
	HAL_SAI_DeInit(&haudio_in_sai);
	
	haudio_out_sai.Instance = SAI2_Block_A;

	/* Disable SAI peripheral */
	__HAL_SAI_DISABLE(&haudio_out_sai);
	HAL_SAI_DeInit(&haudio_out_sai);

	/* PLL clock is set depending on the AudioFreq (44.1khz vs 48khz groups) */
	BSP_AUDIO_OUT_ClockConfig(&haudio_in_sai, AudioFreq, NULL); /* Clock config is shared between AUDIO IN and OUT */
	
	__HAL_RCC_SAI2_CLK_ENABLE();
	__HAL_RCC_DMA2_CLK_ENABLE();
	
	/* Configure SAI in master mode :
	 *   - SAI2_block_A in master TX mode
	 *   - SAI2_block_B in slave RX mode synchronous from SAI2_block_A
	 */
	if (InputDevice == INPUT_DEVICE_DIGITAL_MICROPHONE_2)
	{
		slot_active = CODEC_AUDIOFRAME_SLOT_13;
	}
	else
	{
		slot_active = CODEC_AUDIOFRAME_SLOT_02;
	}
	
	/* Initialize the haudio_out_sai Instance parameter */
	haudio_out_sai.Instance = SAI2_Block_A;

	/* Configure SAI_Block_x
	LSBFirst: Disabled
	DataSize: 16 */
	haudio_out_sai.Init.AudioFrequency = AudioFreq;
	haudio_out_sai.Init.AudioMode = SAI_MODEMASTER_TX;
	haudio_out_sai.Init.NoDivider = SAI_MASTERDIVIDER_ENABLED;
	haudio_out_sai.Init.Protocol = SAI_FREE_PROTOCOL;
	haudio_out_sai.Init.DataSize = SAI_DATASIZE_16;
	haudio_out_sai.Init.FirstBit = SAI_FIRSTBIT_MSB;
	haudio_out_sai.Init.ClockStrobing = SAI_CLOCKSTROBING_RISINGEDGE;
	haudio_out_sai.Init.Synchro = SAI_ASYNCHRONOUS;
	haudio_out_sai.Init.OutputDrive = SAI_OUTPUTDRIVE_ENABLED;
	haudio_out_sai.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_1QF;

	/* Configure SAI_Block_x Frame
	Frame Length: 64
	Frame active Length: 32
	FS Definition: Start frame + Channel Side identification
	FS Polarity: FS active Low
	FS Offset: FS asserted one bit before the first bit of slot 0 */
	haudio_out_sai.FrameInit.FrameLength = 64;
	haudio_out_sai.FrameInit.ActiveFrameLength = 32;
	haudio_out_sai.FrameInit.FSDefinition = SAI_FS_CHANNEL_IDENTIFICATION;
	haudio_out_sai.FrameInit.FSPolarity = SAI_FS_ACTIVE_LOW;
	haudio_out_sai.FrameInit.FSOffset = SAI_FS_BEFOREFIRSTBIT;

	/* Configure SAI Block_x Slot
	Slot First Bit Offset: 0
	Slot Size  : 16
	Slot Number: 4
	Slot Active: All slot actives */
	haudio_out_sai.SlotInit.FirstBitOffset = 0;
	haudio_out_sai.SlotInit.SlotSize = SAI_SLOTSIZE_16B;
	haudio_out_sai.SlotInit.SlotNumber = 4;
	haudio_out_sai.SlotInit.SlotActive = slot_active;

	HAL_SAI_Init(&haudio_out_sai);
	

	/* Initialize SAI2 block B in SLAVE RX synchronous from SAI2 block A */
	/* Initialize the haudio_in_sai Instance parameter */
	haudio_in_sai.Instance = SAI2_Block_B;
	
	/* Configure SAI_Block_x
	LSBFirst: Disabled
	DataSize: 16 */
	haudio_in_sai.Init.AudioFrequency = AudioFreq;
	haudio_in_sai.Init.AudioMode = SAI_MODESLAVE_RX;
	haudio_in_sai.Init.NoDivider = SAI_MASTERDIVIDER_ENABLED;
	haudio_in_sai.Init.Protocol = SAI_FREE_PROTOCOL;
	haudio_in_sai.Init.DataSize = SAI_DATASIZE_16;
	haudio_in_sai.Init.FirstBit = SAI_FIRSTBIT_MSB;
	haudio_in_sai.Init.ClockStrobing = SAI_CLOCKSTROBING_RISINGEDGE;
	haudio_in_sai.Init.Synchro = SAI_SYNCHRONOUS;
	haudio_in_sai.Init.OutputDrive = SAI_OUTPUTDRIVE_DISABLED;
	haudio_in_sai.Init.FIFOThreshold = SAI_FIFOTHRESHOLD_1QF;
	
	/* Configure SAI_Block_x Frame
	Frame Length: 64
	Frame active Length: 32
	FS Definition: Start frame + Channel Side identification
	FS Polarity: FS active Low
	FS Offset: FS asserted one bit before the first bit of slot 0 */
	haudio_in_sai.FrameInit.FrameLength = 64;
	haudio_in_sai.FrameInit.ActiveFrameLength = 32;
	haudio_in_sai.FrameInit.FSDefinition = SAI_FS_CHANNEL_IDENTIFICATION;
	haudio_in_sai.FrameInit.FSPolarity = SAI_FS_ACTIVE_LOW;
	haudio_in_sai.FrameInit.FSOffset = SAI_FS_BEFOREFIRSTBIT;
	
	/* Configure SAI Block_x Slot
	Slot First Bit Offset: 0
	Slot Size  : 16
	Slot Number: 4
	Slot Active: All slot active */
	haudio_in_sai.SlotInit.FirstBitOffset = 0;
	haudio_in_sai.SlotInit.SlotSize = SAI_SLOTSIZE_16B;
	haudio_in_sai.SlotInit.SlotNumber = 4;
	haudio_in_sai.SlotInit.SlotActive = slot_active;

	HAL_SAI_Init(&haudio_in_sai);

	/* Enable SAI peripheral to generate MCLK */
	__HAL_SAI_ENABLE(&haudio_out_sai);
	/* Enable SAI peripheral */
	__HAL_SAI_ENABLE(&haudio_in_sai);
	
	
	/* Configure the hdma_saiTx handle parameters */   
	hdma_sai_tx.Init.Channel             = DMA_CHANNEL_3;
	hdma_sai_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
	hdma_sai_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
	hdma_sai_tx.Init.MemInc              = DMA_MINC_ENABLE;
	hdma_sai_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
	hdma_sai_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
	hdma_sai_tx.Init.Mode                = DMA_CIRCULAR;
	hdma_sai_tx.Init.Priority            = DMA_PRIORITY_HIGH;
	hdma_sai_tx.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;         
	hdma_sai_tx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
	hdma_sai_tx.Init.MemBurst            = DMA_MBURST_SINGLE;
	hdma_sai_tx.Init.PeriphBurst         = DMA_PBURST_SINGLE; 
	
	hdma_sai_tx.Instance =  DMA2_Stream4;
    
	/* Associate the DMA handle */
	__HAL_LINKDMA(&haudio_out_sai, hdmatx, hdma_sai_tx);
	
	/* Deinitialize the Stream for new transfer */
	HAL_DMA_DeInit(&hdma_sai_tx);
	
	/* Configure the DMA Stream */
	HAL_DMA_Init(&hdma_sai_tx); 
  
  /* SAI DMA IRQ Channel configuration */
  HAL_NVIC_SetPriority(DMA2_Stream4_IRQn, 2, 0);
  HAL_NVIC_EnableIRQ(DMA2_Stream4_IRQn); 
	
	
	/* Configure the hdma_sai_rx handle parameters */
	hdma_sai_rx.Init.Channel             =  DMA_CHANNEL_0;
	hdma_sai_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
	hdma_sai_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
	hdma_sai_rx.Init.MemInc              = DMA_MINC_ENABLE;
	hdma_sai_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
	hdma_sai_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_HALFWORD;
	hdma_sai_rx.Init.Mode                = DMA_CIRCULAR;
	hdma_sai_rx.Init.Priority            = DMA_PRIORITY_HIGH;
	hdma_sai_rx.Init.FIFOMode            = DMA_FIFOMODE_ENABLE;
	hdma_sai_rx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
	hdma_sai_rx.Init.MemBurst            = DMA_MBURST_SINGLE;
	hdma_sai_rx.Init.PeriphBurst         = DMA_MBURST_SINGLE;
	
	hdma_sai_rx.Instance = DMA2_Stream7;
	
	/* Associate the DMA handle */
	__HAL_LINKDMA(&haudio_in_sai, hdmarx, hdma_sai_rx);
	
	/* Deinitialize the Stream for new transfer */
	HAL_DMA_DeInit(&hdma_sai_rx);
	
	/* Configure the DMA Stream */
	HAL_DMA_Init(&hdma_sai_rx);

	/* SAI DMA IRQ Channel configuration */
	HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 3, 0);
	HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);
	
	if( wm8994_drv.Init(AUDIO_I2C_ADDRESS, InputDevice | OutputDevice, Volume, AudioFreq) != 0 ) return AUDIO_ERROR;
	
	/* wm8994 codec initialization */
	deviceid = wm8994_drv.ReadID(AUDIO_I2C_ADDRESS);

	if((deviceid) == WM8994_ID)
	{
		ret = AUDIO_OK;
	}
	else
	{
		ret = AUDIO_ERROR;
	}

  return ret;
}


/**
  * @brief  Starts audio recording.
  * @param  pbuf: Main buffer pointer for the recorded data storing  
  * @param  size: size of the recorded buffer in number of elements (typically number of half-words)
  *               Be careful that it is not the same unit than BSP_AUDIO_OUT_Play function
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t  BSP_AUDIO_IN_Record(uint16_t* pbuf, uint32_t size)
{
  uint32_t ret = AUDIO_ERROR;
  
  /* Start the process receive DMA */
  HAL_SAI_Receive_DMA(&haudio_in_sai, (uint8_t*)pbuf, size);
  
  /* Return AUDIO_OK when all operations are correctly done */
  ret = AUDIO_OK;
  
  return ret;
}

/**
  * @brief  Stops audio recording.
  * @param  Option: could be one of the following parameters
  *           - CODEC_PDWN_SW: for software power off (by writing registers).
  *                            Then no need to reconfigure the Codec after power on.
  *           - CODEC_PDWN_HW: completely shut down the codec (physically).
  *                            Then need to reconfigure the Codec after power on.
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t BSP_AUDIO_IN_Stop(uint32_t Option)
{
  /* Call the Media layer stop function */
  HAL_SAI_DMAStop(&haudio_in_sai);
  
  /* Call Audio Codec Stop function */
  if(wm8994_drv.Stop(AUDIO_I2C_ADDRESS, Option) != 0)
  {
    return AUDIO_ERROR;
  }
  else
  {
    if(Option == CODEC_PDWN_HW)
    {
      /* Wait at least 100us */
      WM8994_Delay_1ms( 1 );
    }
    /* Return AUDIO_OK when all operations are correctly done */
    return AUDIO_OK;
  }
}

/**
  * @brief  Pauses the audio file stream.
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t BSP_AUDIO_IN_Pause(void)
{    
  /* Call the Media layer pause function */
  HAL_SAI_DMAPause(&haudio_in_sai);
  /* Return AUDIO_OK when all operations are correctly done */
  return AUDIO_OK;
}

/**
  * @brief  Resumes the audio file stream.
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t BSP_AUDIO_IN_Resume(void)
{    
  /* Call the Media layer pause/resume function */
  HAL_SAI_DMAResume(&haudio_in_sai);
  /* Return AUDIO_OK when all operations are correctly done */
  return AUDIO_OK;
}

/**
  * @brief  Controls the audio in volume level. 
  * @param  Volume: Volume level in range 0(Mute)..80(+0dB)..100(+17.625dB)
  * @retval AUDIO_OK if correct communication, else wrong communication
  */
uint8_t BSP_AUDIO_IN_SetVolume(uint8_t Volume)
{
  /* Call the codec volume control function with converted volume value */
  if(wm8994_drv.SetVolume(AUDIO_I2C_ADDRESS, Volume) != 0)
  {
    return AUDIO_ERROR;
  }
  else
  {
    /* Set the Global variable AudioInVolume  */
    AudioInVolume = Volume;
    /* Return AUDIO_OK when all operations are correctly done */
    return AUDIO_OK;
  }
}



 /**
  * @brief  Rx Transfer completed callbacks.
  * @param  hsai: SAI handle
  * @retval None
  */
void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)
{
  /* Call the record update function to get the next buffer to fill and its size (size is ignored) */
	RxCpltFlag = 1;
}

/**
  * @brief  Rx Half Transfer completed callbacks.
  * @param  hsai: SAI handle
  * @retval None
  */
void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
  /* Manage the remaining file size and new address offset: This function 
     should be coded by user (its prototype is already declared in stm32746g_discovery_audio.h) */
	RxHalfCpltFlag = 1;
}



/**
  * @brief  Audio IN Error callback function.
  * @retval None
  */
__weak void BSP_AUDIO_IN_Error_CallBack(void)
{   
  /* This function is called when an Interrupt due to transfer error on or peripheral
     error occurs. */
}




/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
