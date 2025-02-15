#ifndef __SDRAM_H
#define __SDRAM_H
#endif


#define SDRAM_BANK1_ADDR															((uint32_t)0xC0000000)
#define SDRAM_BANK2_ADDR															((uint32_t)0xC0000000+0x400000)
#define SDRAM_BANK3_ADDR															((uint32_t)0xC0000000+0x800000)
#define SDRAM_BANK4_ADDR															((uint32_t)0xC0000000+0xc00000)
#define SDRAM_BUFFER																	(uint32_t)0X600000
#define AVI_IN_BUF1																		(uint32_t)0XE80000
#define AVI_IN_BUF2																		(uint32_t)0XEC0000
#define AVI_Frame_Index																(uint32_t)0XF00000
#define AVI_Audio_Index																(uint32_t)0XF80000

//#define HanZiFont_Buffer_Addr													(uint32_t)0X500000
//#define BACKGROUND_BUFFER_ADDR												(uint32_t)0X680000
//#define MAP_BUFFER_ADDR																(uint32_t)0X700000
//#define PNG_DECODE_ADDR																(uint32_t)0X780000



unsigned char SDRAM_Init( void );																																					//初始化SDRAM

void SDRAM_Write_8B( unsigned int addr, unsigned char *buf, unsigned int num );														//Writes 8-bit data buffer to SDRAM memory.

void SDRAM_Read_8B( unsigned int addr, unsigned char *buf, unsigned int num );														//Reads 8-bit data buffer from the SDRAM memory.

void SDRAM_Write_16B( unsigned int addr, unsigned short int *buf, unsigned int num );											//Writes 16-bit data buffer to SDRAM memory.

void SDRAM_Read_16B( unsigned int addr, unsigned short int *buf, unsigned int num );											//Reads 16-bit data buffer from the SDRAM memory.

void SDRAM_Write_32B( unsigned int addr, unsigned int *buf, unsigned int num );														// Writes 32-bit data buffer to SDRAM memory.

void SDRAM_Read_32B( unsigned int addr, unsigned int *buf, unsigned int num );														//Reads 32-bit data buffer from the SDRAM memory.

#ifdef HAL_SDRAM_MODULE_ENABLED

unsigned char Load_File_From_SD( unsigned char *file_name, unsigned int *file_size, unsigned int addr );						//从SD卡加载文件到SDRAM

#endif

#ifdef  HAL_FLASH_MODULE_ENABLED

unsigned char Load_File_From_Flsah( unsigned int flash_addr, unsigned int file_size, unsigned int sdram_addr );	//从Flash加载文件到SDRAM

#endif


