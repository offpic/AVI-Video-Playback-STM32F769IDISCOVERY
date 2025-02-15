#ifndef __AUDIOPLAYER_H
#define __AUDIOPLAYER_H
#endif




typedef enum
{
	OK = 0,
	Pause = 1,
	Resume = 2,
	Stop = 3
}_SongState;





typedef	struct
{
	unsigned char TIT2[30];
	unsigned char	TALB[30];
	unsigned char	TPE1[20];
	volatile unsigned int Bitrate;
	volatile unsigned int FileSize;
	volatile unsigned int time;
	volatile unsigned short int time_count;
	unsigned char FileName[30];
	_SongState state;
}_SongInfo;										//MP3歌曲信息（歌曲名，专辑，歌手，比特率, 文件大小，播放时间, 时间计数, 文件名字, 播放状态 ）




typedef	struct
{
	volatile unsigned int Bitrate;
	volatile unsigned int FileSize;
	volatile unsigned int time;
	volatile unsigned int time_count;
	unsigned char FileName[30];
	volatile int Samplerate;
	volatile _SongState state;
}_RecordInfo;	




typedef struct 
{
	// MPEG Audio Frame Header /////////////////////////////////////////////////
	
	/**
	 * MPEG-1.0: 0
	 * MPEG-2.0: 1
	 * MPEG-2.5: 2
	 * invalid : other
	 */
	volatile char mMPEGVersion;
	
	/**
	 * Layer I  : 0
	 * Layer II : 1
	 * Layer III: 2
	 * invalid  : other
	 */
	volatile char mLayer;
	
	/**
	 * Protection off: 0
	 * Protection on : 1
	 */
	volatile char mProtection;
	
	/**
	 * in kbits/s
	 */
	volatile int mBitrate;
	
	/**
	 * in Hz
	 */
	volatile int mSamplerate;
	
	/**
	 * in Bytes
	 */
	volatile char mPaddingSize;
	
	
	/**
	 * Channel mode
	 *
	 * Stereo - 0
	 * Joint Stereo (Stereo) - 1
	 * Dual channel (Two mono channels) - 2
	 * Single channel (Mono) - 3
	 */
	volatile char mChannelMode;
	
	/**
	 * Mode extension, Only used in Joint Stereo Channel mode.
	 * not process
	 */
	volatile char mExtensionMode;
	
	/**
	 * Copyright bit, only informative
	 */
	volatile char mCopyrightBit;
	
	/**
	 * Original bit, only informative
	 */
	volatile char mOriginalBit;
	
	/**
     * Emphasis:
     * The emphasis indication is here to tell the decoder that the file must be 
     * de-emphasized, that means the decoder must 're-equalize' the sound after 
     * a Dolby-like noise suppression. It is rarely used.
     * 
     * 0 - none
     * 1 - 50/15 ms
     * 2 - reserved (invalid)
     * 3 - CCIT J.17
     */
   volatile char mEmphasis;

	/**
	 * 边沿信息(Layer III), in Bytes
	 */
	volatile char mSideInfoSize;
	
	
	/**
	 * Samples per frame
	 */
	volatile int mSamplesPerFrame;
	
	/**
	 * 0 - CBR
	 * 1 - CBR(INFO)
	 * 2 - VBR(XING)
	 * 3 - VBR(VBRI)
	 */
	volatile char mBitrateType;
	
	/**
	 * 2 Bytes
	 */
	volatile unsigned short int mCRCValue;
	
	
	// XING, INFO or VBRI Header /////////////////////////////////////////////////////

	/**
	 * mTotalFrames
	 */
	volatile unsigned int mTotalFrames;
	
	/**
	 * mTotalBytes
	 */
	volatile unsigned int mTotalBytes;
	
	/**
	 * Quality indicator
	 * 
	 * From 0 - worst quality to 100 - best quality
	 */
	volatile short int mQuality;
	
	/**
	 * Frame size
	 */
	volatile int mFrameSize;
	
	/**
	 * First Frame Address
	 */
	volatile unsigned int mFirstFrameAddr;
	
	/**
	*  播放一帧的时间（单位: ms）
	 */
	volatile double DecodeFrameTime;
	
} _MPEGAudioFrameInfo;









unsigned char AudioPlayer_Basic_Init( void );																								//音频播放器初始化函数

unsigned char mp3_player_Init( char *mp3_file );																						//MP3播放器的初始化函数

unsigned char MP3Play( unsigned short int time );																						//播放MP3

unsigned char wav_player_Init( char *wav_file );																						//WAV播放器的初始化函数

unsigned char WAVPlay( unsigned short int time );																						//播放WAV

unsigned char AudioRecord( char *wav_file, int AudioFreq );																	//开始录音

unsigned char *strstr2( unsigned char *str1, unsigned char *str2, unsigned short int num );	//搜索一个字符串在另一个字符串的第一次出现


