/*
 * Copyright 2010-2011 Foscam Intelligent Technologies Co., Ltd. 
 *
 * All rights reserved
 *
*/

#ifndef IPCAM_H
#define IPCAM_H

#define OK								0
#define ERROR_CONNECT					-1
#define ERROR_SOCKET					-2
#define ERROR_TIME_OUT					-3
#define ERROR_VERSION					-4
#define ERROR_CANCEL					-5
#define ERROR_CLOSED					-6
#define ERROR_UNKNOWN					-7
//#define ERROR_OPEN_FILE					-8
//#define ERROR_PARAM						-9
#define ERROR_THREAD					-10
#define ERROR_STATUS					-11
//#define ERROR_ID						-12
#define FAIL_INCORRECT_USER				1
#define FAIL_MAX_CONNS					2
#define FAIL_INCORRECT_VERSION			3
#define FAIL_INCORRECT_ID				4
#define FAIL_INCORRECT_PWD				5
#define FAIL_INCORRECT_PRI				6

#define WM_MONITOR_CONNECT_RESULT		WM_USER + 1
#define WM_MONITOR_DISCONNECTED			WM_USER + 2
#define WM_PLAYVIDEO_RESULT				WM_USER + 3
#define WM_VIDEO_STOPPED				WM_USER + 4
#define WM_VIDEO						WM_USER + 5
#define WM_PLAYAUDIO_RESULT				WM_USER + 6
#define WM_AUDIO_STOPPED				WM_USER + 7
#define WM_AUDIO						WM_USER + 8
#define WM_MONITOR_PARAMS_CHANGED		WM_USER + 9
#define WM_STATISTIC					WM_USER + 10
#define WM_ALARM2						WM_USER + 26
#define WM_OTHERDEVICES_PARAMS_CHANGED	WM_USER + 38
#define WM_STARTTALK_RESULT				WM_USER + 39
#define WM_TALK_STOPPED					WM_USER + 40
#define WM_TALK							WM_USER + 41

#define SENSOR_RESOLUTION				0			
#define SENSOR_BRIGHTNESS				1
#define SENSOR_CONTRAST					2
#define SENSOR_SATURATION				3
#define SENSOR_HUE						4
#define SENSOR_FLIP						5
#define SENSOR_RATE						6

#define PRI_MONITOR						0
#define PRI_CONTROL						1
#define PRI_MANAGE						2

typedef struct tagAudio
{
	unsigned int uiDataLen;
	unsigned char * pData;
	int iTime;
	unsigned int uiTick;
	unsigned int uiSeq;
	unsigned char ucFormat;
} AUDIO;

typedef struct tagImage
{
	unsigned int uiDataLen;
	char * pData;
	int iTime;
	unsigned int uiTick;
} IMAGE;

typedef struct tagOtherDeviceParams
{
	char msid[13];
	char alias[21];
	char host[65];
	unsigned short port;
	char user[13];
	char pwd[13];
	char mode;
} OTHER_DEVICE_PARAMS;

class CSend
{
public:
	CSend();
	~CSend();
	
	bool Init(short sCommand,const char * MoIP_FLAG);
	
	bool AddNext(char cData);
	bool AddNext(unsigned char ucData);
	bool AddNext(short sData);
	bool AddNext(unsigned short usData);
	bool AddNext(int iData);
	bool AddNext(unsigned int uiData);
	bool AddNext(const char * lpData,int iDataLen);
	bool AddNext(const unsigned char * ulpData,int iDataLen);
	bool AddNext(const void * pData,int iDataLen);
	bool EncodeCommand();
	
	short Send_t(SOCKET s);
	
	char * m_pBuffer;
	int m_iLength;
	
private:
	char * m_pPos;
	int m_iBufferLen;
};

class CRecv_t
{
public:
	CRecv_t();
	~CRecv_t();
	
	short Recv(SOCKET s, DWORD dwTimeOut);
	bool CheckCommand(short * pOpCode,const char * MoIP_FLAG);
	bool GetNext(char * pData);
	bool GetNext(unsigned char * pData);
	bool GetNext(short * pData);
	bool GetNext(unsigned short * pData);
	bool GetNext(int * pData);
	bool GetNext(unsigned int * pData);
	bool GetNext(char * pData,int iDataLen);
	bool GetNext(unsigned char * pData,int iDataLen);
	bool GetNext(void * pData,int iDataLen);
	
private:
	char * m_pPos;
	char * m_pData;
	char * m_pBuffer;
	char m_pRecvBuffer[1024];
	int m_iDataLength;
	int m_iBufferLength;
};

class CIpcam_sampleDlg;

class COpr 
{
public:
	COpr(CIpcam_sampleDlg * pDlg);
	~COpr();
	
	DWORD m_dwIP;
	unsigned short m_usPort;
	CString m_strUser;
	CString m_strPwd;
	
	short Connect(DWORD dwIP,unsigned short usPort,CString strUser,CString strPwd);
	short Disconnect();

	short PlayVideo();
	short StopVideo();
	short PlayAudio();
	short StopAudio();
	short StartTalk();
	short StopTalk();
	short SetMaxRate(short sMaxRate);
	short SetBufferTime(short sBufferTime);
	short DecoderControl(unsigned short usCode);
	short SensorControl(unsigned short usCode,unsigned short usValue);
	short CommWrite(unsigned char * pData,unsigned char ucLen,unsigned int uiBaud);
	void AddTalkAudio(AUDIO * pAudio);

private:
	HANDLE m_hThread;
	DWORD m_dwThreadID;
	static DWORD WINAPI ThreadProc(LPVOID lpParameter);
	
	CIpcam_sampleDlg * m_pDlg;

	void OnConnectResult(short sResult,short sPri);
	void OnDisconnected(short sReason);
	void OnPlayVideoResult(short sResult);
	void OnVideoStopped(short sReason);
	void OnPlayAudioResult(short sResult);
	void OnAudioStopped(short sReason);
	void OnMonitorParamsChanged(unsigned short usCode,unsigned short usValue);
	void OnAlarm2(short sAlarm,short sMotionLeft,short sMotionTop,short sMotionRight,short sMotionBottom);
	void OnStatistic(long lVideoBandwidth,short sVideoFrames,long lAudioBandwidth,short sAudioSamples);
	void OnImage(IMAGE * pImage,bool bPlay);
	void OnAudio(AUDIO * pAudio,bool bPlay);
	void OnOtherDevicesParamsChanged(OTHER_DEVICE_PARAMS * pOtherDevicesParams);
	void OnStartTalkResult(short sResult);
	void OnTalkStopped(short sReason);
	
	void FreeImage(IMAGE * pImage);
	void FreeAudio(AUDIO * pAudio);
};

extern long g_lImagesWaitShow;

void adpcm_encode(unsigned char * raw, int len, unsigned char * encoded, int * pre_sample, int * index);
void adpcm_decode(unsigned char * raw, int len, unsigned char * decoded, int * pre_sample, int * index);

#endif

