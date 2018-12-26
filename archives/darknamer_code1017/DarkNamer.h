// DarkNamer.h : main header file for the DARKNAMER application
//

#if !defined(AFX_DARKNAMER_H__9A92228A_3CA3_43CB_B051_7987ED20E0CD__INCLUDED_)
#define AFX_DARKNAMER_H__9A92228A_3CA3_43CB_B051_7987ED20E0CD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CDarkNamerApp:
// See DarkNamer.cpp for the implementation of this class
//

class CDarkNamerApp : public CWinApp
{
public:
	CDarkNamerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CDarkNamerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CDarkNamerApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_DARKNAMER_H__9A92228A_3CA3_43CB_B051_7987ED20E0CD__INCLUDED_)
