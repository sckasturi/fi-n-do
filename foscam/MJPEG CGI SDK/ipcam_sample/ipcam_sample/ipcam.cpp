/*
 * Copyright 2010-2011 Foscam Intelligent Technologies Co., Ltd. 
 *
 * All rights reserved
 *
*/

#include "stdafx.h"
#include "ipcam.h"
#include "ipcam_sample.h"
#include "ipcam_sampledlg.h"

#define HEAD_LENGTH				23
#define CAMERA_ID_LENGTH		12

#define WM_PLAY_VIDEO				WM_USER + 1
#define WM_STOP_VIDEO				WM_USER + 2
#define WM_RATE						WM_USER + 3
#define WM_BUFFER					WM_USER + 4
#define WM_SENSOR					WM_USER + 5
#define WM_DECODER					WM_USER + 6
#define WM_PLAY_AUDIO				WM_USER + 7
#define WM_STOP_AUDIO				WM_USER + 8
#define WM_COMM						WM_USER + 9
#define WM_START_TALK				WM_USER + 15
#define WM_STOP_TALK				WM_USER + 16


typedef struct tagAVSample
{
	int iSampleType;
	AUDIO * pAudio;
	IMAGE * pImage;
} AV_SAMPLE;


const char * MoIP_OPR_FLAG = "MO_O";
const char * MoIP_AV_FLAG = "MO_V";

const unsigned int MOIP_OPR_VER = 0x01000000;

const unsigned short OPR_LOGIN_REQ  = 0;
const unsigned short OPR_LOGIN_RESP = 1;
const unsigned short OPR_VERIFY_REQ = 2;
const unsigned short OPR_VERIFY_RESP = 3;
const unsigned short OPR_VIDEO_START_REQ = 4;
const unsigned short OPR_VIDEO_START_RESP = 5;
const unsigned short OPR_VIDEO_END = 6;
const unsigned short OPR_VIDEO_RATE = 7;
const unsigned short OPR_AUDIO_START_REQ = 8;
const unsigned short OPR_AUDIO_START_RESP = 9;
const unsigned short OPR_AUDIO_END = 10;
const unsigned short OPR_SPEAK_START_NOTIFY = 11;
const unsigned short OPR_SPEAK_START_RESP = 12;
const unsigned short OPR_SPEAK_END = 13;
const unsigned short OPR_DECODER_CONTROL_REQ = 14;
const unsigned short OPR_COMM_DATA = 15;
const unsigned short OPR_PARAMS_FETCH_REQ = 16;
const unsigned short OPR_PARAMS_FETCH_RESP = 17;
const unsigned short OPR_PARAMS_CHANGED_NOTIFY = 18;
const unsigned short OPR_PARAMS_SET_REQ = 19;
const unsigned short OPR_ALARM2_NOTIFY = 25;
const unsigned short OPR_OTHERDEVICES_PARAMS_NOTIFY = 28;
const unsigned short OPR_BUFFER = 29;
const unsigned short OPR_KEEP_ALIVE = 255;

const unsigned short AV_LOGIN_REQ = 0;
const unsigned short VIDEO_DATA = 1;
const unsigned short AUDIO_DATA = 2;
const unsigned short TALK_DATA = 3;

const unsigned char UNICAST_TCP = 2;

long g_lImagesWaitShow = 0;

CSend::CSend()
{
	m_pPos = m_pBuffer = NULL;
	m_iLength = m_iBufferLen = 0;
}

CSend::~CSend()
{
	if (m_pBuffer != NULL)
		delete []m_pBuffer;
	m_pPos = m_pBuffer = NULL;
	m_iLength = m_iBufferLen = 0;
}
	
bool CSend::Init(short sCommand,const char * MoIP_FLAG)
{
	if (m_pBuffer != NULL)
		delete []m_pBuffer;
	m_pPos = m_pBuffer = NULL;
	m_iLength = m_iBufferLen = 0;
	
	m_pBuffer = new char[100];
	if (m_pBuffer == NULL)
		return false;
	
	strncpy(m_pBuffer,MoIP_FLAG,4);
	memcpy((void *)(m_pBuffer + 4),(void *)&sCommand,2);
	memset((void *)(m_pBuffer + 6),0,17);
	
	m_iBufferLen = 100;
	m_iLength = HEAD_LENGTH;
	m_pPos = m_pBuffer + HEAD_LENGTH;

	return true;
}
	
bool CSend::AddNext(char cData)
{
	return AddNext((void *)&cData,sizeof(char));
}

bool CSend::AddNext(unsigned char ucData)
{
	return AddNext((void *)&ucData,sizeof(unsigned char));
}

bool CSend::AddNext(short sData)
{
	return AddNext((void *)&sData,sizeof(short));
}

bool CSend::AddNext(unsigned short usData)
{
	return AddNext((void *)&usData,sizeof(unsigned short));
}

bool CSend::AddNext(int iData)
{
	return AddNext((void *)&iData,sizeof(int));
}

bool CSend::AddNext(unsigned int uiData)
{
	return AddNext((void *)&uiData,sizeof(unsigned int));
}

bool CSend::AddNext(const char * lpData,int iDataLen)
{
	return AddNext((void *)lpData,iDataLen);
}

bool CSend::AddNext(const unsigned char * ulpData,int iDataLen)
{
	return AddNext((void *)ulpData,iDataLen);
}

bool CSend::AddNext(const void * pData,int iDataLen)
{
	if (iDataLen == 0)
		return true;
	if (pData == NULL)
		return false;
	
	if (iDataLen + m_iLength > m_iBufferLen)
	{
		int iBufferLen = m_iBufferLen;
		while (iDataLen + m_iLength > iBufferLen)
			iBufferLen = iBufferLen + 100;

		char * pBuffer = new char[iBufferLen];
		if (pBuffer == NULL)
			return false;
		
		memcpy((void *)pBuffer,(void *)m_pBuffer,m_iLength);
		delete []m_pBuffer;

		m_pBuffer = pBuffer;
		m_pPos = pBuffer + m_iLength;
		m_iBufferLen = iBufferLen;
	}
	
	memcpy((void *)m_pPos,pData,iDataLen);
	m_pPos = m_pPos + iDataLen;
	m_iLength = m_iLength + iDataLen;
	int iTotalDataLen = m_iLength - HEAD_LENGTH;
	memcpy((void *)(m_pBuffer + 19),(void *)&iTotalDataLen,4);
	
	return true;
}

bool CSend::EncodeCommand()
{
	memcpy((void *)(m_pBuffer + 15),(void *)(m_pBuffer + 19),4);
	return true;
}

short CSend::Send_t(SOCKET s)
{
	int iResult;
	int len = m_iLength;
	int offset = 0;
	do 
	{
		iResult = send(s,m_pBuffer + offset,len,0);	
		if (iResult == SOCKET_ERROR)
			return ERROR_SOCKET;
		
		offset = offset + iResult;
		len = len - iResult;
	} while(len > 0);
	
	return OK;
}

CRecv_t::CRecv_t()
{
	m_pPos = m_pBuffer = m_pData = NULL;
	m_iDataLength = m_iBufferLength = 0;
}

CRecv_t::~CRecv_t()
{
	if (m_pBuffer)
		delete [] m_pBuffer;
	if (m_pData)
		delete [] m_pData;
}

