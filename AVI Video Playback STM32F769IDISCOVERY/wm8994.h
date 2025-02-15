/**
  ******************************************************************************
  * @file    wm8994.h
  * @author  MCD Application Team
  * @version V2.0.0
  * @date    24-June-2015
  * @brief   This file contains all the functions prototypes for the 
  *          wm8994.c driver.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WM8994_H
#define __WM8994_H


/******************************************************************************/
/***************************  Codec User defines ******************************/
/******************************************************************************/
/* Codec output DEVICE */
#define OUTPUT_DEVICE_SPEAKER                 ((uint16_t)0x0001)
#define OUTPUT_DEVICE_HEADPHONE               ((uint16_t)0x0002)
#define OUTPUT_DEVICE_BOTH                    ((uint16_t)0x0003)
#define OUTPUT_DEVICE_AUTO                    ((uint16_t)0x0004)
#define INPUT_DEVICE_DIGITAL_MICROPHONE_1     ((uint16_t)0x0100)
#define INPUT_DEVICE_DIGITAL_MICROPHONE_2     ((uint16_t)0x0200)
#define INPUT_DEVICE_INPUT_LINE_1             ((uint16_t)0x0300)
#define INPUT_DEVICE_INPUT_LINE_2             ((uint16_t)0x0400)
#define INPUT_DEVICE_DIGITAL_MIC1_MIC2        ((uint16_t)0x0800)

/* Volume Levels values */
#define DEFAULT_VOLMIN                0x00
#define DEFAULT_VOLMAX                0xFF
#define DEFAULT_VOLSTEP               0x04

#define AUDIO_PAUSE                   0
#define AUDIO_RESUME                  1

/* Codec POWER DOWN modes */
#define CODEC_PDWN_HW                 1
#define CODEC_PDWN_SW                 2

/* MUTE commands */
#define AUDIO_MUTE_ON                 1
#define AUDIO_MUTE_OFF                0

/* AUDIO FREQUENCY */
#define AUDIO_FREQUENCY_192K          ((unsigned int)192000)
#define AUDIO_FREQUENCY_96K           ((unsigned int)96000)
#define AUDIO_FREQUENCY_48K           ((unsigned int)48000)
#define AUDIO_FREQUENCY_44K           ((unsigned int)44100)
#define AUDIO_FREQUENCY_32K           ((unsigned int)32000)
#define AUDIO_FREQUENCY_22K           ((unsigned int)22050)
#define AUDIO_FREQUENCY_16K           ((unsigned int)16000)
#define AUDIO_FREQUENCY_11K           ((unsigned int)11025)
#define AUDIO_FREQUENCY_8K            ((unsigned int)8000)  

#define VOLUME_CONVERT(Volume)        (((Volume) > 100)? 100:((unsigned char)(((Volume) * 63) / 100)))
#define VOLUME_IN_CONVERT(Volume)     (((Volume) >= 100)? 239:((unsigned char)(((Volume) * 240) / 100)))

/******************************************************************************/
/****************************** REGISTER MAPPING ******************************/
/******************************************************************************/
/** 
  * @brief  WM8994 ID  
  */  
#define  WM8994_ID    0x8994

/**
  * @brief Device ID Register: Reading from this register will indicate device 
  *                            family ID 8994h
  */
#define WM8994_CHIPID_ADDR                  0x00





typedef struct
{
  unsigned int  (*Init)(unsigned short int, unsigned short int, unsigned char, unsigned int);
  void      (*DeInit)(void);
  unsigned int  (*ReadID)(unsigned short int);
  unsigned int  (*Play)(unsigned short int, unsigned short int*, unsigned short int);
  unsigned int  (*Pause)(unsigned short int);
  unsigned int  (*Resume)(unsigned short int);
  unsigned int  (*Stop)(unsigned short int, unsigned int);
  unsigned int  (*SetFrequency)(unsigned short int, unsigned int);
  unsigned int  (*SetVolume)(unsigned short int, unsigned char);
  unsigned int  (*SetMute)(unsigned short int, unsigned int);
  unsigned int  (*SetOutputMode)(unsigned short int, unsigned char);
  unsigned int  (*Reset)(unsigned short int);
}AUDIO_DrvTypeDef;




