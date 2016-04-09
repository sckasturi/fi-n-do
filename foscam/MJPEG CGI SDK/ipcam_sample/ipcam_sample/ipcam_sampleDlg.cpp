/*
 * Copyright 2010-2011 Foscam Intelligent Technologies Co., Ltd. 
 *
 * All rights reserved
 *
*/

// ipcam_sampleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ipcam_sample.h"
#include "ipcam_sampleDlg.h"
#include "showwnd.h"
#include "WaveOut.h"
#include "ipcam.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CIpcam_sampleDlg dialog

CIpcam_sampleDlg::CIpcam_sampleDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CIpcam_sampleDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CIpcam_sampleDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CIpcam_sampleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CIpcam_sampleDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CIpcam_sampleDlg, CDialog)
	//{{AFX_MSG_MAP(CIpcam_sampleDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_BN_CLICKED(ID_CONNECT, OnConnect)
	ON_BN_CLICKED(ID_DISCONNECT, OnDisconnect)
	ON_BN_CLICKED(ID_PLAY_VIDEO, OnPlayVideo)
	ON_BN_CLICKED(ID_STOP_VIDEO, OnStopVideo)
	ON_BN_CLICKED(ID_PLAY_AUDIO, OnPlayAudio)
	ON_MESSAGE(WM_MONITOR_CONNECT_RESULT, OnConnectResult)
	ON_MESSAGE(WM_MONITOR_DISCONNECTED, OnDisconnected)
	ON_MESSAGE(WM_PLAYVIDEO_RESULT, OnPlayVideoResult)
	ON_MESSAGE(WM_PLAYAUDIO_RESULT, OnPlayAudioResult)
	ON_MESSAGE(WM_VIDEO_STOPPED, OnVideoStopped)
	ON_MESSAGE(WM_AUDIO_STOPPED, OnAudioStopped)
	ON_MESSAGE(WM_VIDEO, OnImage)
	ON_MESSAGE(WM_AUDIO, OnAudio)
	ON_MESSAGE(WM_OTHERDEVICES_PARAMS_CHANGED, OnOtherDevicesParamsChanged)
	ON_BN_CLICKED(ID_STOP_AUDIO, OnStopAudio)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CIpcam_sampleDlg message handlers

BOOL CIpcam_sampleDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	RECT rect;
	rect.left = 5;
	rect.top = 5;
	rect.right = rect.left + 320;
	rect.bottom = rect.top + 240;
	
	m_pShowWnd = new CShowWnd();
	m_pShowWnd->Create(NULL,"Show Wnd",WS_CHILD|WS_VISIBLE,rect,this,1236,NULL);
	
	m_pWaveOut = new CWaveOut();
	m_pWaveOut->StartPlay();


	m_pIPCam = new COpr(this);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CIpcam_sampleDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CIpcam_sampleDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CIpcam_sampleDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	m_pShowWnd->DestroyWindow();
	delete m_pShowWnd;	
	m_pIPCam->Disconnect();
	delete m_pIPCam;
}

void CIpcam_sampleDlg::OnConnect() 
{
	// TODO: Add your control notification handler code here
	CString user, pwd, ip;
	unsigned short port;
	this->GetDlgItemText(IDC_IP, ip);
	this->GetDlgItemText(IDC_USER, user);
	this->GetDlgItemText(IDC_PWD, pwd);
	port = this->GetDlgItemInt(IDC_PORT);
	int ret;
	if (OK != (ret = m_pIPCam->Connect(inet_addr(ip), htons(port), user, pwd)))
	{
		CString err;
		err.Format("connect failed: %d", ret);
		AfxMessageBox(err);
	}
}

void CIpcam_sampleDlg::OnConnectResult(WPARAM wParam, LPARAM lParam)
{
	CString err;
	if (wParam == OK)
		err = "connected";
	else
		err.Format("connect failed: %d", wParam);

	this->SetDlgItemText(IDC_CONN_STATUS, err);
}

void CIpcam_sampleDlg::OnDisconnected(WPARAM wParam, LPARAM lParam)
{
	CString err;
	if (wParam == OK)
		err = "disconnected";
	else
		err.Format("disconnected: %d", wParam);
	
	this->SetDlgItemText(IDC_CONN_STATUS, err);
}