short CRecv_t::Recv(SOCKET s, DWORD dwTimeOut)
{
	timeval timeout;
	timeout.tv_sec = dwTimeOut / 1000;
	timeout.tv_usec = dwTimeOut % 1000 * 1000;
	
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(s,&fds);
	DWORD dwErr;
	int iResult = select(0,&fds,NULL,NULL,&timeout);
	if (iResult != 1)
	{
		if (iResult == 0)
			return ERROR_TIME_OUT;
		else
		{
			dwErr = WSAGetLastError();
			if ((dwErr == WSAECONNABORTED) || (dwErr == WSAECONNRESET))
				return ERROR_CLOSED;
			return ERROR_SOCKET;
		}
	}
	
	int iRecvLen = recv(s,m_pRecvBuffer,1024,0);
	if (iRecvLen < 0)
	{
		dwErr = WSAGetLastError();
		if ((dwErr == WSAECONNABORTED) || (dwErr == WSAECONNRESET))
			return ERROR_CLOSED;
		return ERROR_SOCKET;
	}
	if (iRecvLen == 0)
		return ERROR_CLOSED;

	char * pBuffer = new char[m_iBufferLength + iRecvLen];
	if (m_pBuffer)
		memcpy(pBuffer,m_pBuffer,m_iBufferLength);
	memcpy(pBuffer + m_iBufferLength,m_pRecvBuffer,iRecvLen);
	if (m_pBuffer)
		delete [] m_pBuffer;
	m_pBuffer = pBuffer;
	m_iBufferLength = m_iBufferLength + iRecvLen;
	
	return OK;
}

bool CRecv_t::CheckCommand(short * pOpCode,const char * MoIP_FLAG)
{
	int i;
	char * pBuffer;
	int iBufferLen;
	int iDataLen;
	
	for (i = 0;i < m_iBufferLength - 3;i ++)
	{
		if (0 == strncmp(m_pBuffer + i,MoIP_FLAG,4))
			break;
	}
	if (i)
	{
		iBufferLen = m_iBufferLength - i;
		pBuffer = new char [iBufferLen];
		memcpy(pBuffer,m_pBuffer + i,iBufferLen);
		delete [] m_pBuffer;
		m_pBuffer = pBuffer;
		m_iBufferLength = iBufferLen;
	}
	
	if (m_iBufferLength < HEAD_LENGTH)
	{
		return false;
	}
	
	unsigned char ucDescryption;
	unsigned char key[8];
	
	memcpy(pOpCode,m_pBuffer + 4,sizeof(unsigned short));
	memcpy(&ucDescryption,m_pBuffer + 6,sizeof(unsigned char));
	memcpy(key,m_pBuffer + 7,8);
	memcpy(&iDataLen,m_pBuffer + 15,sizeof(unsigned int));
	
	if (m_iBufferLength < HEAD_LENGTH + iDataLen)
	{
		return false;
	}
	
	if (m_pData)
		delete [] m_pData;
	m_iDataLength = iDataLen;
	
	if (m_iDataLength)
	{
		m_pData = new char [m_iDataLength];
		memcpy(m_pData,m_pBuffer + HEAD_LENGTH,m_iDataLength);
	}
	else
	{
		m_pData = NULL;
	}
	
	m_pPos = m_pData;
	
	pBuffer = m_pBuffer;
	m_iBufferLength = m_iBufferLength - HEAD_LENGTH - m_iDataLength;
	if (m_iBufferLength)
	{
		m_pBuffer = new char[m_iBufferLength];
		memcpy(m_pBuffer,pBuffer + HEAD_LENGTH + m_iDataLength,m_iBufferLength);
	}
	else
	{
		m_pBuffer = NULL;
	}
	
	delete [] pBuffer;
	
	return true;
}

bool CRecv_t::GetNext(char * pData)
{
	return GetNext((void *)pData,sizeof(char));
}

bool CRecv_t::GetNext(unsigned char * pData)
{
	return GetNext((void *)pData,sizeof(unsigned char));
}

bool CRecv_t::GetNext(short * pData)
{
	return GetNext((void *)pData,sizeof(short));
}

bool CRecv_t::GetNext(unsigned short * pData)
{
	return GetNext((void *)pData,sizeof(unsigned short));
}

bool CRecv_t::GetNext(int * pData)
{
	return GetNext((void *)pData,sizeof(int));
}

bool CRecv_t::GetNext(unsigned int * pData)
{
	return GetNext((void *)pData,sizeof(unsigned int));
}

bool CRecv_t::GetNext(char * pData,int iDataLen)
{
	return GetNext((void *)pData,iDataLen);
}

bool CRecv_t::GetNext(unsigned char * pData,int iDataLen)
{
	return GetNext((void *)pData,iDataLen);
}

bool CRecv_t::GetNext(void * pData,int iDataLen)
{
	if (m_pPos == NULL)
		return false;
	if (m_pPos + iDataLen > m_pData + m_iDataLength)
		return false;            

	memcpy(pData,(void *)m_pPos,iDataLen);
	
	m_pPos = m_pPos + iDataLen;

	return true;
}

COpr::COpr(CIpcam_sampleDlg * pDlg)
{
	m_hThread = NULL;
	m_pDlg = pDlg;
}

COpr::~COpr()
{
	if (m_hThread)
		Disconnect();
}
	
short COpr::Connect(DWORD dwIP,unsigned short usPort,CString strUser,CString strPwd)
{
	if (m_hThread)
	{
		DWORD dwExitCode;
		GetExitCodeThread(m_hThread,&dwExitCode);
		if (dwExitCode == STILL_ACTIVE)
			return ERROR_STATUS;
		
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}

	m_dwIP = dwIP;
	m_usPort = usPort;
	m_strUser = strUser;
	m_strPwd = strPwd;

	if (NULL == (m_hThread = CreateThread(NULL,
										  0,
										  (LPTHREAD_START_ROUTINE)ThreadProc,
										  (LPVOID)this,
										  FALSE,
										  &m_dwThreadID)))
	{
		return ERROR_THREAD;
	}
	else
	{
		SetThreadPriority(m_hThread,THREAD_PRIORITY_HIGHEST);
		return OK;
	}
}

short COpr::Disconnect()
{
	DWORD dwExitCode;

	if (m_hThread)
	{
		while (1)
		{
			PostThreadMessage(m_dwThreadID,WM_QUIT,0,0);
			Sleep(10);
			GetExitCodeThread(m_hThread,&dwExitCode);
			if (dwExitCode != STILL_ACTIVE)
				break;
		}
		
		CloseHandle(m_hThread);
		m_hThread = NULL;
		return OK;
	}
	else
	{
		return ERROR_STATUS;
	}
}

short COpr::PlayVideo()
{
	if (! m_hThread)
		return ERROR_STATUS;
	
	if (! PostThreadMessage(m_dwThreadID,WM_PLAY_VIDEO,0,0))
		return ERROR_THREAD;
	else
		return OK;
}
	
short COpr::StopVideo()
{
	if (! m_hThread)
		return ERROR_STATUS;
	
	if (! PostThreadMessage(m_dwThreadID,WM_STOP_VIDEO,0,0))
		return ERROR_THREAD;
	else
		return OK;
}
	
short COpr::PlayAudio()
{
	if (! m_hThread)
		return ERROR_STATUS;
	
	if (! PostThreadMessage(m_dwThreadID,WM_PLAY_AUDIO,0,0))
		return ERROR_THREAD;
	else
		return OK;
}

short COpr::StopAudio()
{
	if (! m_hThread)
		return ERROR_STATUS;
	
	if (! PostThreadMessage(m_dwThreadID,WM_STOP_AUDIO,0,0))
		return ERROR_THREAD;
	else
		return OK;
}

short COpr::StartTalk()
{
	if (! m_hThread)
		return ERROR_STATUS;
	
	if (! PostThreadMessage(m_dwThreadID,WM_START_TALK,0,0))
		return ERROR_THREAD;
	else
		return OK;
}

short COpr::StopTalk()
{
	if (! m_hThread)
		return ERROR_STATUS;
	
	if (! PostThreadMessage(m_dwThreadID,WM_STOP_TALK,0,0))
		return ERROR_THREAD;
	else
		return OK;
}

short COpr::SetMaxRate(short sMaxRate)
{
	if (! m_hThread)
		return ERROR_STATUS;
	
	if (! PostThreadMessage(m_dwThreadID,WM_RATE,sMaxRate,0))
		return ERROR_THREAD;
	else
		return OK;
}

