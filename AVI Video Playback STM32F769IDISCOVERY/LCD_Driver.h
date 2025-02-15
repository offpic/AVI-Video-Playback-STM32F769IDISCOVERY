#ifndef __LCD_DRIVER_H
#define	__LCD_DRIVER_H
#endif




#define LCD_COLOR_BLUE          ((uint32_t)0xff0000FF)
#define LCD_COLOR_GREEN         ((uint32_t)0xFF00FF00)
#define LCD_COLOR_RED           ((uint32_t)0xffFF0000)
#define LCD_COLOR_CYAN          ((uint32_t)0xFF00FFFF)
#define LCD_COLOR_MAGENTA       ((uint32_t)0xFFFF00FF)
#define LCD_COLOR_YELLOW        ((uint32_t)0xFFFFFF00)
#define LCD_COLOR_LIGHTBLUE     ((uint32_t)0xFF8080FF)
#define LCD_COLOR_LIGHTGREEN    ((uint32_t)0xFF80FF80)
#define LCD_COLOR_LIGHTRED      ((uint32_t)0xFFFF8080)
#define LCD_COLOR_LIGHTCYAN     ((uint32_t)0xFF80FFFF)
#define LCD_COLOR_LIGHTMAGENTA  ((uint32_t)0xFFFF80FF)
#define LCD_COLOR_LIGHTYELLOW   ((uint32_t)0xFFFFFF80)
#define LCD_COLOR_DARKBLUE      ((uint32_t)0xFF000080)
#define LCD_COLOR_DARKGREEN     ((uint32_t)0xFF008000)
#define LCD_COLOR_DARKRED       ((uint32_t)0xFF800000)
#define LCD_COLOR_DARKCYAN      ((uint32_t)0xFF008080)
#define LCD_COLOR_DARKMAGENTA   ((uint32_t)0xFF800080)
#define LCD_COLOR_DARKYELLOW    ((uint32_t)0xFF808000)
#define LCD_COLOR_WHITE         ((uint32_t)0xFFFFFFFF)
#define LCD_COLOR_LIGHTGRAY     ((uint32_t)0xFFD3D3D3)
#define LCD_COLOR_GRAY          ((uint32_t)0xFF808080)
#define LCD_COLOR_DARKGRAY      ((uint32_t)0xFF404040)
#define LCD_COLOR_BLACK         ((uint32_t)0xFF000000)
#define LCD_COLOR_BROWN         ((uint32_t)0xFFA52A2A)
#define LCD_COLOR_ORANGE        ((uint32_t)0xFFFFA500)
#define LCD_COLOR_TRANSPARENT   ((uint32_t)0xFF000000)


/** 
  * @brief  RK043FN48H Size  
  */     
//#define  RK043FN48H_WIDTH    ((uint16_t)480)          /* LCD PIXEL WIDTH            */
//#define  RK043FN48H_HEIGHT   ((uint16_t)272)          /* LCD PIXEL HEIGHT           */

/** 
  * @brief  RK043FN48H Timing  
  */     
#define  RK043FN48H_HSYNC            ((uint16_t)41)   /* Horizontal synchronization */
#define  RK043FN48H_HBP              ((uint16_t)13)   /* Horizontal back porch      */
#define  RK043FN48H_HFP              ((uint16_t)32)   /* Horizontal front porch     */
#define  RK043FN48H_VSYNC            ((uint16_t)10)   /* Vertical synchronization   */
#define  RK043FN48H_VBP              ((uint16_t)2)    /* Vertical back porch        */
#define  RK043FN48H_VFP              ((uint16_t)2)    /* Vertical front porch       */



#define LCD_LAYER0_FRAME_BUFFER  ((unsigned int)0xC0000000)
#define LCD_LAYER1_FRAME_BUFFER  ((unsigned int)0xC02F0000)



typedef struct 
{
  signed short int X;
  signed short int Y;
}Point, * pPoint; 




void LCD_Init( unsigned char orientation );																										//LCD的初始化函数

//void LCD_DisplayOn( void );																																	//Enables the display

//void LCD_DisplayOff(void);																																	//Disables the display.

void LCD_SetTransparency( unsigned int LayerIndex, unsigned char Transparency );						//Configures the transparency.

void LCD_SetLayerVisible( unsigned int LayerIndex, unsigned char State);										//Sets an LCD Layer visible

void LCD_SetLayerWindow( unsigned short int LayerIndex, unsigned short int Xpos, unsigned short int Ypos, unsigned short int Width, unsigned short int Height);					//Sets display window.

void LCD_Clear( unsigned int LayerIndex, unsigned int Color);																// Clears the hole LCD.

