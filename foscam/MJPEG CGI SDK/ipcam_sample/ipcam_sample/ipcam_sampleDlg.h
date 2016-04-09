/*
 * Copyright 2010-2011 Foscam Intelligent Technologies Co., Ltd. 
 *
 * All rights reserved
 *
*/

// ipcam_sampleDlg.h : header file
//

#if !defined(AFX_IPCAM_SAMPLEDLG_H__35DBC68C_9C18_41F6_8C64_29094E3F2052__INCLUDED_)
#define AFX_IPCAM_SAMPLEDLG_H__35DBC68C_9C18_41F6_8C64_29094E3F2052__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CIpcam_sampleDlg dialog
class CShowWnd;
class CWaveOut;
class COpr;

class CIpcam_sampleDlg : public CDialog
{
// Construction
public:
	CIpcam_sampleDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CIpcam_sampleDlg)
	enum { IDD = IDD_IPCAM_SAMPLE_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIpcam_sampleDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CIpcam_sampleDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDestroy();
	afx_msg void OnConnect();
	afx_msg void OnDisconnect();
	afx_msg void OnPlayVideo();
	afx_msg void OnStopVideo();
	afx_msg void OnPlayAudio();
	afx_msg void OnStopAudio();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
	CShowWnd * m_pShowWnd;
	CWaveOut * m_pWaveOut;
	COpr * m_pIPCam;

	afx_msg void OnConnectResult(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDisconnected(WPARAM wParam, LPARAM lParam);
	afx_msg void OnPlayVideoResult(WPARAM wParam, LPARAM lParam);
	afx_msg void OnPlayAudioResult(WPARAM wParam, LPARAM lParam);
	afx_msg void OnVideoStopped(WPARAM wParam, LPARAM lParam);
	afx_msg void OnAudioStopped(WPARAM wParam, LPARAM lParam);
	afx_msg void OnImage(WPARAM wParam, LPARAM lParam);
	afx_msg void OnAudio(WPARAM wParam, LPARAM lParam);

	afx_msg void OnOtherDevicesParamsChanged(WPARAM wParam, LPARAM lParam);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IPCAM_SAMPLEDLG_H__35DBC68C_9C18_41F6_8C64_29094E3F2052__INCLUDED_)