short COpr::SetBufferTime(short sBufferTime)
{
	if (! m_hThread)
		return ERROR_STATUS;
	
	if (! PostThreadMessage(m_dwThreadID,WM_BUFFER,sBufferTime,0))
		return ERROR_THREAD;
	else
		return OK;
}

short COpr::SensorControl(unsigned short usCode,unsigned short usValue)
{
	if (! m_hThread)
		return ERROR_STATUS;
	
	if (! PostThreadMessage(m_dwThreadID,WM_SENSOR,usCode,usValue))
		return ERROR_THREAD;
	else
		return OK;
}

void COpr::AddTalkAudio(AUDIO * pAudio)
{
	if (! m_hThread)
		FreeAudio(pAudio);
	if (! PostThreadMessage(m_dwThreadID,WM_TALK,(WPARAM)pAudio,0))
		FreeAudio(pAudio);
}

short COpr::DecoderControl(unsigned short usCode)
{
	if (! m_hThread)
		return ERROR_STATUS;
	
	if (! PostThreadMessage(m_dwThreadID,WM_DECODER,usCode,0))
		return ERROR_THREAD;
	else
		return OK;
}

short COpr::CommWrite(unsigned char * pData,unsigned char ucLen,unsigned int uiBaud)
{
	if (! m_hThread)
		return ERROR_STATUS;
	
	unsigned char * p = new unsigned char[ucLen];
	memcpy(p,pData,ucLen);
	unsigned int * p2 = new unsigned int[2];
	* p2 = ucLen;
	* (p2 + 1) = uiBaud;
	if (! PostThreadMessage(m_dwThreadID,WM_COMM,(WPARAM)p2,(LPARAM)p))
	{
		delete [] p;
		delete [] p2;
		return ERROR_THREAD;
	}
	else
		return OK;
}

void COpr::OnConnectResult(short sResult,short sPri)
{
	m_pDlg->PostMessage(WM_MONITOR_CONNECT_RESULT,sResult,sPri);
}

void COpr::OnDisconnected(short sReason)
{
	m_pDlg->PostMessage(WM_MONITOR_DISCONNECTED,sReason);
}

void COpr::OnPlayVideoResult(short sResult)
{
	InterlockedExchange(&g_lImagesWaitShow,0);
	m_pDlg->PostMessage(WM_PLAYVIDEO_RESULT,sResult);
}



void COpr::OnVideoStopped(short sReason)
{
	m_pDlg->PostMessage(WM_VIDEO_STOPPED,sReason);
}

void COpr::OnPlayAudioResult(short sResult)
{
	m_pDlg->PostMessage(WM_PLAYAUDIO_RESULT,sResult);
}

void COpr::OnAudioStopped(short sReason)
{
	m_pDlg->PostMessage(WM_AUDIO_STOPPED,sReason);
}

void COpr::OnStartTalkResult(short sResult)
{
	m_pDlg->PostMessage(WM_STARTTALK_RESULT,sResult);
}

void COpr::OnTalkStopped(short sReason)
{
	m_pDlg->PostMessage(WM_TALK_STOPPED,sReason);
}

void COpr::OnMonitorParamsChanged(unsigned short usCode,unsigned short usValue)
{
	m_pDlg->PostMessage(WM_MONITOR_PARAMS_CHANGED,usCode,usValue);	
}

void COpr::OnAlarm2(short sAlarm,short sMotionLeft,short sMotionTop,short sMotionRight,short sMotionBottom)
{
	m_pDlg->PostMessage(WM_ALARM2,sAlarm,MAKELPARAM(MAKEWORD(sMotionLeft,sMotionTop),MAKEWORD(sMotionRight,sMotionBottom)));
}

void COpr::OnStatistic(long lVideoBandwidth,short sVideoFrames,long lAudioBandwidth,short sAudioSamples)
{
	m_pDlg->PostMessage(WM_STATISTIC,lVideoBandwidth,MAKELPARAM(lAudioBandwidth,MAKEWORD(sVideoFrames,sAudioSamples)));	
}

void COpr::OnOtherDevicesParamsChanged(OTHER_DEVICE_PARAMS * pOtherDevicesParams)
{
	if (! m_pDlg->PostMessage(WM_OTHERDEVICES_PARAMS_CHANGED,(WPARAM)pOtherDevicesParams))
		delete pOtherDevicesParams;
}

void COpr::OnImage(IMAGE * pImage,bool bPlay)
{
	if (m_pDlg->PostMessage(WM_VIDEO,(WPARAM)pImage,(LPARAM)bPlay))
		InterlockedIncrement(&g_lImagesWaitShow);
	else
	{
		delete [] pImage->pData;
		delete pImage;
	}
}

void COpr::OnAudio(AUDIO * pAudio,bool bPlay)
{
	if (! m_pDlg->PostMessage(WM_AUDIO,(WPARAM)pAudio,(LPARAM)bPlay))
	{
		delete [] pAudio->pData;
		delete pAudio;
	}
}

void COpr::FreeImage(IMAGE * pImage)
{
	if (pImage != NULL)
	{
		if (pImage->pData != NULL)
			delete [] pImage->pData;
		delete pImage;
	}
}

void COpr::FreeAudio(AUDIO * pAudio)
{
	if (pAudio != NULL)
	{
		if (pAudio->pData != NULL)
			delete [] pAudio->pData;
		delete pAudio;
	}
}

