#ifndef __VIDEO_H_
#define __VIDEO_H_
#endif




typedef struct 
{
	volatile unsigned int offaddr;										//列表的偏移地址									
	volatile unsigned int size;												//列表的大小（不包括LIST字段和数据大小字段）
}
_list;




typedef struct 
{
  volatile unsigned int cb;												//avih块的大小（不包括avih字段和数据大小字段）
  volatile unsigned int dwMicroSecPerFrame;				//显示每帧所需要的时间，以us为单位
  volatile unsigned int dwMaxBytesPerSec;					//AVI文件的最大数据率
  volatile unsigned int dwPaddingGranularity;			//数据填充的粒度
  volatile unsigned int dwFlags;									//AVI文件的全局标志，是否包含索引块
  volatile unsigned int dwTotalFrames;						//视频帧总数
  volatile unsigned int dwInitialFrames;					//交互格式指定初始帧数
  volatile unsigned int dwStreams;								//本文件包含的流个数
  volatile unsigned int dwSuggestedBufferSize;		//建议读取本文件的缓存大小
  volatile unsigned int dwWidth;									//视频图像的宽度（以像素为单位）
  volatile unsigned int dwHeight;									//视频图像的高度（以像素为单位）
}_avimainheader;






typedef struct 
{
  volatile unsigned int cb;												//strh块的大小（不包括strh字段和数据大小字段）
  unsigned char fccType[4];												//流的类型（auds：音频流，vids：视频流，mids：MIDI流，txts：文字流
  unsigned char fccHandler[4];										//指定编码器的类型
  volatile unsigned int dwFlags;									//是否允许这个流输出，调色板是否变化
  volatile unsigned short int wPriority;					//流的优先级（当有多个相同类型的流时，优先级最高的为默认流）
  volatile unsigned short int wLanguage;					//语言	
  volatile unsigned int dwInitialFrames;					//交互式指定初始帧数
  volatile unsigned int dwScale;									//这个流使用的时间尺度
  volatile unsigned int dwRate;										//dwRate/dwScale--视频：每秒帧数  音频：采样率
  volatile unsigned int dwStart;									//流的开始时间
  volatile unsigned int dwLength;									//流的长度
  volatile unsigned int dwSuggestedBufferSize;		//读取这个流数据建议使用的缓存大小
  volatile unsigned int dwQuality;								//流数据的质量指标(0~10,000)
  volatile unsigned int dwSampleSize;							//音频采样的大小
  struct
	{
		volatile unsigned short int left;
		volatile unsigned short int top;
		volatile unsigned short int right;
		volatile unsigned short int bottom;
  }_rcFrame;																			//指定这个流（视频流或者文字流）在视频主窗口显示的位置
}_avistreamheader;



typedef struct 
{
	volatile unsigned short int wFormatTag;					//音频格式
  volatile unsigned short int nChannels; 					//音频声道数
  volatile unsigned int nSamplesPerSec; 					//音频采样率
  volatile unsigned int nAvgBytesPerSec; 					//WAVE声音中每秒的数据量
  volatile unsigned short int nBlockAlign; 				//数据块的对齐标志
  volatile unsigned short int biSize; 						//此结构大小
}
_avi_audio_info;




typedef struct 
{
	volatile unsigned int offaddr;									//索引块的偏移地址									
	volatile unsigned int size;											//索引块的大小（不包括idx1字段和数据大小字段）
}
_avi_index;
	


typedef struct 
{
	volatile unsigned char ErrorFlag;								//错误标志
	volatile unsigned char VideoStreamFlag;					//视频流标志
	volatile unsigned char AudioStreamFlag;					//音频流标志
	volatile unsigned int FileSize;									//文件大小
	volatile unsigned short int ImageWith;					//画面宽度（像素）
	volatile unsigned short int ImageHeigh;					//画面高度（像素）
	volatile unsigned char VideoFrameRate;					//视频帧率
	volatile unsigned int TotalTime;								//文件总时长（单位ms)
	volatile unsigned int CurrenPlayTime;						//当前正在播放的时间节点（单位ms)
	volatile unsigned int StateFlag;								//状态标志（0：停止；1：播放；2：暂停；3：恢复 ）
}
_avi_player_info;




unsigned char AVI_Player( unsigned char *file, unsigned short int Xpos, unsigned short int Ypos, unsigned char vol, unsigned int time );				//播放AVI视频文件		



