/**
  * @}
  */ 

/** @defgroup WM8994_Exported_Macros
  * @{
  */ 
/**
  * @}
  */ 

/** @defgroup WM8994_Exported_Functions
  * @{
  */
    
/*------------------------------------------------------------------------------
                           Audio Codec functions 
------------------------------------------------------------------------------*/
/* High Layer codec functions */
void WM8994_GPIO_Config( void );
void WM8994_I2C_Config( void );
void WM8994_Delay_1ms( unsigned int ms );




unsigned int wm8994_Init(unsigned short int DeviceAddr, unsigned short int OutputInputDevice, unsigned char Volume, unsigned int AudioFreq);
void     wm8994_DeInit(void);
unsigned int wm8994_ReadID(unsigned short int DeviceAddr);
unsigned int wm8994_Play(unsigned short int DeviceAddr, unsigned short int* pBuffer, unsigned short int Size);
unsigned int wm8994_Pause(unsigned short int DeviceAddr);
unsigned int wm8994_Resume(unsigned short int DeviceAddr);
unsigned int wm8994_Stop(unsigned short int DeviceAddr, unsigned int Cmd);
unsigned int wm8994_SetVolume(unsigned short int DeviceAddr, unsigned char Volume);
unsigned int wm8994_SetMute(unsigned short int DeviceAddr, unsigned int Cmd);
unsigned int wm8994_SetOutputMode(unsigned short int DeviceAddr, unsigned char Output);
unsigned int wm8994_SetFrequency(unsigned short int DeviceAddr, unsigned int AudioFreq);
unsigned int wm8994_Reset(unsigned short int DeviceAddr);


/* Audio driver structure */
extern AUDIO_DrvTypeDef   wm8994_drv;

#endif /* __WM8994_H */

/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  *//**
  ******************************************************************************
  * @file    wm8994.h
  * @author  MCD Application Team
  * @version V2.0.0
  * @date    24-June-2015
  * @brief   This file contains all the functions prototypes for the 
  *          wm8994.c driver.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2015 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __WM8994_H
#define __WM8994_H




/** @defgroup WM8994_Exported_Constants
  * @{
  */ 

/******************************************************************************/
/***************************  Codec User defines ******************************/
/******************************************************************************/
/* Codec output DEVICE */
#define OUTPUT_DEVICE_SPEAKER                 ((unsigned short int)0x0001)
#define OUTPUT_DEVICE_HEADPHONE               ((unsigned short int)0x0002)
#define OUTPUT_DEVICE_BOTH                    ((unsigned short int)0x0003)
#define OUTPUT_DEVICE_AUTO                    ((unsigned short int)0x0004)
#define INPUT_DEVICE_DIGITAL_MICROPHONE_1     ((unsigned short int)0x0100)
#define INPUT_DEVICE_DIGITAL_MICROPHONE_2     ((unsigned short int)0x0200)
#define INPUT_DEVICE_INPUT_LINE_1             ((unsigned short int)0x0300)
#define INPUT_DEVICE_INPUT_LINE_2             ((unsigned short int)0x0400)

/* Volume Levels values */
#define DEFAULT_VOLMIN                0x00
#define DEFAULT_VOLMAX                0xFF
#define DEFAULT_VOLSTEP               0x04

#define AUDIO_PAUSE                   0
#define AUDIO_RESUME                  1

/* Codec POWER DOWN modes */
#define CODEC_PDWN_HW                 1
#define CODEC_PDWN_SW                 2

/* MUTE commands */
#define AUDIO_MUTE_ON                 1
#define AUDIO_MUTE_OFF                0

