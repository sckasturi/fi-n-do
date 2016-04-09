/*
 * Copyright 2010-2011 Foscam Intelligent Technologies Co., Ltd. 
 *
 * All rights reserved
 *
*/
// ipcam_sample.h : main header file for the IPCAM_SAMPLE application
//

#if !defined(AFX_IPCAM_SAMPLE_H__474F6B23_329F_4085_A8AB_CDD45CB7C729__INCLUDED_)
#define AFX_IPCAM_SAMPLE_H__474F6B23_329F_4085_A8AB_CDD45CB7C729__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CIpcam_sampleApp:
// See ipcam_sample.cpp for the implementation of this class
//

class CIpcam_sampleApp : public CWinApp
{
public:
	CIpcam_sampleApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CIpcam_sampleApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CIpcam_sampleApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_IPCAM_SAMPLE_H__474F6B23_329F_4085_A8AB_CDD45CB7C729__INCLUDED_)
