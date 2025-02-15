#ifndef __VIDEO_H_
#define __VIDEO_H_
#endif




typedef struct 
{
	volatile unsigned int offaddr;										//�б��ƫ�Ƶ�ַ									
	volatile unsigned int size;												//�б�Ĵ�С��������LIST�ֶκ����ݴ�С�ֶΣ�
}
_list;




typedef struct 
{
  volatile unsigned int cb;												//avih��Ĵ�С��������avih�ֶκ����ݴ�С�ֶΣ�
  volatile unsigned int dwMicroSecPerFrame;				//��ʾÿ֡����Ҫ��ʱ�䣬��usΪ��λ
  volatile unsigned int dwMaxBytesPerSec;					//AVI�ļ������������
  volatile unsigned int dwPaddingGranularity;			//������������
  volatile unsigned int dwFlags;									//AVI�ļ���ȫ�ֱ�־���Ƿ����������
  volatile unsigned int dwTotalFrames;						//��Ƶ֡����
  volatile unsigned int dwInitialFrames;					//������ʽָ����ʼ֡��
  volatile unsigned int dwStreams;								//���ļ�������������
  volatile unsigned int dwSuggestedBufferSize;		//�����ȡ���ļ��Ļ����С
  volatile unsigned int dwWidth;									//��Ƶͼ��Ŀ�ȣ�������Ϊ��λ��
  volatile unsigned int dwHeight;									//��Ƶͼ��ĸ߶ȣ�������Ϊ��λ��
}_avimainheader;






typedef struct 
{
  volatile unsigned int cb;												//strh��Ĵ�С��������strh�ֶκ����ݴ�С�ֶΣ�
  unsigned char fccType[4];												//�������ͣ�auds����Ƶ����vids����Ƶ����mids��MIDI����txts��������
  unsigned char fccHandler[4];										//ָ��������������
  volatile unsigned int dwFlags;									//�Ƿ�����������������ɫ���Ƿ�仯
  volatile unsigned short int wPriority;					//�������ȼ������ж����ͬ���͵���ʱ�����ȼ���ߵ�ΪĬ������
  volatile unsigned short int wLanguage;					//����	
  volatile unsigned int dwInitialFrames;					//����ʽָ����ʼ֡��
  volatile unsigned int dwScale;									//�����ʹ�õ�ʱ��߶�
  volatile unsigned int dwRate;										//dwRate/dwScale--��Ƶ��ÿ��֡��  ��Ƶ��������
  volatile unsigned int dwStart;									//���Ŀ�ʼʱ��
  volatile unsigned int dwLength;									//���ĳ���
  volatile unsigned int dwSuggestedBufferSize;		//��ȡ��������ݽ���ʹ�õĻ����С
  volatile unsigned int dwQuality;								//�����ݵ�����ָ��(0~10,000)
  volatile unsigned int dwSampleSize;							//��Ƶ�����Ĵ�С
  struct
	{
		volatile unsigned short int left;
		volatile unsigned short int top;
		volatile unsigned short int right;
		volatile unsigned short int bottom;
  }_rcFrame;																			//ָ�����������Ƶ������������������Ƶ��������ʾ��λ��
}_avistreamheader;



typedef struct 
{
	volatile unsigned short int wFormatTag;					//��Ƶ��ʽ
  volatile unsigned short int nChannels; 					//��Ƶ������
  volatile unsigned int nSamplesPerSec; 					//��Ƶ������
  volatile unsigned int nAvgBytesPerSec; 					//WAVE������ÿ���������
  volatile unsigned short int nBlockAlign; 				//���ݿ�Ķ����־
  volatile unsigned short int biSize; 						//�˽ṹ��С
}
_avi_audio_info;




typedef struct 
{
	volatile unsigned int offaddr;									//�������ƫ�Ƶ�ַ									
	volatile unsigned int size;											//������Ĵ�С��������idx1�ֶκ����ݴ�С�ֶΣ�
}
_avi_index;
	


typedef struct 
{
	volatile unsigned char ErrorFlag;								//�����־
	volatile unsigned char VideoStreamFlag;					//��Ƶ����־
	volatile unsigned char AudioStreamFlag;					//��Ƶ����־
	volatile unsigned int FileSize;									//�ļ���С
	volatile unsigned short int ImageWith;					//�����ȣ����أ�
	volatile unsigned short int ImageHeigh;					//����߶ȣ����أ�
	volatile unsigned char VideoFrameRate;					//��Ƶ֡��
	volatile unsigned int TotalTime;								//�ļ���ʱ������λms)
	volatile unsigned int CurrenPlayTime;						//��ǰ���ڲ��ŵ�ʱ��ڵ㣨��λms)
	volatile unsigned int StateFlag;								//״̬��־��0��ֹͣ��1�����ţ�2����ͣ��3���ָ� ��
}
_avi_player_info;




unsigned char AVI_Player( unsigned char *file, unsigned short int Xpos, unsigned short int Ypos, unsigned char vol, unsigned int time );				//����AVI��Ƶ�ļ�		



