/* AUDIO FREQUENCY */
#define AUDIO_FREQUENCY_192K          ((unsigned int)192000)
#define AUDIO_FREQUENCY_96K           ((unsigned int)96000)
#define AUDIO_FREQUENCY_48K           ((unsigned int)48000)
#define AUDIO_FREQUENCY_44K           ((unsigned int)44100)
#define AUDIO_FREQUENCY_32K           ((unsigned int)32000)
#define AUDIO_FREQUENCY_22K           ((unsigned int)22050)
#define AUDIO_FREQUENCY_16K           ((unsigned int)16000)
#define AUDIO_FREQUENCY_11K           ((unsigned int)11025)
#define AUDIO_FREQUENCY_8K            ((unsigned int)8000)  

#define VOLUME_CONVERT(Volume)        (((Volume) > 100)? 100:((unsigned char)(((Volume) * 63) / 100)))
#define VOLUME_IN_CONVERT(Volume)     (((Volume) >= 100)? 239:((unsigned char)(((Volume) * 240) / 100)))

/******************************************************************************/
/****************************** REGISTER MAPPING ******************************/
/******************************************************************************/
/** 
  * @brief  WM8994 ID  
  */  
#define  WM8994_ID    0x8994

/**
  * @brief Device ID Register: Reading from this register will indicate device 
  *                            family ID 8994h
  */
#define WM8994_CHIPID_ADDR                  0x00

/**
  * @}
  */ 

/** @defgroup WM8994_Exported_Macros
  * @{
  */ 
/**
  * @}
  */ 

typedef struct
{
  unsigned int  (*Init)(unsigned short int, unsigned short int, unsigned char, unsigned int);
  void      (*DeInit)(void);
  unsigned int  (*ReadID)(unsigned short int);
  unsigned int  (*Play)(unsigned short int, unsigned short int*, unsigned short int);
  unsigned int  (*Pause)(unsigned short int);
  unsigned int  (*Resume)(unsigned short int);
  unsigned int  (*Stop)(unsigned short int, unsigned int);
  unsigned int  (*SetFrequency)(unsigned short int, unsigned int);
  unsigned int  (*SetVolume)(unsigned short int, unsigned char);
  unsigned int  (*SetMute)(unsigned short int, unsigned int);
  unsigned int  (*SetOutputMode)(unsigned short int, unsigned char);
  unsigned int  (*Reset)(unsigned short int);
}AUDIO_DrvTypeDef;

    
/*------------------------------------------------------------------------------
                           Audio Codec functions 
------------------------------------------------------------------------------*/
/* High Layer codec functions */
unsigned int wm8994_Init(unsigned short int DeviceAddr, unsigned short int OutputInputDevice, unsigned char Volume, unsigned int AudioFreq);
void     wm8994_DeInit(void);
unsigned int wm8994_ReadID(unsigned short int DeviceAddr);
unsigned int wm8994_Play(unsigned short int DeviceAddr, unsigned short int* pBuffer, unsigned short int Size);
unsigned int wm8994_Pause(unsigned short int DeviceAddr);
unsigned int wm8994_Resume(unsigned short int DeviceAddr);
unsigned int wm8994_Stop(unsigned short int DeviceAddr, unsigned int Cmd);
unsigned int wm8994_SetVolume(unsigned short int DeviceAddr, unsigned char Volume);
unsigned int wm8994_SetMute(unsigned short int DeviceAddr, unsigned int Cmd);
unsigned int wm8994_SetOutputMode(unsigned short int DeviceAddr, unsigned char Output);
unsigned int wm8994_SetFrequency(unsigned short int DeviceAddr, unsigned int AudioFreq);
unsigned int wm8994_Reset(unsigned short int DeviceAddr);

void WM8994_GPIO_Config( void );
void WM8994_I2C_Config( void );

void WM8994_Delay_1ms( unsigned int ms );




/* Audio driver structure */
extern AUDIO_DrvTypeDef   wm8994_drv;

#endif /* __WM8994_H */

/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */

/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/


/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