void CIpcam_sampleDlg::OnDisconnect() 
{
	// TODO: Add your control notification handler code here
	int ret;
	if (OK != (ret = m_pIPCam->Disconnect()))
	{
		CString err;
		err.Format("disconnect failed: %d", ret);
		AfxMessageBox(err);
	}	
}

void CIpcam_sampleDlg::OnPlayVideo() 
{
	// TODO: Add your control notification handler code here
	int ret;
	if (OK != (ret = m_pIPCam->PlayVideo()))
	{
		CString err;
		err.Format("play video failed: %d", ret);
		AfxMessageBox(err);
	}
}

void CIpcam_sampleDlg::OnStopVideo() 
{
	// TODO: Add your control notification handler code here
	int ret;
	if (OK != (ret = m_pIPCam->StopVideo()))
	{
		CString err;
		err.Format("stop video failed: %d", ret);
		AfxMessageBox(err);
	}
}

void CIpcam_sampleDlg::OnPlayVideoResult(WPARAM wParam, LPARAM lParam)
{
	CString err;
	if (wParam == OK)
		err = "playing";
	else
		err.Format("play failed: %d", wParam);
	
	this->SetDlgItemText(IDC_VIDEO_STATUS, err);
}

void CIpcam_sampleDlg::OnVideoStopped(WPARAM wParam, LPARAM lParam)
{
	CString err;
	if (wParam == OK)
		err = "stopped";
	else
		err.Format("stopped: %d", wParam);
	
	this->SetDlgItemText(IDC_VIDEO_STATUS, err);
	m_pShowWnd->set_jpeg(NULL, 0);
}

void CIpcam_sampleDlg::OnImage(WPARAM wParam, LPARAM lParam)
{
	IMAGE * pImage = (IMAGE *)wParam;
	
	if (! lParam)
	{
		delete [] pImage->pData;
		delete pImage;
	}
	else
	{
		m_pShowWnd->set_jpeg((unsigned char *)pImage->pData, pImage->uiDataLen);
		delete pImage;
	}

	InterlockedDecrement(&g_lImagesWaitShow);
}


void CIpcam_sampleDlg::OnPlayAudio() 
{
	// TODO: Add your control notification handler code here
	int ret;
	if (OK != (ret = m_pIPCam->PlayAudio()))
	{
		CString err;
		err.Format("play Audio failed: %d", ret);
		AfxMessageBox(err);
	}
	
}

void CIpcam_sampleDlg::OnPlayAudioResult(WPARAM wParam, LPARAM lParam)
{
	CString err;
	if (wParam == OK)
		err = "playing";
	else
		err.Format("play failed: %d", wParam);
	
	this->SetDlgItemText(IDC_AUDIO_STATUS, err);
}

void CIpcam_sampleDlg::OnStopAudio() 
{
	// TODO: Add your control notification handler code here
	int ret;
	if (OK != (ret = m_pIPCam->StopAudio()))
	{
		CString err;
		err.Format("stop Audio failed: %d", ret);
		AfxMessageBox(err);
	}
	
}


void CIpcam_sampleDlg::OnAudio(WPARAM wParam, LPARAM lParam)
{
	AUDIO * pAudio = (AUDIO *)wParam;
	
	if (! lParam)
	{
		delete [] pAudio->pData;
		delete pAudio;
	}
	else
	{
		m_pWaveOut->Play(  (char *)(pAudio->pData), pAudio->uiDataLen );
		delete pAudio;
	}
	
	InterlockedDecrement(&g_lImagesWaitShow);
}

void CIpcam_sampleDlg::OnAudioStopped(WPARAM wParam, LPARAM lParam)
{
	CString err;
	if (wParam == OK)
		err = "stopped";
	else
		err.Format("stopped: %d", wParam);
	
	this->SetDlgItemText(IDC_AUDIO_STATUS, err);
//	m_pWaveOut->StopPlay();
}


void CIpcam_sampleDlg::OnOtherDevicesParamsChanged(WPARAM wParam, LPARAM lParam)
{
	delete (OTHER_DEVICE_PARAMS *)wParam;
}