void LCD_DrawHLine( unsigned int LayerIndex, unsigned short int Xpos, unsigned short int Ypos, unsigned short int Length, unsigned int Color );													//Draws an horizontal line.

//void LCD_DrawVLine( unsigned int LayerIndex, unsigned short int Xpos, unsigned short int Ypos, unsigned short int Length, unsigned int Color );													//Draws a vertical line

//void LCD_DrawPixel( unsigned int LayerIndex, unsigned short int Xpos, unsigned short int Ypos, unsigned int Color);		//Draws a pixel on LCD.
	
unsigned int LCD_ReadPixel( unsigned int LayerIndex, unsigned short int Xpos, unsigned short int Ypos);						//Reads an LCD pixel.

//void LCD_DrawLine( unsigned int LayerIndex, unsigned short int x1, unsigned short int y1, unsigned short int x2, unsigned short int y2, unsigned int Color );						//Draws an uni-line (between two points).

//void LCD_DrawRect(unsigned int LayerIndex, unsigned short int Xpos, unsigned short int Ypos, unsigned short int Width, unsigned short int Height, unsigned int Color);	//Draws a rectangle.

//void LCD_FillRect( unsigned int LayerIndex, unsigned short int Xpos, unsigned short int Ypos, unsigned short int Width, unsigned short int Height, unsigned int Color );	//Draws a full rectangle.
	
void LCD_DrawCircle( unsigned int LayerIndex, unsigned short int Xpos, unsigned short int Ypos, unsigned short int Radius, unsigned int Color );												//Draws a circle

void LCD_FillCircle( unsigned int LayerIndex, unsigned short int Xpos, unsigned short int Ypos, unsigned short int Radius, unsigned int Color );												//Draws a full circle

//void LCD_DrawPolygon( unsigned int LayerIndex, pPoint Points, unsigned short int PointCount, unsigned int Color );			//Draws an poly-line (between many points)

//void LCD_DrawEllipse( unsigned int LayerIndex, unsigned short int Xpos, unsigned short int Ypos, unsigned short int XRadius, unsigned short int YRadius, unsigned int Color);//Draws an ellipse on LCD.

//void LCD_DrawEllipse( unsigned int LayerIndex, unsigned short int Xpos, unsigned short int Ypos, unsigned short int XRadius, unsigned short int YRadius, unsigned int Color);	//Draws an ellipse on LCD.

//void LCD_DrawBitmap( unsigned int LayerIndex, unsigned int Xpos, unsigned int Ypos, unsigned char *pbmp );							//Draws a bitmap picture loaded in the internal Flash in RGB888 format (24 bits per pixel).

//unsigned char disp_8x16character( unsigned int LayerIndex, unsigned int x, unsigned int y, unsigned char *str, unsigned char num, unsigned int ZiFu_Color, unsigned int Background_Color );	//指定位置(x,y)显示字符串(不能显示汉字）

//unsigned char disp_8x16character1( unsigned int LayerIndex, unsigned int x, unsigned int y, unsigned char *str, unsigned char num, unsigned int ZiFu_Color, unsigned int Background_Color );	//指定位置(x,y)显示字符串(不能显示汉字）

//unsigned char disp_16x24character( unsigned int LayerIndex, unsigned int x, unsigned int y, unsigned char *str, unsigned char num, unsigned int ZiFu_Color, unsigned int Background_Color );	//指定位置(x,y)显示字符串(不能显示汉字）

//unsigned char disp_16x24character1( unsigned int LayerIndex, unsigned int x, unsigned int y, unsigned char *str, unsigned char num, unsigned int ZiFu_Color, unsigned int Background_Color );	//指定位置(x,y)显示字符串(不能显示汉字）

//unsigned char disp_16x16GB( unsigned int LayerIndex, unsigned int x, unsigned int y, unsigned char *str,  unsigned char lenth, unsigned int ZiFu_Color, unsigned int Background_Color );			//指定位置(x,y)显示汉字(不能显示字符）

//unsigned char disp_16x16GB1( unsigned int LayerIndex, unsigned int x, unsigned int y, unsigned char *str,  unsigned char lenth, unsigned int ZiFu_Color, unsigned int Background_Color );			//指定位置(x,y)显示汉字(不能显示字符）

//unsigned char disp_24x24GB( unsigned int LayerIndex, unsigned int x, unsigned int y, unsigned char *str,  unsigned char lenth, unsigned int ZiFu_Color, unsigned int Background_Color );			//指定位置(x,y)显示汉字(不能显示字符）	

//unsigned char disp_24x24GB1( unsigned int LayerIndex, unsigned int x, unsigned int y, unsigned char *str,  unsigned char lenth, unsigned int ZiFu_Color, unsigned int Background_Color );			//指定位置(x,y)显示汉字(不能显示字符）	