DWORD COpr::ThreadProc(LPVOID lpParameter)
{
	COpr * p = (COpr *)lpParameter;
	unsigned char ucProtocol = UNICAST_TCP;
	
	SOCKET s_opr = INVALID_SOCKET,s_av = INVALID_SOCKET;
	bool bConnected = false,bVideoTryPlaying = false,bVideoPlaying = false,bAudioTryPlaying = false,bAudioPlaying = false,bAVLogon = false;
	bool bTalking = false, bTryTalking = false;
	short sResult,iBufferTime = 1,sOpCode,sMotionLeft,sMotionTop,sMotionRight,sMotionBottom;
	int iMaxRate = 0;
	struct sockaddr_in addr;
	CSend Send_s;
	CRecv_t Recv_opr,Recv_av_t;
	DWORD tRecvTime,tSendTime,tAVRecvTime,now,interval;
	int iFramesRecvedPerS = 0,iRateAdjusted = 0,iVideoBandWidth = 0,iAudioSamples = 0,iAudioBandWidth = 0;
	unsigned int opt,dwOprVersion,dwModel,dwCameraVersion,dwAVConnID,dwPre1Second;//,dwPreVideoTick,dwVideoTick,dwPreAudioTick,dwCameraTime,dwImageLen,dwPacketLen;
	unsigned char ucPri,ucSet,ucValue,ucAlarm,ucCodedFormat;//,ucPackets,ucPacketNum;
	IMAGE * pImage = NULL;
	AUDIO * pAudio = NULL;
	std::multimap<unsigned int,AV_SAMPLE> AVBuffer;
	std::multimap<unsigned int,AV_SAMPLE>::iterator pAVFirst;
	AV_SAMPLE av_sample;
	
	OTHER_DEVICE_PARAMS * pOtherDevicesParams;

	char pMsid[13];
	MSG msg;
	
	DWORD dwFirstCameraTick = 0;
	DWORD dwFirstComputerTick = 0;
	DWORD dwCameraTick;
	unsigned char * audio_raw_data;
	int decode_index = 0;
	int decode_sample = 0;

#define TALK_BUFFER_SIZE		2500

	fd_set fdr,fdw;
	struct timeval timeout;
	char talk_buffer[TALK_BUFFER_SIZE];
	int talk_datalen = 0;
	int re;
	int i;

	if (INVALID_SOCKET == (s_opr = socket(AF_INET,SOCK_STREAM,0)))
	{
		sResult = ERROR_SOCKET;
		goto quit;
	}
	
	addr.sin_family = AF_INET;
	addr.sin_addr.S_un.S_addr = p->m_dwIP;
	addr.sin_port = p->m_usPort;

	if (SOCKET_ERROR == connect(s_opr,(struct sockaddr *)&addr,sizeof(addr)))
	{
		sResult = ERROR_CONNECT;
		goto quit;
	}
	
	Send_s.Init(OPR_LOGIN_REQ,MoIP_OPR_FLAG);
	Send_s.EncodeCommand();
	Send_s.Send_t(s_opr);
	
	tRecvTime = tSendTime = GetTickCount();
	
	while (1)
	{
		now = GetTickCount();
		if (now < tRecvTime)
			interval = now;
		else
			interval = now - tRecvTime;
		if (interval > 120000)
		{
			sResult = ERROR_TIME_OUT;
			goto quit;
		}
		
		if (bConnected)
		{
			if (now < tSendTime)
				interval = now;
			else
				interval = now - tSendTime;
			
			if (interval >= 60000)
			{
				Send_s.Init(OPR_KEEP_ALIVE,MoIP_OPR_FLAG);
				Send_s.EncodeCommand();
				Send_s.Send_t(s_opr);
				tSendTime = now;
			}
		}
		
		if (s_av != INVALID_SOCKET)
		{
			now = GetTickCount();
			for (pAVFirst = AVBuffer.begin();pAVFirst != AVBuffer.end();)
			{
				if ((now - pAVFirst->first) >= iBufferTime * 1000)
				{
					if (pAVFirst->second.iSampleType == 0)
					{
						pImage = pAVFirst->second.pImage;
						p->OnImage(pImage,true);
					}
					else
					{
						pAudio = pAVFirst->second.pAudio;
						p->OnAudio(pAudio,true);
					}
					
					AVBuffer.erase(pAVFirst ++);
				}
				else
					break;
			}

			now = GetTickCount();
			if (now < tAVRecvTime)
				interval = now;
			else
				interval = now - tAVRecvTime;
			if (interval > 120000)
			{
				for (pAVFirst = AVBuffer.begin();pAVFirst != AVBuffer.end();)
				{
					if (pAVFirst->second.iSampleType == 0)
					{
						pImage = pAVFirst->second.pImage;
						p->OnImage(pImage,false);
					}
					if (pAVFirst->second.iSampleType == 1)
					{
						pAudio = pAVFirst->second.pAudio;
						p->OnAudio(pAudio,false);
					}
					
					AVBuffer.erase(pAVFirst ++);
				}
				
				if (bVideoPlaying || bVideoTryPlaying)
				{
					Send_s.Init(OPR_VIDEO_END,MoIP_OPR_FLAG);
					Send_s.EncodeCommand();
					Send_s.Send_t(s_opr);
					if (bVideoPlaying)
					{
						bVideoPlaying = false;
						p->OnVideoStopped(ERROR_TIME_OUT);
					}
					if (bVideoTryPlaying)
					{
						bVideoTryPlaying = false;
						p->OnPlayVideoResult(ERROR_TIME_OUT);
					}
				}
				if (bAudioPlaying || bAudioTryPlaying)
				{
					Send_s.Init(OPR_AUDIO_END,MoIP_OPR_FLAG);
					Send_s.EncodeCommand();
					Send_s.Send_t(s_opr);
					if (bAudioPlaying)
					{
						bAudioPlaying = false;
						p->OnAudioStopped(ERROR_TIME_OUT);
					}
					if (bAudioTryPlaying)
					{
						bAudioTryPlaying = false;
						p->OnPlayAudioResult(ERROR_TIME_OUT);
					}

					dwFirstCameraTick = 0;
				}
				if (bTalking || bTryTalking)
				{
					Send_s.Init(OPR_SPEAK_END,MoIP_OPR_FLAG);
					Send_s.EncodeCommand();
					Send_s.Send_t(s_opr);
					
					if (bTalking)
					{
						bTalking = false;
						p->OnTalkStopped(ERROR_SOCKET);
					}
					if (bTryTalking)
					{
						bTryTalking = false;
						p->OnStartTalkResult(ERROR_SOCKET);
					}
				}

				closesocket(s_av);
				s_av = INVALID_SOCKET;
			}
		}
			
		while (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
		{
			now = GetTickCount();
			switch(msg.message) 
			{
			case WM_PLAY_VIDEO:
				if (! bConnected)
					break;
				if (bVideoPlaying || bVideoTryPlaying)
					break;
				
				Send_s.Init(OPR_VIDEO_START_REQ,MoIP_OPR_FLAG);
				Send_s.AddNext((char)ucProtocol);
				Send_s.EncodeCommand();
				Send_s.Send_t(s_opr);
				bVideoTryPlaying = true;
				
				break;
			case WM_STOP_VIDEO:
				if (! bConnected)
					break;
				
				if (bVideoPlaying || bVideoTryPlaying)
				{
					Send_s.Init(OPR_VIDEO_END,MoIP_OPR_FLAG);
					Send_s.EncodeCommand();
					Send_s.Send_t(s_opr);

					if ((! bAudioPlaying) && (! bAudioTryPlaying)
						&& (! bTalking) && (! bTryTalking))
					{
						closesocket(s_av);
						s_av = INVALID_SOCKET;
					}
					
					if (bVideoTryPlaying)
					{
						bVideoTryPlaying = false;
						p->OnPlayVideoResult(ERROR_CANCEL);
					}
					if (bVideoPlaying)
					{
						for (pAVFirst = AVBuffer.begin();pAVFirst != AVBuffer.end();)
						{
							if (pAVFirst->second.iSampleType == 0)
							{
								pImage = pAVFirst->second.pImage;
								p->OnImage(pImage,false);
								AVBuffer.erase(pAVFirst ++);
							}
							else
								++ pAVFirst;
						}
						
						iFramesRecvedPerS = 0;
						iVideoBandWidth = 0;
						
						bVideoPlaying = false;
						p->OnVideoStopped(OK);
					}
				}

				break;
			case WM_PLAY_AUDIO:
				if (! bConnected)
					break;
				if (bAudioPlaying || bAudioTryPlaying)
					break;
				
				Send_s.Init(OPR_AUDIO_START_REQ,MoIP_OPR_FLAG);
				Send_s.AddNext((char)ucProtocol);	
				Send_s.EncodeCommand();
				Send_s.Send_t(s_opr);
				bAudioTryPlaying = true;
				
				break;
			case WM_STOP_AUDIO:
				if (! bConnected)
					break;
				
				if (bAudioPlaying || bAudioTryPlaying)
				{
					Send_s.Init(OPR_AUDIO_END,MoIP_OPR_FLAG);
					Send_s.EncodeCommand();
					Send_s.Send_t(s_opr);

					if ((! bVideoPlaying) && (! bVideoTryPlaying)
						&& (! bTalking) && (! bTryTalking))
					{
						closesocket(s_av);
						s_av = INVALID_SOCKET;
					}
					
					if (bAudioTryPlaying)
					{
						bAudioTryPlaying = false;
						p->OnPlayAudioResult(ERROR_CANCEL);
					}
					if (bAudioPlaying)
					{
						for (pAVFirst = AVBuffer.begin();pAVFirst != AVBuffer.end();)
						{
							if (pAVFirst->second.iSampleType == 0)
							{
								pImage = pAVFirst->second.pImage;
								p->OnImage(pImage,false);
							}
							else if (pAVFirst->second.iSampleType == 1)
							{
								pAudio = pAVFirst->second.pAudio;
								p->OnAudio(pAudio,false);
							}
							
							AVBuffer.erase(pAVFirst ++);
						}

						iAudioSamples = 0;
						iAudioBandWidth = 0;
						
						bAudioPlaying = false;
						p->OnAudioStopped(OK);
					}
					
					dwFirstCameraTick = 0;
				}
				
				break;
			case WM_START_TALK:
				if (! bConnected)
					break;
				if (bTalking || bTryTalking)
					break;
				
				Send_s.Init(OPR_SPEAK_START_NOTIFY,MoIP_OPR_FLAG);
				Send_s.AddNext((unsigned char)iBufferTime);
				Send_s.EncodeCommand();
				Send_s.Send_t(s_opr);
				bTryTalking = true;
				
				break;
			case WM_STOP_TALK:
				if (! bConnected)
					break;
				
				if (bTalking || bTryTalking)
				{
					Send_s.Init(OPR_SPEAK_END,MoIP_OPR_FLAG);
					Send_s.EncodeCommand();
					Send_s.Send_t(s_opr);

					if ((! bVideoPlaying) && (! bVideoTryPlaying)
						&& (! bAudioPlaying) && (! bAudioTryPlaying))
					{
						closesocket(s_av);
						s_av = INVALID_SOCKET;
					}
					
					if (bTryTalking)
					{
						bTryTalking = false;
						p->OnStartTalkResult(ERROR_CANCEL);
					}
					if (bTalking)
					{
						bTalking = false;
						talk_datalen = 0;
						p->OnTalkStopped(OK);
					}
				}
				
				break;
			case WM_RATE:
				if (! bConnected)
					break;
				
				iMaxRate = msg.wParam;
				iMaxRate %= 24;
				iMaxRate *= 10;
				Send_s.Init(OPR_VIDEO_RATE,MoIP_OPR_FLAG);
				Send_s.AddNext(iMaxRate);
				Send_s.EncodeCommand();
				Send_s.Send_t(s_opr);
				
				break;
			case WM_BUFFER:
				if (! bConnected)
					break;
				
				if (msg.wParam < iBufferTime)
				{
					if (bAudioPlaying || bVideoPlaying)
					{
						for (pAVFirst = AVBuffer.begin();pAVFirst != AVBuffer.end();)
						{
							if (GetTickCount() - pAVFirst->first < msg.wParam * 1000)
							{
								//TRACE("buffer lost %d %d\n",now,pAVFirst->first);
								break;
							}

							if (pAVFirst->second.iSampleType == 0)
							{
								pImage = pAVFirst->second.pImage;
								p->OnImage(pImage,false);
							}
							if (pAVFirst->second.iSampleType == 1)
							{
								pAudio = pAVFirst->second.pAudio;
								p->OnAudio(pAudio,false);
							}
							AVBuffer.erase(pAVFirst ++);
						}
					}
				}

				iBufferTime = msg.wParam;

				if (bTalking)
				{
					Send_s.Init(OPR_BUFFER,MoIP_OPR_FLAG);
					Send_s.AddNext((unsigned char)msg.wParam);
					Send_s.EncodeCommand();
					Send_s.Send_t(s_opr);
				}
	
				break;
			case WM_SENSOR:
				if (! bConnected)
					break;

				Send_s.Init(OPR_PARAMS_SET_REQ,MoIP_OPR_FLAG);
				Send_s.AddNext((unsigned char)msg.wParam);
				Send_s.AddNext((unsigned char)msg.lParam);
				Send_s.EncodeCommand();
				Send_s.Send_t(s_opr);
				
				break;
			case WM_DECODER:
				if (! bConnected)
					break;
				
				Send_s.Init(OPR_DECODER_CONTROL_REQ,MoIP_OPR_FLAG);
				Send_s.AddNext((unsigned char)msg.wParam);
				Send_s.EncodeCommand();
				Send_s.Send_t(s_opr);
				
				break;
			case WM_COMM:
				if (! bConnected)
					break;
				
				Send_s.Init(OPR_COMM_DATA,MoIP_OPR_FLAG);
				Send_s.AddNext((unsigned char)(* ((unsigned int *)msg.wParam)));
				Send_s.AddNext((unsigned char *)msg.lParam,* (unsigned int *)msg.wParam);
				Send_s.AddNext(* ((unsigned int *)msg.wParam + 1));
				Send_s.EncodeCommand();
				Send_s.Send_t(s_opr);
				
				delete ((unsigned int *)msg.wParam);
				delete ((unsigned char *)msg.lParam);
				
				break;
			case WM_TALK:
				pAudio = (AUDIO *)msg.wParam;
				if (bTalking)
				{
					Send_s.Init(TALK_DATA,MoIP_AV_FLAG);
					Send_s.AddNext(pAudio->uiTick);
					Send_s.AddNext(pAudio->uiSeq);
					Send_s.AddNext(pAudio->iTime);
					Send_s.AddNext(pAudio->ucFormat);
					Send_s.AddNext(pAudio->uiDataLen);
					Send_s.AddNext(pAudio->pData,pAudio->uiDataLen);
					Send_s.EncodeCommand();
					if (talk_datalen + Send_s.m_iLength < TALK_BUFFER_SIZE)
					{
						memcpy(talk_buffer + talk_datalen,Send_s.m_pBuffer,Send_s.m_iLength);
						talk_datalen += Send_s.m_iLength;
					}
					//else
					//{
					//	TRACE("lost audio\n");
					//}
				}

				p->FreeAudio(pAudio);

				break;
			case WM_QUIT:
				if (bConnected)
					sResult = OK;
				else
					sResult = ERROR_CANCEL;
				
				goto quit;
			}
		}
		
		FD_ZERO(&fdr);
		FD_ZERO(&fdw);
		FD_SET(s_opr,&fdr);
		if (s_av != INVALID_SOCKET)
		{
			FD_SET(s_av,&fdr);
			if (bTalking && talk_datalen)
				FD_SET(s_av,&fdw);
		}
		timeout.tv_sec = 0;
		timeout.tv_usec = 100;

		if (0 >= select(0,&fdr,&fdw,NULL,&timeout))
			continue;
		
		if (FD_ISSET(s_opr,&fdr))
		{
			sResult = Recv_opr.Recv(s_opr,0);
			if ((sResult == ERROR_SOCKET) || (sResult == ERROR_CLOSED))
				goto quit;
			
			if (sResult == OK)
			{
				if (bConnected)
					tRecvTime = now;
			
				while (Recv_opr.CheckCommand(&sOpCode,MoIP_OPR_FLAG))
				{
					now = GetTickCount();
					switch(sOpCode) 
					{
					case OPR_LOGIN_RESP:
						if (bConnected)
							break;
					
						if (! Recv_opr.GetNext(&sResult))
						{
							sResult = ERROR_UNKNOWN;
							goto quit;
						}
						if (sResult != OK)
							goto quit;
						if ((! Recv_opr.GetNext(pMsid,13)) || (! Recv_opr.GetNext(&dwOprVersion)) || (! Recv_opr.GetNext(&dwModel)) || (! Recv_opr.GetNext(&dwCameraVersion)))
						{
							sResult = ERROR_UNKNOWN;
							goto quit;
						}
						if (dwOprVersion > MOIP_OPR_VER)
						{
							sResult = ERROR_VERSION;
							goto quit;
						}

						Send_s.Init(OPR_VERIFY_REQ,MoIP_OPR_FLAG);
						Send_s.AddNext(p->m_strUser.GetBufferSetLength(13),13);
						p->m_strUser.ReleaseBuffer();
						Send_s.AddNext(p->m_strPwd.GetBufferSetLength(13),13);
						p->m_strPwd.ReleaseBuffer();
						Send_s.EncodeCommand();
						Send_s.Send_t(s_opr);
					
						break;
					case OPR_VERIFY_RESP:
						if (bConnected)
							break;

						if (! Recv_opr.GetNext(&sResult))
						{
							sResult = ERROR_UNKNOWN;
							goto quit;
						}
					
						if (sResult == OK)
						{
							if (! Recv_opr.GetNext(&ucPri))
							{
								sResult = ERROR_UNKNOWN;
								goto quit;
							}

							bConnected = true;
							p->OnConnectResult(sResult,ucPri);
							Send_s.Init(OPR_PARAMS_FETCH_REQ,MoIP_OPR_FLAG);
							Send_s.EncodeCommand();
							Send_s.Send_t(s_opr);
						}
						else
						{
							goto quit;
						}
					
						break;
					case OPR_VIDEO_START_RESP:
						if (! bVideoTryPlaying)
							break;

						bVideoTryPlaying = false;
					
						if (! Recv_opr.GetNext(&sResult))
						{
							p->OnPlayVideoResult(ERROR_UNKNOWN);
							break;
						}
					
						if (sResult != OK)
						{
							p->OnPlayVideoResult(sResult);
								break;
						}

						if (s_av == INVALID_SOCKET)
						{
							{
								if (! Recv_opr.GetNext(&dwAVConnID))
								{
									p->OnPlayVideoResult(ERROR_UNKNOWN);
									Send_s.Init(OPR_VIDEO_END,MoIP_OPR_FLAG);
									Send_s.EncodeCommand();
									Send_s.Send_t(s_opr);
							
									break;
								}

								s_av = socket(AF_INET,SOCK_STREAM,0);
								
								if (INVALID_SOCKET == s_av)
								{
									p->OnPlayVideoResult(ERROR_SOCKET);
									Send_s.Init(OPR_VIDEO_END,MoIP_OPR_FLAG);
									Send_s.EncodeCommand();
									Send_s.Send_t(s_opr);
							
									break;
								}
						
								opt = 64 * 1024;
								setsockopt(s_av,SOL_SOCKET,SO_RCVBUF,(char *)&opt,sizeof(opt));

								Send_s.Init(AV_LOGIN_REQ,MoIP_AV_FLAG);
								Send_s.AddNext(dwAVConnID);
								Send_s.EncodeCommand();
								{
									addr.sin_family = AF_INET;
									addr.sin_addr.S_un.S_addr = p->m_dwIP;
									addr.sin_port = p->m_usPort;
								
									if (SOCKET_ERROR == connect(s_av,(struct sockaddr *)&addr,sizeof(addr)))
									{
										closesocket(s_av);
										p->OnPlayVideoResult(ERROR_SOCKET);
										Send_s.Init(OPR_VIDEO_END,MoIP_OPR_FLAG);
										Send_s.EncodeCommand();
										Send_s.Send_t(s_opr);
									
										break;
									}
								
									Send_s.Send_t(s_av);
									bAVLogon = true;	
								}
							}

							tAVRecvTime = now;
							dwPre1Second = GetTickCount();
						}

						iVideoBandWidth = 0;
						iFramesRecvedPerS = 0;
						bVideoPlaying = true;
						p->OnPlayVideoResult(sResult);
					
						break;
					case OPR_AUDIO_START_RESP:
						if (! bAudioTryPlaying)
							break;

						bAudioTryPlaying = false;
					
						if (! Recv_opr.GetNext(&sResult))
						{
							p->OnPlayAudioResult(ERROR_UNKNOWN);
							break;
						}
					
						if (sResult != OK)
						{
							p->OnPlayAudioResult(sResult);
							break;
						}

						if (s_av == INVALID_SOCKET)
						{
							{
								if (! Recv_opr.GetNext(&dwAVConnID))
								{
									p->OnPlayAudioResult(ERROR_UNKNOWN);
									Send_s.Init(OPR_AUDIO_END,MoIP_OPR_FLAG);
									Send_s.EncodeCommand();
									Send_s.Send_t(s_opr);
							
									break;
								}

									s_av = socket(AF_INET,SOCK_STREAM,0);
							
								if (INVALID_SOCKET == s_av)
								{
									p->OnPlayAudioResult(ERROR_SOCKET);
									Send_s.Init(OPR_AUDIO_END,MoIP_OPR_FLAG);
									Send_s.EncodeCommand();
									Send_s.Send_t(s_opr);
							
									break;
								}
						
								opt = 64 * 1024;
								setsockopt(s_av,SOL_SOCKET,SO_RCVBUF,(char *)&opt,sizeof(opt));
	
								Send_s.Init(AV_LOGIN_REQ,MoIP_AV_FLAG);
								Send_s.AddNext(dwAVConnID);
								Send_s.EncodeCommand();
								{
									addr.sin_family = AF_INET;
									addr.sin_addr.S_un.S_addr = p->m_dwIP;
									addr.sin_port = p->m_usPort;
								
									if (SOCKET_ERROR == connect(s_av,(struct sockaddr *)&addr,sizeof(addr)))
									{
										closesocket(s_av);
										p->OnPlayAudioResult(ERROR_SOCKET);
										Send_s.Init(OPR_AUDIO_END,MoIP_OPR_FLAG);
										Send_s.EncodeCommand();
										Send_s.Send_t(s_opr);
									
										break;
									}
								
									Send_s.Send_t(s_av);
									bAVLogon = true;	
								}
							}
	
							tAVRecvTime = now;
							dwPre1Second = GetTickCount();
						}

						iAudioBandWidth = 0;
						iAudioSamples = 0;
						bAudioPlaying = true;
						decode_index = decode_sample = 0;
						p->OnPlayAudioResult(sResult);
					
						break;
					case OPR_SPEAK_START_RESP:
						if (! bTryTalking)
							break;

						bTryTalking = false;
					
						if (! Recv_opr.GetNext(&sResult))
						{
							p->OnStartTalkResult(ERROR_UNKNOWN);
							break;
						}
					
						if (sResult != OK)
						{
							p->OnStartTalkResult(sResult);
							break;
						}

						if (s_av == INVALID_SOCKET)
						{
							if (! Recv_opr.GetNext(&dwAVConnID))
							{
								p->OnStartTalkResult(ERROR_UNKNOWN);
								Send_s.Init(OPR_SPEAK_END,MoIP_OPR_FLAG);
								Send_s.EncodeCommand();
								Send_s.Send_t(s_opr);
							
								break;
							}

							s_av = socket(AF_INET,SOCK_STREAM,0);
							
							if (INVALID_SOCKET == s_av)
							{
								p->OnStartTalkResult(ERROR_SOCKET);
								Send_s.Init(OPR_SPEAK_END,MoIP_OPR_FLAG);
								Send_s.EncodeCommand();
								Send_s.Send_t(s_opr);
							
								break;
							}
						
							opt = 64 * 1024;
							setsockopt(s_av,SOL_SOCKET,SO_RCVBUF,(char *)&opt,sizeof(opt));

							Send_s.Init(AV_LOGIN_REQ,MoIP_AV_FLAG);
							Send_s.AddNext(dwAVConnID);
							Send_s.EncodeCommand();
							addr.sin_family = AF_INET;
							addr.sin_addr.S_un.S_addr = p->m_dwIP;
							addr.sin_port = p->m_usPort;
									
							if (SOCKET_ERROR == connect(s_av,(struct sockaddr *)&addr,sizeof(addr)))
							{
								closesocket(s_av);
								p->OnStartTalkResult(ERROR_SOCKET);
								Send_s.Init(OPR_SPEAK_END,MoIP_OPR_FLAG);
								Send_s.EncodeCommand();
								Send_s.Send_t(s_opr);
								
								break;
							}
								
							Send_s.Send_t(s_av);
							bAVLogon = true;	
							
							tAVRecvTime = now;
						}

						bTalking = true;
						talk_datalen = 0;
						p->OnStartTalkResult(sResult);
						
						break;
					case OPR_PARAMS_FETCH_RESP:
						if (! Recv_opr.GetNext(&ucValue))
							break;
						p->OnMonitorParamsChanged(SENSOR_RESOLUTION,ucValue);
						if (! Recv_opr.GetNext(&ucValue))
							break;
						p->OnMonitorParamsChanged(SENSOR_BRIGHTNESS,ucValue);
						if (! Recv_opr.GetNext(&ucValue))
							break;
						p->OnMonitorParamsChanged(SENSOR_CONTRAST,ucValue);
						if (! Recv_opr.GetNext(&ucValue))
							break;
						p->OnMonitorParamsChanged(SENSOR_SATURATION,ucValue);
						if (! Recv_opr.GetNext(&ucValue))
							break;
						p->OnMonitorParamsChanged(SENSOR_HUE,ucValue);
						if (! Recv_opr.GetNext(&ucValue))
							break;
						p->OnMonitorParamsChanged(SENSOR_FLIP,ucValue);
						if (! Recv_opr.GetNext(&ucValue))
							break;
						p->OnMonitorParamsChanged(SENSOR_RATE,ucValue);

						break;
					case OPR_PARAMS_CHANGED_NOTIFY:
						if ((! Recv_opr.GetNext(&ucSet)) || (! Recv_opr.GetNext(&ucValue)))
							break;
						
						p->OnMonitorParamsChanged(ucSet,ucValue);
						
						break;
					case OPR_ALARM2_NOTIFY:
						if ((! Recv_opr.GetNext(&ucAlarm)) || (! Recv_opr.GetNext(&sMotionLeft)) || (! Recv_opr.GetNext(&sMotionTop)) || (! Recv_opr.GetNext(&sMotionRight)) || (! Recv_opr.GetNext(&sMotionBottom)))
							break;
					
						p->OnAlarm2(ucAlarm,sMotionLeft,sMotionTop,sMotionRight,sMotionBottom);
					
						break;
					case OPR_OTHERDEVICES_PARAMS_NOTIFY:
						pOtherDevicesParams = new OTHER_DEVICE_PARAMS[9];
						memset(pOtherDevicesParams,0,sizeof(OTHER_DEVICE_PARAMS) * 9);
						if (Recv_opr.GetNext(pOtherDevicesParams[0].msid,13)
							&& Recv_opr.GetNext(pOtherDevicesParams[0].alias,21)
							&& Recv_opr.GetNext(pOtherDevicesParams[0].host,65)
							&& Recv_opr.GetNext(&pOtherDevicesParams[0].port)
							&& Recv_opr.GetNext(pOtherDevicesParams[0].user,13)
							&& Recv_opr.GetNext(pOtherDevicesParams[0].pwd,13)
							&& Recv_opr.GetNext(&pOtherDevicesParams[0].mode)
							&& Recv_opr.GetNext(pOtherDevicesParams[1].msid,13)
							&& Recv_opr.GetNext(pOtherDevicesParams[1].alias,21)
							&& Recv_opr.GetNext(pOtherDevicesParams[1].host,65)
							&& Recv_opr.GetNext(&pOtherDevicesParams[1].port)
							&& Recv_opr.GetNext(pOtherDevicesParams[1].user,13)
							&& Recv_opr.GetNext(pOtherDevicesParams[1].pwd,13)
							&& Recv_opr.GetNext(&pOtherDevicesParams[1].mode)
							&& Recv_opr.GetNext(pOtherDevicesParams[2].msid,13)
							&& Recv_opr.GetNext(pOtherDevicesParams[2].alias,21)
							&& Recv_opr.GetNext(pOtherDevicesParams[2].host,65)
							&& Recv_opr.GetNext(&pOtherDevicesParams[2].port)
							&& Recv_opr.GetNext(pOtherDevicesParams[2].user,13)
							&& Recv_opr.GetNext(pOtherDevicesParams[2].pwd,13)
							&& Recv_opr.GetNext(&pOtherDevicesParams[2].mode)
							&& Recv_opr.GetNext(pOtherDevicesParams[3].msid,13)
							&& Recv_opr.GetNext(pOtherDevicesParams[3].alias,21)
							&& Recv_opr.GetNext(pOtherDevicesParams[3].host,65)
							&& Recv_opr.GetNext(&pOtherDevicesParams[3].port)
							&& Recv_opr.GetNext(pOtherDevicesParams[3].user,13)
							&& Recv_opr.GetNext(pOtherDevicesParams[3].pwd,13)
							&& Recv_opr.GetNext(&pOtherDevicesParams[3].mode)
							)
						{
							for (i = 4;i < 9;i ++)
							{
								Recv_opr.GetNext(pOtherDevicesParams[i].msid,13);
								Recv_opr.GetNext(pOtherDevicesParams[i].alias,21);
								Recv_opr.GetNext(pOtherDevicesParams[i].host,65);
								Recv_opr.GetNext(&pOtherDevicesParams[i].port);
								Recv_opr.GetNext(pOtherDevicesParams[i].user,13);
								Recv_opr.GetNext(pOtherDevicesParams[i].pwd,13);
								Recv_opr.GetNext(&pOtherDevicesParams[i].mode);
							}
							p->OnOtherDevicesParamsChanged(pOtherDevicesParams);	
						}
						else
						{
							delete [] pOtherDevicesParams;
						}
						
						break;
					case OPR_KEEP_ALIVE:

						break;
					}
				}
			}
		}

		if ((s_av != INVALID_SOCKET) && (FD_ISSET(s_av,&fdw)))
		{
			re = send(s_av,talk_buffer,talk_datalen,0);
			if (re > 0)
			{
				memcpy(talk_buffer,talk_buffer + re,talk_datalen - re);
				talk_datalen -= re;
			}
		}

		if ((s_av != INVALID_SOCKET) && (FD_ISSET(s_av,&fdr)))
		{
			sResult = Recv_av_t.Recv(s_av,0);
			
			if ((sResult == ERROR_SOCKET) || (sResult == ERROR_CLOSED))
			{
				for (pAVFirst = AVBuffer.begin();pAVFirst != AVBuffer.end();)
				{
					if (pAVFirst->second.iSampleType == 0)
					{
						pImage = pAVFirst->second.pImage;
						p->OnImage(pImage,false);
					}
					if (pAVFirst->second.iSampleType == 1)
					{
						pAudio = pAVFirst->second.pAudio;
						p->OnAudio(pAudio,false);
					}
					
					AVBuffer.erase(pAVFirst ++);
				}
								
				closesocket(s_av);
				s_av = INVALID_SOCKET;
				
				if (bVideoPlaying || bVideoTryPlaying)
				{
					Send_s.Init(OPR_VIDEO_END,MoIP_OPR_FLAG);
					Send_s.EncodeCommand();
					Send_s.Send_t(s_opr);
					
					if (bVideoPlaying)
					{
						bVideoPlaying = false;
						p->OnVideoStopped(ERROR_SOCKET);
					}
					if (bVideoTryPlaying)
					{
						bVideoTryPlaying = false;
						p->OnPlayVideoResult(ERROR_SOCKET);
					}
				}

				if (bAudioPlaying || bAudioTryPlaying)
				{
					Send_s.Init(OPR_AUDIO_END,MoIP_OPR_FLAG);
					Send_s.EncodeCommand();
					Send_s.Send_t(s_opr);
					
					if (bAudioPlaying)
					{
						bAudioPlaying = false;
						p->OnAudioStopped(ERROR_SOCKET);
					}
					if (bAudioTryPlaying)
					{
						bAudioTryPlaying = false;
						p->OnPlayAudioResult(ERROR_SOCKET);
					}
					
					dwFirstCameraTick = 0;
				}

				if (bTalking || bTryTalking)
				{
					Send_s.Init(OPR_SPEAK_END,MoIP_OPR_FLAG);
					Send_s.EncodeCommand();
					Send_s.Send_t(s_opr);
					
					if (bTalking)
					{
						bTalking = false;
						p->OnTalkStopped(ERROR_SOCKET);
					}
					if (bTryTalking)
					{
						bTryTalking = false;
						p->OnStartTalkResult(ERROR_SOCKET);
					}
				}
			}
			
			now = GetTickCount();

			if (sResult == OK)
			{
				{
					tAVRecvTime = now;
					
					while (Recv_av_t.CheckCommand(&sOpCode,MoIP_AV_FLAG))
					{
						switch(sOpCode) 
						{
						case VIDEO_DATA:
							pImage = new IMAGE;
							pImage->pData = NULL;
							if (Recv_av_t.GetNext(&pImage->uiTick) && Recv_av_t.GetNext(&pImage->iTime) && Recv_av_t.GetNext(&ucCodedFormat) && Recv_av_t.GetNext(&pImage->uiDataLen))
							{
								{
									pImage->pData = new char[pImage->uiDataLen];
									if (Recv_av_t.GetNext(pImage->pData,pImage->uiDataLen))
									{
										iVideoBandWidth = iVideoBandWidth + pImage->uiDataLen;
										iFramesRecvedPerS ++;
										if (! bVideoPlaying)
										{
											p->FreeImage(pImage);
										}
										else if (dwFirstCameraTick)
										{
											av_sample.iSampleType = 0;
											av_sample.pAudio = NULL;
											av_sample.pImage = pImage;
											dwCameraTick = (pImage->uiTick - dwFirstCameraTick) * 10 + dwFirstComputerTick + 300;
											AVBuffer.insert(std::make_pair(dwCameraTick,av_sample));
										}
										else
										{
											p->OnImage(pImage,true);					
										}
									}
									else
									{	
										p->FreeImage(pImage);
									}
								}	
							}	
							else
							{
								p->FreeImage(pImage);
							}
							
							break;
						case AUDIO_DATA:
							pAudio = new AUDIO;
							pAudio->pData = NULL;
							if (Recv_av_t.GetNext(&pAudio->uiTick) && Recv_av_t.GetNext(&pAudio->uiSeq) && Recv_av_t.GetNext(&pAudio->iTime) && Recv_av_t.GetNext(&pAudio->ucFormat) && Recv_av_t.GetNext(&pAudio->uiDataLen))
							{
								{
									pAudio->pData = new unsigned char[pAudio->uiDataLen * 4];
									audio_raw_data = new unsigned char[pAudio->uiDataLen];
									if (Recv_av_t.GetNext(audio_raw_data,pAudio->uiDataLen))
									{
										iAudioBandWidth = iAudioBandWidth + pAudio->uiDataLen;
										++ iAudioSamples;
										if (bAudioPlaying)
										{
											adpcm_decode(audio_raw_data,pAudio->uiDataLen,pAudio->pData,&decode_sample,&decode_index);
											pAudio->uiDataLen *= 4;
											av_sample.iSampleType = 1;
											av_sample.pImage = NULL;
											av_sample.pAudio = pAudio;
											if ((dwFirstCameraTick == 0) || (pAudio->uiTick - dwFirstCameraTick) > 60000)
											{
												dwFirstCameraTick = pAudio->uiTick;
												dwFirstComputerTick = GetTickCount();
											}
											dwCameraTick = (pAudio->uiTick - dwFirstCameraTick) * 10 + dwFirstComputerTick;
											AVBuffer.insert(std::make_pair(dwCameraTick,av_sample));
										}
										else
										{
											p->FreeAudio(pAudio);
										}
									}
									else
									{
										p->FreeAudio(pAudio);
									}

									delete audio_raw_data;
								}
							}
							else
							{
								p->FreeAudio(pAudio);
							}
							
							break;
						}
					}
				}
			}
		}
	}

quit:

	if (s_opr != INVALID_SOCKET)
		closesocket(s_opr);
	if (s_av != INVALID_SOCKET)
		closesocket(s_av);
	
	for (pAVFirst = AVBuffer.begin();pAVFirst != AVBuffer.end();)
	{
		if (pAVFirst->second.iSampleType == 0)
		{
			pImage = pAVFirst->second.pImage;
			p->OnImage(pImage,false);
		}
		if (pAVFirst->second.iSampleType == 1)
		{
			pAudio = pAVFirst->second.pAudio;
			p->OnAudio(pAudio,false);
		}
		AVBuffer.erase(pAVFirst ++);
	}
	
	if (bTalking)
		p->OnTalkStopped(ERROR_SOCKET);
	if (bTryTalking)
		p->OnStartTalkResult(ERROR_SOCKET);
	if (bVideoTryPlaying)
		p->OnPlayVideoResult(sResult);
	if (bVideoPlaying)
		p->OnVideoStopped(sResult);
	if (bAudioTryPlaying)
		p->OnPlayAudioResult(sResult);
	if (bAudioPlaying)
		p->OnAudioStopped(sResult);
	if (bConnected)
		p->OnDisconnected(sResult);
	else
		p->OnConnectResult(sResult,PRI_MONITOR);
	
	return 0;
}


static int index_adjust[8] = {-1,-1,-1,-1,2,4,6,8};

static int step_table[89] = 
{
	7,8,9,10,11,12,13,14,16,17,19,21,23,25,28,31,34,37,41,45,
	50,55,60,66,73,80,88,97,107,118,130,143,157,173,190,209,230,253,279,307,337,371,
	408,449,494,544,598,658,724,796,876,963,1060,1166,1282,1411,1552,1707,1878,2066,
	2272,2499,2749,3024,3327,3660,4026,4428,4871,5358,5894,6484,7132,7845,8630,9493,
	10442,11487,12635,13899,15289,16818,18500,20350,22385,24623,27086,29794,32767
};

void adpcm_encode(unsigned char * raw, int len, unsigned char * encoded, int * pre_sample, int * index)
{
	short * pcm = (short *)raw;
	int cur_sample;
	int i;
	int delta;
	int sb;
	int code;
	len >>= 1;
	
	for (i = 0;i < len;i ++)
	{
		cur_sample = pcm[i]; // 得到当前的采样数据
		delta = cur_sample - * pre_sample; // 计算出和上一个的增量
		if (delta < 0)
		{
			delta = -delta;
			sb = 8;	//	取绝对值
		}
		else 
		{
			sb = 0;
		}	// sb 保存的是符号位
		code = 4 * delta / step_table[* index];	// 根据 steptable[]得到一个 0-7 的值
		if (code>7) 
			code=7;	// 它描述了声音强度的变化量
		
		delta = (step_table[* index] * code) / 4 + step_table[* index] / 8;	// 后面加的一项是为了减少误差
		if (sb) 
			delta = -delta;
		* pre_sample += delta;	// 计算出当前的波形数据
		if (* pre_sample > 32767)
			* pre_sample = 32767;
		else if (* pre_sample < -32768)
			* pre_sample = -32768;
		//* pre_sample = cur_sample;
		
		* index += index_adjust[code];	// 根据声音强度调整下次取steptable 的序号
		if (* index < 0) 
			* index = 0;	// 便于下次得到更精确的变化量的描述
		else if (* index > 88) 
			* index = 88;
		
		if (i & 0x01)
			encoded[i >> 1] |= code | sb;
		else
			encoded[i >> 1] = (code | sb) << 4;	// 加上符号位保存起来
	}
}

void adpcm_decode(unsigned char * raw, int len, unsigned char * decoded, int * pre_sample, int * index)
{
	int i;
	int code;
	int sb;
	int delta;
	short * pcm = (short *)decoded;
	len <<= 1;
	
	for (i = 0;i < len;i ++)
	{
		if (i & 0x01)
			code = raw[i >> 1] & 0x0f;	// 得到下一个数据
		else
			code = raw[i >> 1] >> 4;	// 得到下一个数据
		if ((code & 8) != 0) 
			sb = 1;
		else 
			sb = 0;
		code &= 7;	//	将 code 分离为数据和符号

		delta = (step_table[* index] * code) / 4 + step_table[* index] / 8;	// 后面加的一项是为了减少误差
		if (sb) 
			delta = -delta;
		* pre_sample += delta;	// 计算出当前的波形数据
		if (* pre_sample > 32767)
			* pre_sample = 32767;
		else if (* pre_sample < -32768)
			* pre_sample = -32768;
		pcm[i] = * pre_sample;
		/*
		if (* pre_sample > 32767)
		pcm[i] = 32767;
		else if (* pre_sample < -32768)
		pcm[i] = -32768;
		else 
		pcm[i] = * pre_sample;
		*/
		* index += index_adjust[code];
		if (* index < 0) 
			* index = 0;
		if (* index > 88) 
			* index = 88;
	}
}
