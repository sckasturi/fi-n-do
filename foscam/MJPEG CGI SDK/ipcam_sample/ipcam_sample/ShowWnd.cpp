/*
 * Copyright 2010-2011 Foscam Intelligent Technologies Co., Ltd. 
 *
 * All rights reserved
 *
*/

// ShowWnd.cpp : implementation file
//

#include "stdafx.h"
#include "ShowWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CShowWnd

CShowWnd::CShowWnd()
{
	m_jpeg = NULL;
	m_jpeg_len = 0;
}

CShowWnd::~CShowWnd()
{
	if (m_jpeg)
		delete m_jpeg;
}


BEGIN_MESSAGE_MAP(CShowWnd, CWnd)
	//{{AFX_MSG_MAP(CShowWnd)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CShowWnd message handlers

void CShowWnd::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	// TODO: Add your message handler code here
	show();
	// Do not call CWnd::OnPaint() for painting messages
}

void CShowWnd::set_jpeg(unsigned char * jpeg, unsigned int jpeg_len)
{
	if (m_jpeg) delete m_jpeg;
	m_jpeg = jpeg;
	m_jpeg_len = jpeg_len;
	show();
}

void CShowWnd::show()
{
	CDC * pdc = this->GetDC();
	
	const COLORREF BLACK = RGB(0, 0, 0);
	RECT rcBounds;
	this->GetClientRect(&rcBounds);
	
	if (m_jpeg)
	{
		IStream *pStm;
		
		HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, m_jpeg_len);  
		//HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE,m_pJpeg->nBufferLen);  
		LPVOID pvData = NULL;  
		if (hGlobal == NULL)
			goto quit;
		if ((pvData = GlobalLock(hGlobal)) == NULL)
		{
			GlobalFree(hGlobal);
			goto quit;
		}
		
		memcpy(pvData, m_jpeg, m_jpeg_len);
		
		GlobalUnlock(hGlobal);  
		CreateStreamOnHGlobal(hGlobal,TRUE,&pStm); 
		
		IPicture *pPic;
		if (FAILED(OleLoadPicture(pStm,m_jpeg_len,TRUE,IID_IPicture,(LPVOID*)&pPic)))
		{
			GlobalFree(hGlobal);
			goto quit;
		}
		
		OLE_XSIZE_HIMETRIC hmWidth;  
		OLE_YSIZE_HIMETRIC hmHeight;  
		pPic->get_Width(&hmWidth);  
		pPic->get_Height(&hmHeight);  
		
		SIZE ImageSize;
		ImageSize.cx = hmWidth;
		ImageSize.cy = hmHeight;
		pdc->HIMETRICtoLP(&ImageSize);
		
		pdc->SetBkMode(TRANSPARENT);
		SetStretchBltMode(pdc->m_hDC,COLORONCOLOR);
		
		pPic->Render(pdc->m_hDC,(rcBounds.right - rcBounds.left - ImageSize.cx) / 2,(rcBounds.bottom - rcBounds.top - ImageSize.cy) / 2,ImageSize.cx,ImageSize.cy,0,hmHeight,hmWidth,-hmHeight,NULL);  
		pPic->Release();  
		
		GlobalFree(hGlobal);
	}
	else
	{
		pdc->FillSolidRect(&rcBounds,BLACK);
	}
quit:	
	this->ReleaseDC(pdc);
}

