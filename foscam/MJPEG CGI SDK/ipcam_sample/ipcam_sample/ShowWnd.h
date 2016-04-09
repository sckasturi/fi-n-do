/*
 * Copyright 2010-2011 Foscam Intelligent Technologies Co., Ltd. 
 *
 * All rights reserved
 *
*/

#if !defined(AFX_SHOWWND_H__F53FD01A_EEF5_413D_8597_92071DC78234__INCLUDED_)
#define AFX_SHOWWND_H__F53FD01A_EEF5_413D_8597_92071DC78234__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ShowWnd.h : header file
//


/////////////////////////////////////////////////////////////////////////////
// CShowWnd window

class CShowWnd : public CWnd
{
// Construction
public:
	CShowWnd();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CShowWnd)
	//}}AFX_VIRTUAL

// Implementation
public:
	void set_jpeg(unsigned char * jpeg, unsigned int jpeg_len);
	virtual ~CShowWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CShowWnd)
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	void show();
		
	unsigned char * m_jpeg;
	unsigned int m_jpeg_len;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SHOWWND_H__F53FD01A_EEF5_413D_8597_92071DC78234__INCLUDED_)
