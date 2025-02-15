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
}_SongInfo;										//MP3������Ϣ����������ר�������֣�������, �ļ���С������ʱ��, ʱ�����, �ļ�����, ����״̬ ��




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
	 * ������Ϣ(Layer III), in Bytes
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
	*  ����һ֡��ʱ�䣨��λ: ms��
	 */
	volatile double DecodeFrameTime;
	
} _MPEGAudioFrameInfo;









unsigned char AudioPlayer_Basic_Init( void );																								//��Ƶ��������ʼ������

unsigned char mp3_player_Init( char *mp3_file );																						//MP3�������ĳ�ʼ������

unsigned char MP3Play( unsigned short int time );																						//����MP3

unsigned char wav_player_Init( char *wav_file );																						//WAV�������ĳ�ʼ������

unsigned char WAVPlay( unsigned short int time );																						//����WAV

unsigned char AudioRecord( char *wav_file, int AudioFreq );																	//��ʼ¼��

unsigned char *strstr2( unsigned char *str1, unsigned char *str2, unsigned short int num );	//����һ���ַ�������һ���ַ����ĵ�һ�γ���


